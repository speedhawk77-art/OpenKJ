// SPDX-License-Identifier: GPL-3.0-or-later
// Native MainWindow, actual local HTTP server and SQLite models. No user data or hardware playback.
#include "mainwindow.h"
#include <QSpinBox>
#include "remotecontrolserver.h"
#include "hostconsole/remotevalidation.h"
#include "idledetect.h"
#include <QApplication>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QElapsedTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QThread>
#include <gst/gst.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
IdleDetect *filter=nullptr;
struct HostConsoleTestAccess {
    static RemoteControlServer *server(MainWindow &w){return w.m_remoteServer;}
    static void reload(MainWindow &w){w.m_karaokeSongsModel.loadData();w.m_tableModelBreakSongs.loadDatabase();}
    static QJsonObject state(MainWindow &w,int id=0){return w.remoteHostState({{"singer_id",QString::number(id)}});}
    static QJsonObject search(MainWindow &w,QJsonObject args){return w.remoteHostSearch(args);}
    static void history(MainWindow &w,const QString &path){w.m_historySongsModel.saveSong("Andrew B",path,"History Artist","History Title","TEST",2);}
    static int key(MainWindow &w){return w.findChild<QSpinBox *>("spinBoxKey")->value();}
    static int tempo(MainWindow &w){return w.findChild<QSpinBox *>("spinBoxTempo")->value();}
    static int selected(MainWindow &w){return w.m_qModel.getSingerId();}
    static QString database(MainWindow &w){return w.m_database.databaseName();}
};
static bool waitFor(const std::function<bool()> &predicate,int timeout=8000){QElapsedTimer t;t.start();while(!predicate()&&t.elapsed()<timeout){QApplication::processEvents();QThread::msleep(5);}return predicate();}
int main(int argc,char **argv){
 QApplication app(argc,argv);gst_init(nullptr,nullptr);auto logger=spdlog::stdout_color_mt("logger");logger->set_level(spdlog::level::err);
 QCoreApplication::setOrganizationName("OpenKJ");QCoreApplication::setApplicationName("OpenKJ");QStandardPaths::setTestModeEnabled(true);
 Settings settings;settings.setCheckUpdates(false);settings.dbSetLazyLoadDurations(false);settings.setShowCdgWindow(false);settings.setBmAutoStart(false);settings.setRecordingEnabled(false);settings.sync();
 QJsonArray checks;bool all=true;auto check=[&](QString name,bool passed){checks.append(QJsonObject{{"name",name},{"passed",passed}});all=all&&passed;std::cout<<(passed?"PASS ":"FAIL ")<<name.toStdString()<<std::endl;};
 try{
  MainWindow window;QTemporaryDir media;const auto songPath=media.filePath("Test karaoke.mp4");QFile song(songPath);song.open(QIODevice::WriteOnly);song.write(QByteArray(128,'x'));song.close();
  check("isolated_profile_database",HostConsoleTestAccess::database(window).contains("test",Qt::CaseInsensitive)||HostConsoleTestAccess::database(window).contains("isolated",Qt::CaseInsensitive));
  QSqlQuery sql;sql.exec("DELETE FROM queuesongs");sql.exec("DELETE FROM rotationsingers");sql.exec("DELETE FROM dbsongs");
  check("fixture_transaction",QSqlDatabase::database().transaction());
  sql.prepare("INSERT INTO dbsongs(artist,title,discid,duration,path,filename,searchstring,plays) VALUES(:artist,:title,'TEST',180,:path,'test.mp4',:search,0)");
  for(int i=0;i<76010;++i){sql.bindValue(":artist",i==0?"O'Neil":"Fixture Artist");sql.bindValue(":title",QString("Karaoke %1").arg(i,5,10,QChar('0')));sql.bindValue(":path",i==0?songPath:media.filePath(QString("missing-%1.mp4").arg(i)));sql.bindValue(":search",QString("fixture karaoke %1").arg(i));if(!sql.exec())throw std::runtime_error("Catalog fixture insert failed");}
  check("fixture_commit",QSqlDatabase::database().commit());HostConsoleTestAccess::reload(window);
  auto *server=HostConsoleTestAccess::server(window);QNetworkAccessManager network;
  struct Response{int status;QJsonObject json;QByteArray bytes;};
  auto http=[&](QString path,QJsonObject body=QJsonObject{},bool post=false){QNetworkRequest req(QUrl(QString("http://127.0.0.1:%1").arg(server->serverPort())+path));req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");auto *reply=post?network.post(req,QJsonDocument(body).toJson(QJsonDocument::Compact)):network.get(req);if(!waitFor([&]{return reply->isFinished();}))reply->abort();const auto bytes=reply->readAll();Response result{reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),QJsonDocument::fromJson(bytes).object(),bytes};delete reply;return result;};
  auto state=[&](int singer=0){return http("/host/state?pin="+server->accessPin()+"&singer_id="+QString::number(singer)).json;};
  auto call=[&](QString cmd,QJsonObject body=QJsonObject{},int singer=0){const auto s=state(singer);QJsonObject args{{"cmd",cmd},{"pin",server->accessPin()},{"session",s.value("session")},{"rotation_revision",s.value("rotation_revision")},{"queue_revision",s.value("queue_revision")},{"break_revision",s.value("break_revision")},{"settings_revision",s.value("settings_revision")},{"playback_revision",s.value("playback_revision")}};if(singer>0)args.insert("singer_id",singer);for(auto it=body.begin();it!=body.end();++it)args.insert(it.key(),it.value());return http("/host/action",args,true);};
  check("new_console_served",http("/").bytes.contains("Host Console"));check("classic_remote_preserved",http("/classic").bytes.contains("youtubeStart"));check("guest_page_preserved",http("/request").status==200&&!http("/request").bytes.contains("host-console.js"));
  check("embedded_css_served",http("/host-console.css").status==200);check("embedded_js_served",http("/host-console.js").status==200);
  for(auto endpoint:{"/host/state","/host/search","/host/action"})check(QString("host_requires_pin_")+endpoint,http(endpoint).status==403);
  check("host_mutations_require_post",http("/host/action?pin="+server->accessPin()+"&cmd=add_singer").status==405);
  check("host_read_requires_get",http("/host/state",{{"pin",server->accessPin()}},true).status==405);
  state();bool pinLimit=true;
  for(int i=0;i<12;++i)pinLimit=pinLimit&&http("/host/state?pin=wrong").status==403;
  check("incorrect_pin_attempts_throttled",pinLimit&&http("/host/state?pin=wrong").status==429);
  check("correct_pin_survives_failed_attempts",state().value("ok").toBool());
  check("invalid_host_session_rejected",call("volume",{{"target","karaoke"},{"value",10},{"session","old"}}).status==400);
  check("unknown_host_command_rejected",call("run_shell",{{"path","arbitrary.exe"}}).status==400);
  auto initial=state();const auto initialLibraryRevision=initial.value("library_revision");HostConsoleTestAccess::reload(window);check("library_changes_have_live_revision",state().value("library_revision").toInt()>initialLibraryRevision.toInt());
  check("full_catalog_count",initial.value("library_count").toInt()==76010);
  check("state_contains_no_pin_or_local_paths",!QJsonDocument(initial).toJson().contains(server->accessPin().toUtf8())&&!QJsonDocument(initial).toJson().contains(media.path().toUtf8()));
  auto search=HostConsoleTestAccess::search(window,{{"mode","all"},{"offset","0"}});check("full_library_paginated",search.value("ok").toBool()&&search.value("results").toArray().size()==50&&search.value("more").toBool());
  auto next=HostConsoleTestAccess::search(window,{{"mode","all"},{"offset","50"}});check("search_next_page_distinct",next.value("results").toArray().first().toObject().value("id")!=search.value("results").toArray().first().toObject().value("id"));
  check("search_bad_mode_rejected",!HostConsoleTestAccess::search(window,{{"mode","s.path"}}).value("ok").toBool());
  check("search_bad_offset_rejected",!HostConsoleTestAccess::search(window,{{"offset","-1"}}).value("ok").toBool());
  check("search_oversized_query_rejected",!HostConsoleTestAccess::search(window,{{"q",QString(101,'x')}}).value("ok").toBool());
  auto exact=HostConsoleTestAccess::search(window,{{"q","oneil karaoke 00000"},{"mode","all"}});check("search_apostrophe_and_words",exact.value("results").toArray().size()==1);const int songId=exact.value("results").toArray().first().toObject().value("id").toInt();
  // Exercise the HTTP boundary, not just remoteHostSearch: browser
  // URLSearchParams encodes spaces as '+', unlike encodeURIComponent.
  auto httpSearch=[&](const QString &query){return http("/host/search?pin="+server->accessPin()+"&mode=all&q="+query);};
  auto oneSong=[&](const Response &response){const auto rows=response.json.value("results").toArray();return response.status==200&&rows.size()==1&&rows.first().toObject().value("id").toInt()==songId;};
  check("http_search_form_encoded_words",oneSong(httpSearch("oneil+karaoke+00000")));
  check("http_search_percent_encoded_spaces",oneSong(httpSearch("oneil%20karaoke%2000000")));
  check("http_search_encoded_apostrophe",oneSong(httpSearch("O%27Neil+Karaoke+00000")));
  check("http_search_repeated_spaces",oneSong(httpSearch("oneil+++karaoke++00000")));
  check("http_search_words_across_artist_title",oneSong(httpSearch("00000+oneil")));
  check("http_search_literal_plus_preserved",httpSearch("karaoke%2B00000").json.value("results").toArray().isEmpty());
  check("http_search_percent_decoded_once",httpSearch("karaoke%252000000").json.value("results").toArray().isEmpty());
  auto a=call("add_singer",{{"singer_name","Andrew"},{"position_hint",1}});const int andrew=a.json.value("singer_id").toInt();check("native_singer_added",a.status==200&&andrew>0);
  auto b=call("add_singer",{{"singer_name","Mike"},{"position_hint",2}});const int mike=b.json.value("singer_id").toInt();check("second_singer_added",b.status==200&&mike>0);
  check("duplicate_singer_rejected",call("add_singer",{{"singer_name","Andrew"}}).status==400);check("invalid_singer_rejected",call("add_singer",{{"singer_name",QString(71,'x')}}).status==400);
  check("fractional_singer_id_rejected",call("rename_singer",{{"singer_id",1.5},{"name","bad"}}).status==400);
  check("independent_queue_inspection",state(andrew).value("selected_singer_id").toInt()==andrew&&state(mike).value("selected_singer_id").toInt()==mike&&HostConsoleTestAccess::selected(window)<=0);
  auto rotationSinger=[&](int id){for(const auto &entry:state().value("rotation").toArray())if(entry.toObject().value("id").toInt()==id)return entry.toObject();return QJsonObject{};};
  check("rotation_empty_singer_has_no_next_song",!rotationSinger(andrew).value("has_song").toBool()&&rotationSinger(andrew).value("next_song").toString().isEmpty());
  check("native_song_added",call("add_song",{{"song_id",songId}},andrew).status==200);
  check("rotation_next_song_matches_native_queue",rotationSinger(andrew).value("has_song").toBool()&&rotationSinger(andrew).value("next_song").toString()=="O'Neil - Karaoke 00000");
  check("fractional_song_rejected",call("add_song",{{"song_id",1.5}},andrew).status==400);
  check("missing_song_rejected",call("add_song",{{"song_id",99999999}},andrew).status==400);
  auto first=state(andrew);const int queueId=first.value("queue").toArray().first().toObject().value("id").toInt();check("queue_state_uses_stable_ids",queueId>0&&first.value("queue").toArray().first().toObject().value("song_id").toInt()==songId);
  check("queue_key_saved",call("queue_key",{{"queue_song_id",queueId},{"value",3}},andrew).status==200&&state(andrew).value("queue").toArray().first().toObject().value("key_change").toInt()==3);
  check("queue_key_range_rejected",call("queue_key",{{"queue_song_id",queueId},{"value",13}},andrew).status==400);
  check("stale_queue_revision_rejected",call("queue_key",{{"queue_song_id",queueId},{"value",1},{"queue_revision",first.value("queue_revision")}},andrew).status==400);
  check("cross_singer_queue_id_rejected",call("queue_remove",{{"queue_song_id",queueId},{"confirmed",true}},mike).status==400);
  check("song_removal_needs_confirmation",call("queue_remove",{{"queue_song_id",queueId}},andrew).status==400);
  check("queue_mark_played",call("queue_played",{{"queue_song_id",queueId},{"value",true}},andrew).status==200&&state(andrew).value("queue").toArray().first().toObject().value("played").toBool());
  check("rotation_played_song_is_not_up_next",!rotationSinger(andrew).value("has_song").toBool()&&rotationSinger(andrew).value("next_song").toString().isEmpty());
  check("queue_played_requires_boolean",call("queue_played",{{"queue_song_id",queueId},{"value","false"}},andrew).status==400);
  check("queue_mark_unplayed",call("queue_played",{{"queue_song_id",queueId},{"value",false}},andrew).status==200);
  check("queue_second_add",call("add_song",{{"song_id",songId}},andrew).status==200);
  check("queue_move_uses_native_model",call("queue_move",{{"queue_song_id",queueId},{"position",1}},andrew).status==200&&state(andrew).value("queue").toArray().last().toObject().value("id").toInt()==queueId);
  check("queue_move_bounds_rejected",call("queue_move",{{"queue_song_id",queueId},{"position",8}},andrew).status==400);
  check("queue_remove_updates_database",call("queue_remove",{{"queue_song_id",queueId},{"confirmed",true}},andrew).status==200&&state(andrew).value("queue").toArray().size()==1);
  check("queue_edit_does_not_select_on_laptop",HostConsoleTestAccess::selected(window)<=0);
  check("queue_clear_needs_confirmation",call("queue_clear",{},andrew).status==400);
  check("queue_clear_uses_native_model",call("queue_clear",{{"confirmed",true}},andrew).status==200&&state(andrew).value("queue").toArray().isEmpty());
  check("rotation_cleared_queue_has_no_next_song",!rotationSinger(andrew).value("has_song").toBool()&&rotationSinger(andrew).value("next_song").toString().isEmpty());
  check("native_singer_rename",call("rename_singer",{{"name","Andrew B"}},andrew).status==200);
  check("duplicate_rename_rejected",call("rename_singer",{{"name","Mike"}},andrew).status==400);
  check("regular_tracking_requires_confirmation",call("regular_singer",{{"value",true}},andrew).status==400);
  check("regular_tracking_enabled",call("regular_singer",{{"value",true},{"confirmed",true}},andrew).status==200);
  const auto beforeMove=state().value("rotation").toArray();const int lastSinger=beforeMove.last().toObject().value("id").toInt();
  check("rotation_reorder",call("move_singer",{{"move_direction","up"}},lastSinger).status==200&&state().value("rotation").toArray().first().toObject().value("id").toInt()==lastSinger);
  check("rotation_stale_revision_rejected",call("remove_singer",{{"confirmed",true},{"rotation_revision","old"}},andrew).status==400);
  for(int value:{-1,101})check(QString("volume_bound_")+QString::number(value),call("volume",{{"target","karaoke"},{"value",value}}).status==400);
  check("karaoke_volume_updates_native",call("volume",{{"target","karaoke"},{"value",73}}).status==200&&state().value("karaoke").toObject().value("volume").toInt()==73);
  check("break_volume_updates_native",call("volume",{{"target","break"},{"value",41}}).status==200&&state().value("break_music").toObject().value("volume").toInt()==41);
  check("fractional_volume_rejected",call("volume",{{"target","karaoke"},{"value",5.5}}).status==400);
  check("mute_requires_boolean",call("mute",{{"target","karaoke"},{"value",1}}).status==400);
  check("mute_updates_native",call("mute",{{"target","karaoke"},{"value",true}}).status==200&&state().value("karaoke").toObject().value("muted").toBool());
  check("native_key_tempo_controls",call("tune",{{"target","karaoke"},{"key",2},{"tempo",108}}).status==200&&HostConsoleTestAccess::key(window)==2&&HostConsoleTestAccess::tempo(window)==108);
  check("key_out_of_range_rejected",call("tune",{{"target","karaoke"},{"key",13},{"tempo",100}}).status==400);
  check("tempo_out_of_range_rejected",call("tune",{{"target","karaoke"},{"key",0},{"tempo",201}}).status==400);
  check("stale_playback_tune_rejected",call("tune",{{"target","karaoke"},{"key",0},{"tempo",100},{"playback_revision",-1}}).status==400);
  check("stopped_seek_rejected",call("seek",{{"target","karaoke"},{"value",1000}}).status==400);
  check("unknown_setting_rejected",call("setting",{{"name","audio_device"},{"value",true}}).status==400);
  check("native_autoplay_setting",call("setting",{{"name","autoplay"},{"value",true}}).status==200&&state().value("settings").toObject().value("autoplay").toBool());
  check("stale_setting_rejected",call("setting",{{"name","autoplay"},{"value",false},{"settings_revision","stale"}}).status==400);
  check("setting_requires_boolean",call("setting",{{"name","autoplay"},{"value","true"}}).status==400);
  check("native_ticker_setting",call("setting",{{"name","ticker_text"},{"value","Welcome <Andrew>"}}).status==200&&state().value("settings").toObject().value("ticker_text").toString()=="Welcome <Andrew>");
  check("ticker_length_rejected",call("setting",{{"name","ticker_text"},{"value",QString(501,'x')}}).status==400);
  check("equalizer_updates_native",call("eq",{{"target","karaoke"},{"band",2},{"value",4}}).status==200&&state().value("settings").toObject().value("eq_k").toArray().at(2).toInt()==4);
  check("equalizer_invalid_band_rejected",call("eq",{{"target","karaoke"},{"band",10},{"value",0}}).status==400);
  check("equalizer_invalid_level_rejected",call("eq",{{"target","karaoke"},{"band",0},{"value",13}}).status==400);
  HostConsoleTestAccess::history(window,songPath);
  const auto history=HostConsoleTestAccess::search(window,{{"mode","history"},{"name","Andrew B"},{"offset","0"}}).value("results").toArray();
  check("native_history_search_joins_local_catalog",history.size()==1&&history.first().toObject().value("id").toInt()==songId&&history.first().toObject().value("key_change").toInt()==2);
  const auto httpHistory=http("/host/search?pin="+server->accessPin()+"&mode=history&name=Andrew+B&q=History+Title");
  check("http_history_form_encoded_singer_and_words",httpHistory.status==200&&httpHistory.json.value("results").toArray().size()==1&&httpHistory.json.value("results").toArray().first().toObject().value("id").toInt()==songId);
  check("saved_singer_search",!HostConsoleTestAccess::search(window,{{"mode","regulars"},{"q","Andrew"}}).value("results").toArray().isEmpty());
  sql.prepare("INSERT INTO bmsongs(artist,title,path,filename,duration,searchstring) VALUES('Break Artist','Break Track',:path,'break.mp4','180','break track')");sql.bindValue(":path",songPath);check("break_fixture_insert",sql.exec());const int breakId=sql.lastInsertId().toInt();HostConsoleTestAccess::reload(window);
  check("break_library_search",!HostConsoleTestAccess::search(window,{{"mode","break"},{"q","Break Track"}}).value("results").toArray().isEmpty());
  check("native_break_playlist_add",call("break_add",{{"id",breakId}}).status==200&&state().value("break_queue").toArray().size()==1);
  const auto breakState=state();const int breakQueueId=breakState.value("break_queue").toArray().first().toObject().value("id").toInt();
  check("break_play_requires_confirmation",call("break_play_song",{{"id",breakQueueId}}).status==400);
  check("break_remove_requires_confirmation",call("break_remove",{{"id",breakQueueId}}).status==400);
  check("stale_break_playlist_rejected",call("break_remove",{{"id",breakQueueId},{"confirmed",true},{"break_revision","stale"}}).status==400);
  check("native_break_playlist_remove",call("break_remove",{{"id",breakQueueId},{"confirmed",true}}).status==200&&state().value("break_queue").toArray().isEmpty());
  check("missing_break_playlist_rejected",call("break_playlist",{{"id",99999999}}).status==400);
  check("native_multiplex_control",call("setting",{{"name","multiplex"},{"value",1}}).status==200&&state().value("settings").toObject().value("multiplex").toInt()==1);
  check("break_eq_control",call("eq",{{"target","break"},{"band",3},{"value",-5}}).status==200&&state().value("settings").toObject().value("eq_b").toArray().at(3).toInt()==-5);
  settings.setRecordingOutputDir(media.filePath("not-configured"));check("recording_requires_laptop_setup",call("setting",{{"name","recording"},{"value",true}}).status==400);
  check("invalid_sound_clip_rejected",call("sound_clip",{{"id",9999},{"name","Not configured"}}).status==400);
  check("unconfirmed_play_rejected",call("play_singer",{},andrew).status==400);
  check("empty_playback_rejected",call("play_singer",{{"confirmed",true}},andrew).status==400);
  check("empty_next_rejected",call("next",{{"confirmed",true}}).status==400);
  check("rotation_clear_confirmation_required",call("clear_rotation").status==400);
  check("rotation_clear_native",call("clear_rotation",{{"confirmed",true}}).status==200&&state().value("rotation").toArray().isEmpty());
  check("youtube_existing_validation_preserved",call("youtube_download",{{"url","file:///secret"}}).status==400);
  check("no_playback_started_by_model_controls",state().value("karaoke").toObject().value("state").toString()=="Stopped");
 }catch(const std::exception &e){check(QString("exception: ")+e.what(),false);}
 QFile report("host-console-native-evidence.json");report.open(QIODevice::WriteOnly);report.write(QJsonDocument(QJsonObject{{"passed",all},{"checks",checks},{"physical_playback_tested",false},{"real_phone_tested",false}}).toJson());return all?0:1;
}
