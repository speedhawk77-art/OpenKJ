#include "optionalcodec.h"
#include <QApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QTemporaryDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstdio>
#include <QProcess>
#include <QTimer>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>

int main(int argc,char **argv) {
    QApplication app(argc,argv);
    QTemporaryDir settings;
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,settings.path());
    QJsonArray checks;bool all=true;
    auto check=[&](const char *name,bool ok){checks.append(QJsonObject{{"name",name},{"passed",ok}});all&=ok;};
    check("disabled_by_default",!OptionalCodec::enabled());
    check("full_binary_license_embedded",OptionalCodec::licenseText().contains("AVC/H.264 Patent Portfolio License Conditions")
          && OptionalCodec::licenseText().contains("Any third-party software that makes use"));
    check("exact_original_cisco_license",QCryptographicHash::hash(OptionalCodec::licenseText().toUtf8(),QCryptographicHash::Sha256).toHex()=="bd9f363c5ea11ef723d0304cddacb5273c43c0e1194097c7a045d05273635418");
    QString error;
    check("reject_empty_download",!OptionalCodec::installDownloaded(QByteArray(),&error));
    check("reject_oversized_download",!OptionalCodec::installDownloaded(QByteArray(2097153,'x'),&error));
    check("reject_wrong_download_hash",!OptionalCodec::installDownloaded(QByteArray("not-a-codec"),&error));
    error.clear();auto compressed=OptionalCodec::download(&error);
    check("direct_cisco_https_download",error.isEmpty()&&!compressed.isEmpty());
    error.clear();check("verified_download_decompress_and_atomic_install",OptionalCodec::installDownloaded(compressed,&error));
    check("installed_exact_pinned_binary",OptionalCodec::verified());
    QFile corrupted(OptionalCodec::codecPath());
    if(corrupted.open(QIODevice::WriteOnly|QIODevice::Truncate)){corrupted.write("tampered");corrupted.close();}
    check("reject_tampered_cached_codec",!OptionalCodec::verified());
    OptionalCodec::configureStartup();check("tampered_codec_cannot_enable_plugin_path",qgetenv("GST_PLUGIN_PATH_1_0").isEmpty());
    OptionalCodec::setEnabled(false);corrupted.remove();
    error.clear();check("reinstall_after_cache_tamper",OptionalCodec::installDownloaded(compressed,&error));
    OptionalCodec::setEnabled(true);check("enable_persists",OptionalCodec::enabled());
    OptionalCodec::configureStartup();
    check("enabled_optional_plugin_path",qgetenv("GST_PLUGIN_PATH_1_0").contains("optional-codecs"));
    auto probe=[&](const char *label,bool wantCodec){
        if(argc!=3){check("probe_arguments_supplied",false);return;}
        QString report=QString("codec-controller-%1/probes.json").arg(label);
        int code=QProcess::execute("python",{QString::fromLocal8Bit(argv[1]),"--candidate",QCoreApplication::applicationDirPath(),
            "--fixture",QString::fromLocal8Bit(argv[2]),"--report",report,"--from-codec-controller"});
        QFile f(report);QJsonObject result;
        if(f.open(QIODevice::ReadOnly))result=QJsonDocument::fromJson(f.readAll()).object();
        bool factory=result.value("factories_present").toObject().value("openh264dec").toBool();
        bool h264=result.value("pipelines").toObject().value("h264_mp4").toObject().value("passed").toBool();
        check(label,code==0 && factory==wantCodec && (!wantCodec||h264));
    };
    probe("enabled_controller_h264_pipeline",true);

    OptionalCodec::setEnabled(false);check("disable_persists",!OptionalCodec::enabled());
    OptionalCodec::configureStartup();check("disabled_plugin_path_empty",qgetenv("GST_PLUGIN_PATH_1_0").isEmpty());
    probe("disabled_controller_decoder_absent",false);
    OptionalCodec::setEnabled(true);check("reenable_downloaded_codec",OptionalCodec::enabled()&&OptionalCodec::verified());
    QTimer::singleShot(0,[&](){
        auto dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());bool attribution=false,terms=false,disabled=false,reenabled=false;
        if(dialog){
            for(auto label:dialog->findChildren<QLabel*>())attribution|=label->text().contains("OpenH264 Video Codec provided by Cisco Systems, Inc.");
            for(auto text:dialog->findChildren<QPlainTextEdit*>())terms|=text->toPlainText()==OptionalCodec::licenseText();
            for(auto button:dialog->findChildren<QPushButton*>())if(button->text()=="Disable"){button->click();disabled=!OptionalCodec::enabled();}
            for(auto button:dialog->findChildren<QPushButton*>())if(button->text()=="Enable downloaded codec"){button->click();reenabled=OptionalCodec::enabled();}
            dialog->reject();
        }
        check("dialog_attribution_and_full_license",attribution&&terms);
        check("dialog_disable_and_reenable_controls",disabled&&reenabled);
    });
    OptionalCodec::showDialog(nullptr);
    OptionalCodec::setEnabled(false);
    QJsonObject result{{"checks",checks},{"passed",all},{"binary_distribution_approved",false}};
    auto output=QJsonDocument(result).toJson();
    QFile report("optional-codec-test-evidence.json");report.open(QIODevice::WriteOnly);report.write(output);
    std::fwrite(output.constData(),1,output.size(),stdout);
    return all?0:1;
}
