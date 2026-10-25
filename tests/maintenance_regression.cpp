// Focused integration regressions for the v9 maintenance update.
#include "dlgeq.h"
#include "mediabackend.h"
#include "audiorecorder.h"
#include "softwarerendervideosink.h"
#include "tickernew.h"
#include "idledetect.h"
#include "models/tablemodelhistorysongs.h"
#include "cdg/cdgimageframe.h"
#include <QApplication>
#include <QSlider>
#include <QTemporaryDir>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QElapsedTimer>
#include <QDirIterator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <functional>
#include <iostream>
#include <stdexcept>

IdleDetect *filter = nullptr;
struct MaintenanceTestAccess {
    static void observePlayback(MediaBackend &backend,int *destroyed,bool attach) {
        for (auto *element : {backend.m_audioBin,backend.m_videoBin,backend.m_decoder,
                              backend.m_cdgSrc->getSrcElement()}) {
            g_object_weak_ref(G_OBJECT(element),[](gpointer p,GObject *){++*static_cast<int *>(p);},destroyed);
            if (attach && !gst_object_has_as_parent(GST_OBJECT(element),GST_OBJECT(backend.m_pipeline)))
                gst_bin_add(GST_BIN(backend.m_pipeline),element);
        }
    }
    static QImage image(GstSample *sample) { return SoftwareRenderVideoSink::imageFromSample(sample); }
    static void messages(AudioRecorder &r) { r.processGstMessage(); }
    static bool finished(AudioRecorder &r) { return !r.m_recording && !r.m_finalizing && !r.m_finalizeTimer.isActive(); }
    static void syntheticInput(AudioRecorder &r) {
        r.m_timer.stop();
        // Exercise the production encoder/muxer graph; substitute only the microphone.
        gst_element_set_state(r.m_pipeline, GST_STATE_NULL);
        gst_element_unlink(r.m_autoAudioSrc,r.m_audioRate);
        gst_bin_remove(GST_BIN(r.m_pipeline),r.m_autoAudioSrc);
        r.m_autoAudioSrc=gst_element_factory_make("audiotestsrc",nullptr);
        if (!r.m_autoAudioSrc) throw std::runtime_error("Synthetic recording source unavailable");
        g_object_set(r.m_autoAudioSrc,"is-live",TRUE,nullptr);
        if (!gst_bin_add(GST_BIN(r.m_pipeline),r.m_autoAudioSrc) ||
            !gst_element_link(r.m_autoAudioSrc,r.m_audioRate))
            throw std::runtime_error("Synthetic recording source could not join production graph");
        r.m_timer.start(10);
    }
    static bool warningReleasedOnce(AudioRecorder &r) {
        int destroyed = 0;
        auto *error = g_error_new_literal(g_quark_from_static_string("maintenance-test"), 1, "Expected synthetic warning");
        auto *message = gst_message_new_warning(GST_OBJECT(r.m_pipeline), error, "Synthetic test warning");
        g_error_free(error);
        gst_mini_object_weak_ref(GST_MINI_OBJECT(message), [](gpointer data, GstMiniObject *) { ++*static_cast<int *>(data); }, &destroyed);
        gst_bus_post(r.m_bus, message);
        r.processGstMessage();
        return destroyed == 1;
    }
};
static bool waitFor(const std::function<bool()> &predicate, int timeout=3000) {
    QElapsedTimer timer; timer.start();
    while (!predicate() && timer.elapsed() < timeout) {
        QApplication::processEvents(); QThread::msleep(5);
    }
    return predicate();
}
static void spin(int ms) {
    QElapsedTimer t; t.start();
    waitFor([&] { return t.elapsed() >= ms; }, ms+1000);
}
static bool oggFinalized(const QString &root, const QString &name) {
    QDirIterator files(root, {name}, QDir::Files, QDirIterator::Subdirectories);
    if (!files.hasNext()) return false;
    QFile f(files.next()); if (!f.open(QIODevice::ReadOnly)) return false;
    const auto bytes = f.readAll(); int position=0; bool eos=false;
    while (position+27 <= bytes.size()) {
        if (bytes.mid(position,4)!="OggS") return false;
        const int segments=static_cast<uchar>(bytes[position+26]);
        if (position+27+segments > bytes.size()) return false;
        int payload=0;
        for (int i=0;i<segments;++i) payload+=static_cast<uchar>(bytes[position+27+i]);
        eos=(static_cast<uchar>(bytes[position+5])&4)!=0;
        position+=27+segments+payload;
        if (position > bytes.size()) return false;
    }
    return position==bytes.size() && eos;
}
int main(int argc,char **argv) {
    QApplication app(argc,argv);
    QCoreApplication::setOrganizationName("OpenKJ-Maintenance-Tests");
    QCoreApplication::setApplicationName("Isolated-Regressions");
    QTemporaryDir profile;
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,profile.path());
    auto logger=spdlog::stdout_color_mt("logger"); logger->set_level(spdlog::level::warn);
    gst_init(nullptr,nullptr);
    g_log_set_always_fatal(static_cast<GLogLevelFlags>(G_LOG_LEVEL_ERROR|G_LOG_LEVEL_CRITICAL));
    QJsonArray checks; bool all=true;
    auto check=[&](const QString &name,bool ok) {
        checks.append(QJsonObject{{"name",name},{"passed",ok}}); all &= ok;
        std::cout << (ok?"PASS ":"FAIL ") << name.toStdString() << std::endl;
    };
    try {
        {
            DlgEq eq;
            for (const QString &side : {QString("K"),QString("B")}) {
                int band=-1;
                auto connection = side=="K"
                    ? QObject::connect(&eq,&DlgEq::karEqLevelChanged,[&](int b,int){band=b;})
                    : QObject::connect(&eq,&DlgEq::bmEqLevelChanged,[&](int b,int){band=b;});
                for (int i=0;i<10;++i) {
                    auto *slider=eq.findChild<QSlider *>(QString("verticalSliderEq%1%2").arg(side).arg(i+1));
                    if (!slider) throw std::runtime_error("Equalizer slider missing");
                    slider->setValue(slider->value()==slider->maximum()?slider->minimum():slider->value()+1);
                    check(QString("equalizer_%1_band_%2").arg(side).arg(i),band==i);
                }
                QObject::disconnect(connection);
            }
        }
        {
            auto db=QSqlDatabase::addDatabase("QSQLITE"); db.setDatabaseName(":memory:");
            if (!db.open()) throw std::runtime_error("SQLite unavailable");
            QSqlQuery q;
            for (const auto *sql : {"CREATE TABLE historySingers(id INTEGER PRIMARY KEY,name TEXT)",
                 "CREATE TABLE historySongs(id INTEGER PRIMARY KEY,historySinger INT,filepath TEXT,artist TEXT,title TEXT,songid TEXT,keychange INT,plays INT,lastplay TEXT)",
                 "INSERT INTO historySingers VALUES(1,'Andrew')",
                 "INSERT INTO historySongs VALUES(1,1,'song.zip','Artist','Title','ID',0,1,'2026-10-25')"})
                if (!q.exec(sql)) throw std::runtime_error(q.lastError().text().toStdString());
            TableModelKaraokeSongs songs; TableModelHistorySongs history(songs);
            history.loadSinger(QString("Andrew")); check("history_initial_load",history.rowCount({})==1);
            history.refresh(); check("history_refresh_keeps_existing_singer",history.rowCount({})==1);
            q.exec("DELETE FROM historySingers"); history.refresh();
            check("history_missing_singer_clears_view",history.rowCount({})==0);
            q.exec("SELECT COUNT(*) FROM historySongs"); q.next();
            check("history_refresh_preserves_stored_songs",q.value(0).toInt()==1);
        }
        {
            std::array<char,16> data{}; data[1]=0x17;data[2]=0x1f;
            const cdg::CdgScrollCmdData parsed(data);
            check("cdg_invalid_offsets_clamped",parsed.hSOffset==5 && parsed.vSOffset==11);
            auto command=[](cdg::CdgCommand instruction,int h,int v,int color) {
                cdg::CDG_SubCode code{};code.command=static_cast<cdg::CdgCommand>(9);code.instruction=instruction;
                code.data[0]=color;code.data[1]=h;code.data[2]=v;return code;
            };
            CdgImageFrame frame;
            frame.applySubCode(command(cdg::CmdScrollPreset,0x20,0,5));
            auto image=frame.getImage();check("cdg_left_preset_fills_right_edge",image.pixelIndex(294,0)==5 && image.pixelIndex(6,0)==0);
            frame.applySubCode(command(cdg::CmdScrollCopy,0x10,0,0));
            image=frame.getImage();check("cdg_right_copy_wraps_pixels",image.pixelIndex(0,0)==5 && image.pixelIndex(6,0)==0);
            frame.applySubCode(command(cdg::CmdScrollPreset,0,0x20,7));
            image=frame.getImage();check("cdg_up_preset_fills_bottom",image.pixelIndex(0,215)==7 && image.pixelIndex(6,0)==0);
            frame.applySubCode(command(cdg::CmdScrollCopy,0,0x10,0));
            image=frame.getImage();check("cdg_down_copy_wraps_pixels",image.pixelIndex(0,0)==7);
            frame.applySubCode(command(cdg::CmdScrollCopy,0x17,0x1f,0));
            QByteArray output(cdg::CDG_IMAGE_SIZE,0);frame.copyCroppedImagedata(reinterpret_cast<uchar *>(output.data()));
            check("cdg_malformed_packet_crop_completes",!frame.getImage().isNull());
        }
        {
            auto sample=[](const char *format,int bytes,int *freed=nullptr) {
                auto *caps=gst_caps_new_simple("video/x-raw","format",G_TYPE_STRING,format,"width",G_TYPE_INT,3,"height",G_TYPE_INT,2,nullptr);
                auto *buffer=gst_buffer_new_allocate(nullptr,bytes,nullptr);
                auto *s=gst_sample_new(buffer,caps,nullptr,nullptr);gst_buffer_unref(buffer);gst_caps_unref(caps);
                if (freed) gst_mini_object_weak_ref(GST_MINI_OBJECT(s),[](gpointer p,GstMiniObject *){++*static_cast<int *>(p);},freed);
                return s;
            };
            check("video_null_sample_rejected",MaintenanceTestAccess::image(nullptr).isNull());
            check("video_unsupported_format_rejected",MaintenanceTestAccess::image(sample("I420",64)).isNull());
            check("video_short_buffer_rejected",MaintenanceTestAccess::image(sample("BGRx",1)).isNull());
            int freed=0;auto image=MaintenanceTestAccess::image(sample("BGRx",24,&freed));auto copy=image;
            check("video_valid_frame_mapped",!image.isNull() && image.size()==QSize(3,2));
            image={};check("video_sample_retained_by_image_copy",freed==0);
            copy={};check("video_sample_released_once",freed==1);
        }
        {
            int rendered=0;TickerNew ticker;ticker.setWidth(300);
            QObject::connect(&ticker,&TickerNew::newFrameRect,&app,[&](const QImage &image,QRect){if (!image.isNull()) ++rendered;});
            ticker.setText("Maintenance ticker rendering regression",true);spin(100);
            ticker.startScrolling();check("ticker_image_frames_delivered",waitFor([&]{return rendered>0;}));
            ticker.stop();check("ticker_stops",ticker.wait(2000));
            bool stopped=true;
            for(int i=0;i<25;++i){ticker.startScrolling();ticker.stop();stopped &= ticker.wait(2000);}
            check("ticker_immediate_start_stop_is_safe",stopped);
        }
        {
            for (int i=0;i<4;++i) {
                int destroyed=0;
                { MediaBackend backend(nullptr,"Maintenance teardown",MediaBackend::Karaoke);
                  MaintenanceTestAccess::observePlayback(backend,&destroyed,i%2!=0); }
                check(QString("playback_owned_objects_destroyed_%1").arg(i),destroyed==4);
            }
            int destroyed=0;
            { QWidget surface; SoftwareRenderVideoSink sink(&surface);
              g_object_weak_ref(G_OBJECT(sink.getSink()),[](gpointer p,GObject *){++*static_cast<int *>(p);},&destroyed); }
            check("unused_software_sink_destroyed_once",destroyed==1);
        }
        {
            Settings settings;settings.setRecordingOutputDir(profile.path());settings.setRecordingCodec("OGG Vorbis");
            AudioRecorder recorder;MaintenanceTestAccess::syntheticInput(recorder);
            check("recording_warning_released_once",MaintenanceTestAccess::warningReleasedOnce(recorder));
            recorder.record("one");spin(250);recorder.stop();
            check("recording_finalizes_before_closing",waitFor([&]{return MaintenanceTestAccess::finished(recorder);}));
            check("recording_ogg_has_complete_eos_page",oggFinalized(profile.path(),"one.ogg"));
            recorder.record("paused");spin(200);recorder.pause();recorder.stop();
            check("paused_recording_finalizes",waitFor([&]{return MaintenanceTestAccess::finished(recorder);}));
            check("paused_recording_ogg_has_eos",oggFinalized(profile.path(),"paused.ogg"));
            recorder.record("queued-first");spin(100);recorder.stop();recorder.record("queued-next");spin(400);recorder.stop();
            check("queued_next_recording_finalizes",waitFor([&]{return MaintenanceTestAccess::finished(recorder);}));
            check("queued_next_recording_was_not_dropped",oggFinalized(profile.path(),"queued-next.ogg"));
        }
        {
            { AudioRecorder recorder;MaintenanceTestAccess::syntheticInput(recorder);
              recorder.record("shutdown-paused");spin(200);recorder.pause(); }
            check("paused_recording_finalizes_on_destruction",oggFinalized(profile.path(),"shutdown-paused.ogg"));
        }
    } catch (const std::exception &e) { check(QString("exception: ")+e.what(),false); }
    QFile report("maintenance-regression-evidence.json");report.open(QIODevice::WriteOnly);
    report.write(QJsonDocument(QJsonObject{{"passed",all},{"checks",checks}}).toJson());
    return all?0:1;
}
