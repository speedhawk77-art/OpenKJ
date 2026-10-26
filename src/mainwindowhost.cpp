// SPDX-License-Identifier: GPL-3.0-or-later
// Host-only adapter. Reuses native models/widgets; no media is sent to the browser.
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dlgyoutubedownload.h"
#include "hostconsole/remotevalidation.h"
#include <QDateTime>
#include <QScopedValueRollback>
#include <QTimer>
#include <QFileInfo>
#include <QJsonArray>
#include <QUrlQuery>
#include <limits>
using namespace HostRemote;
namespace {
constexpr int MaxId=std::numeric_limits<int>::max();
int queryNumber(const QJsonObject &args,const QString &name,int fallback,int maximum) {
    if (!args.contains(name)) return fallback;
    bool ok=false; const auto s=args.value(name).toString(); const int n=s.toInt(&ok);
    return ok && n>=0 && n<=maximum && QString::number(n)==s ? n : -1;
}
QString mediaName(MediaBackend::State state) {
    return state==MediaBackend::PlayingState ? "Playing" : state==MediaBackend::PausedState ? "Paused" : "Stopped";
}
QJsonObject mediaState(MediaBackend &media) {
    return {{"state",mediaName(media.state())},{"elapsed_ms",double(qMax(qint64(0),media.position()))},
            {"duration_ms",double(qMax(qint64(0),media.duration()))},{"volume",media.getVolume()},{"muted",media.isMuted()}};
}
}

QJsonObject MainWindow::remoteHostState(const QJsonObject &args) {
    const int selected=queryNumber(args,"singer_id",0,MaxId);
    if (selected<0) return error(tr("Invalid singer ID"));
    QJsonArray singers,queue,playlists,breakQueue,eqK,eqB,clips;
    const auto entries=m_settings.getSfxEntries();for(int i=0;i<entries.size();++i)clips.append(QJsonObject{{"id",i},{"name",entries[i].name}});
    QHash<int,QPair<int,int>> counts;
    QSqlQuery count;
    if (count.exec("SELECT singer,count(*),sum(CASE WHEN played=0 THEN 1 ELSE 0 END) FROM queuesongs GROUP BY singer"))
        while (count.next()) counts.insert(count.value(0).toInt(),{count.value(1).toInt(),count.value(2).toInt()});
    for (int i=0;i<static_cast<int>(m_rotModel.singerCount());++i) {
        const auto &s=m_rotModel.getSingerAtPosition(i); const auto c=counts.value(s.id);
        singers.append(QJsonObject{{"id",s.id},{"name",s.name},{"position",i},{"regular",s.regular},
            {"current",s.id==m_rotModel.currentSinger()},{"top",s.id==m_settings.lastRunRotationTopSingerId()},{"songs",c.first},{"waiting",c.second}});
    }
    QSqlQuery q; q.prepare("SELECT q.qsongid,q.song,s.artist,s.title,q.played,q.keychg,q.position,s.duration,s.discid "
                          "FROM queuesongs q JOIN dbsongs s ON s.songid=q.song WHERE q.singer=:id ORDER BY q.position LIMIT 2001");
    q.bindValue(":id",selected);
    if (!q.exec()) return error(tr("Cannot read the singer queue"));
    while(q.next()) queue.append(QJsonObject{{"id",q.value(0).toInt()},{"song_id",q.value(1).toInt()},
        {"artist",q.value(2).toString()},{"title",q.value(3).toString()},{"played",q.value(4).toBool()},
        {"key_change",q.value(5).toInt()},{"position",q.value(6).toInt()},{"duration_s",q.value(7).toInt()},{"disc_id",q.value(8).toString()}});
    if (queue.size()>2000) return error(tr("This queue exceeds 2,000 songs. Manage it on the laptop."));
    QSqlQuery p;
    if(p.exec("SELECT playlistid,title FROM bmplaylists ORDER BY title COLLATE NOCASE"))
        while(p.next()) playlists.append(QJsonObject{{"id",p.value(0).toInt()},{"title",p.value(1).toString()}});
    for (int i=0;i<m_tableModelPlaylistSongs.rowCount();++i) {
        const auto song=m_tableModelPlaylistSongs.getPlSongByPosition(i); if(!song) continue;
        breakQueue.append(QJsonObject{{"id",song->get().id},{"title",song->get().title},{"artist",song->get().artist},
            {"position",i},{"current",m_tableModelPlaylistSongs.isCurrentlyPlayingSong(song->get().id)}});
    }
    for(int i=0;i<10;++i) {eqK.append(m_settings.getEqKLevel(i));eqB.append(m_settings.getEqBLevel(i));}
    auto karaoke=mediaState(m_mediaBackendKar);karaoke.insert("singer",m_curSinger);karaoke.insert("title",m_curTitle);karaoke.insert("artist",m_curArtist);
    karaoke.insert("key",ui->spinBoxKey->value());karaoke.insert("tempo",ui->spinBoxTempo->value());
    auto bm=mediaState(m_mediaBackendBm); const auto track=m_tableModelPlaylistSongs.getCurrentSong();
    bm.insert("title",track?track->get().title:QString());bm.insert("artist",track?track->get().artist:QString());
    QJsonObject settings{{"autoplay",m_settings.karaokeAutoAdvance()},{"crossfade",m_settings.bmKCrossFade()},
        {"break_autostart",m_settings.bmAutoStart()},{"break_after_current",ui->checkBoxBmBreak->isChecked()},
        {"singer_display",cdgWindow->isVisible()},{"fullscreen",cdgWindow->isFullScreen()},
        {"ticker_enabled",m_settings.tickerEnabled()},{"ticker_text",m_settings.tickerCustomString()},
        {"ticker_speed",m_settings.tickerSpeed()},{"ticker_rotation",m_settings.tickerShowRotationInfo()},
        {"karaoke_fade",m_settings.audioUseFader()},{"break_fade",m_settings.audioUseFaderBm()},
        {"karaoke_mono",m_settings.audioDownmix()},{"break_mono",m_settings.audioDownmixBm()},
        {"multiplex",m_settings.mplxMode()},{"recording",m_settings.recordingEnabled()},
        {"eq_k_bypass",m_settings.eqKBypass()},{"eq_b_bypass",m_settings.eqBBypass()},{"eq_k",eqK},{"eq_b",eqB}};
    // The legacy payload supplies the existing guest-request and YouTube status contracts.
    const auto legacy=remoteLibraryStatus();
    QSqlQuery size; int libraryCount=0; if(size.exec("SELECT count(*) FROM dbsongs")&&size.next())libraryCount=size.value(0).toInt();
    return {{"ok",true},{"api_version",1},{"session",m_hostSession},{"playback_revision",m_hostPlaybackRevision},{"server_time_ms",double(QDateTime::currentMSecsSinceEpoch())},
        {"rotation",singers},{"rotation_revision",revision(singers)},{"selected_singer_id",selected},
        {"queue",queue},{"queue_revision",revision(queue)},{"karaoke",karaoke},{"break_music",bm},
        {"playlists",playlists},{"playlist_id",m_tableModelPlaylistSongs.currentPlaylist()},
        {"break_queue",breakQueue},{"break_revision",revision(breakQueue)},{"library_count",libraryCount},{"library_revision",m_hostLibraryRevision},
        {"settings",settings},{"settings_revision",revision(settings)},{"sound_clips",clips},
        {"youtube_download",legacy.value("youtube_download")},{"guest_requests",legacy.value("guest_requests")}};
}

QJsonObject MainWindow::remoteHostSearch(const QJsonObject &args) {
    const QString term=args.value("q").toString().trimmed(), mode=args.value("mode").toString("all");
    const int offset=queryNumber(args,"offset",0,10000000);
    if(term.size()>100||offset<0||!QStringList{"all","artist","title","disc","popular","break","history","regulars"}.contains(mode))
        return error(tr("Invalid search options"));
    QString from,fields,where=" WHERE 1=1",order; QMap<QString,QVariant> bindings;
    if(mode=="history") {
        const auto name=args.value("name").toString(); if(!text(name,70))return error(tr("Choose a singer for history"));
        from="historySongs h JOIN historySingers hs ON hs.id=h.historySinger LEFT JOIN dbsongs s ON s.path=h.filepath";
        fields="coalesce(s.songid,0),h.artist,h.title,h.songid,coalesce(s.duration,0),h.plays,h.keychange";
        where+=" AND hs.name=:name";bindings[":name"]=name;order="h.lastplay DESC,h.id DESC";
    } else if(mode=="regulars") {
        from="historySingers hs";fields="hs.id,hs.name,'' AS title,'' AS disc,0,0,0";order="hs.name COLLATE NOCASE";
    } else {
        from=mode=="break"?"bmsongs s":"dbsongs s";
        fields=mode=="break"?"s.songid,s.artist,s.title,'' AS disc,s.duration,0,0":"s.songid,s.artist,s.title,s.discid,s.duration,s.plays,0";
        order=mode=="popular"?"s.plays DESC,s.artist COLLATE NOCASE,s.title COLLATE NOCASE,s.songid":"s.artist COLLATE NOCASE,s.title COLLATE NOCASE,s.songid";
    }
    QString column=mode=="regulars"?"hs.name":mode=="history"?"h.artist || ' ' || h.title":mode=="artist"?"s.artist":mode=="title"?"s.title":mode=="disc"?"s.discid":mode=="break"?"s.artist || ' ' || s.title":"s.artist || ' ' || s.title || ' ' || coalesce(s.discid,'')";
    QString normalized=term.simplified().toLower(); normalized.remove('\'');normalized.remove(QChar(0x2019));normalized.replace('-',' ');
    int i=0;for(const auto &word:normalized.split(' ',Qt::SkipEmptyParts)) {
        const auto key=QString(":w%1").arg(i++);
        where+=" AND instr(lower(replace(replace(replace(coalesce("+column+",''),char(39),''),char(8217),''),'-',' ')),"+key+")>0";
        bindings[key]=word;
    }
    QSqlQuery query;query.prepare("SELECT "+fields+" FROM "+from+where+" ORDER BY "+order+" LIMIT 51 OFFSET :offset");
    for(auto it=bindings.begin();it!=bindings.end();++it)query.bindValue(it.key(),it.value());query.bindValue(":offset",offset);
    if(!query.exec())return error(tr("Library search failed"));
    QJsonArray results;bool more=false;
    while(query.next()) {
        if(results.size()==50){more=true;break;}
        results.append(QJsonObject{{"id",query.value(0).toInt()},{"artist",query.value(1).toString()},{"title",query.value(2).toString()},
            {"disc_id",query.value(3).toString()},{"duration_s",query.value(4).toInt()},{"plays",query.value(5).toInt()},{"key_change",query.value(6).toInt()}});
    }
    return {{"ok",true},{"results",results},{"offset",offset},{"more",more}};
}

QJsonObject MainWindow::remoteHostAction(const QString &command,const QJsonObject &args) {
    if(m_shuttingDown)return error(tr("OpenKJ is closing"));
    if(args.value("session").toString()!=m_hostSession)return error(tr("OpenKJ restarted. Reconnect before changing controls."));
    int singerId=0; if(args.contains("singer_id")&&!integer(args.value("singer_id"),1,MaxId,singerId))return error(tr("Invalid singer ID"));
    const auto state=remoteHostState({{"singer_id",QString::number(singerId)}});
    if(!state.value("ok").toBool())return state;
    auto fresh=[&](const QString &key){return !args.value(key).toString().isEmpty() && args.value(key)==state.value(key);};
    auto confirmed=[&](){return args.value("confirmed").isBool()&&args.value("confirmed").toBool();};
    auto active=[&](){return m_mediaBackendKar.state()==MediaBackend::PlayingState||m_mediaBackendKar.state()==MediaBackend::PausedState;};
    if(command=="volume"||command=="mute"||command=="seek"||command=="tune") {
        const auto target=args.value("target").toString(); if(target!="karaoke"&&target!="break")return error(tr("Choose karaoke or break music"));
        auto &media=target=="karaoke"?m_mediaBackendKar:m_mediaBackendBm;
        if((command=="seek"||command=="tune")&&args.value("playback_revision")!=state.value("playback_revision"))return error(tr("Playback changed. Refresh before adjusting the song."));
        if(command=="volume") {int value;if(!integer(args.value("value"),0,100,value))return error(tr("Volume must be 0–100"));
            if(target=="karaoke")ui->sliderVolume->setValue(value);else ui->sliderBmVolume->setValue(value);
        } else if(command=="mute") {
            if(!args.value("value").isBool())return error(tr("Invalid mute setting"));media.setMuted(args.value("value").toBool());
        } else if(command=="seek") {
            int value;if(!integer(args.value("value"),0,MaxId,value)||media.duration()<=0||value>media.duration()||media.state()==MediaBackend::StoppedState)return error(tr("Seek requires an active song and a valid time"));
            media.setPosition(value);
        } else {
            if(target!="karaoke")return error(tr("Key and tempo apply to karaoke"));
            int key,tempo;if(!integer(args.value("key"),-12,12,key)||!integer(args.value("tempo"),50,200,tempo))return error(tr("Key must be −12 to +12; tempo 50–200%"));
            ui->spinBoxKey->setValue(key);ui->spinBoxTempo->setValue(tempo);
        }
        return done();
    }
    if(command=="setting") {
        if(!fresh("settings_revision"))return error(tr("Settings changed on another device. Refresh and retry."));
        const auto name=args.value("name").toString();const auto value=args.value("value");
        if(name=="ticker_text") {
            if(!value.isString()||value.toString().size()>500)return error(tr("Ticker text is limited to 500 characters"));
            m_settings.setTickerCustomString(value.toString());rotationDataChanged();
        } else if(name=="ticker_speed") {
            int speed;if(!integer(value,0,50,speed))return error(tr("Ticker speed must be 0–50"));m_settings.setTickerSpeed(speed);cdgWindow->tickerSpeedChanged();
        } else if(name=="multiplex") {
            int mode;if(!integer(value,0,2,mode))return error(tr("Choose left, both or right"));
            if(mode==0)ui->pushButtonMplxBoth->setChecked(true);else if(mode==1)ui->pushButtonMplxLeft->setChecked(true);else ui->pushButtonMplxRight->setChecked(true);
        } else {
            if(!value.isBool())return error(tr("A true/false setting is required"));const bool on=value.toBool();
            if(name=="autoplay")ui->actionAutoplay_mode->setChecked(on);
            else if(name=="crossfade")m_settings.setBmKCrossfade(on);
            else if(name=="break_autostart")m_settings.setBmAutoStart(on);
            else if(name=="break_after_current")ui->checkBoxBmBreak->setChecked(on);
            else if(name=="singer_display")ui->btnToggleCdgWindow->setChecked(on);
            else if(name=="fullscreen") {if(on!=cdgWindow->isFullScreen())cdgWindow->btnToggleFullscreenClicked();}
            else if(name=="ticker_enabled"){m_settings.setTickerEnabled(on);cdgWindow->tickerEnableChanged();}
            else if(name=="ticker_rotation"){m_settings.setTickerShowRotationInfo(on);rotationDataChanged();}
            else if(name=="karaoke_fade"){m_settings.setAudioUseFader(on);m_mediaBackendKar.setUseFader(on);}
            else if(name=="break_fade"){m_settings.setAudioUseFaderBm(on);m_mediaBackendBm.setUseFader(on);}
            else if(name=="karaoke_mono"){m_settings.setAudioDownmix(on);m_mediaBackendKar.setDownmix(on);}
            else if(name=="break_mono"){m_settings.setAudioDownmixBm(on);m_mediaBackendBm.setDownmix(on);}
            else if(name=="recording"){
                if(active())return error(tr("Change recording between songs"));
                if(on&&(!QFileInfo(m_settings.recordingOutputDir()).isDir()||m_settings.recordingInput().isEmpty()))return error(tr("Configure recording input and output folder on the laptop first"));
                m_settings.setRecordingEnabled(on);
            }
            else if(name=="eq_k_bypass"){m_settings.setEqKBypass(on);m_mediaBackendKar.setEqBypass(on);}
            else if(name=="eq_b_bypass"){m_settings.setEqBBypass(on);m_mediaBackendBm.setEqBypass(on);}
            else return error(tr("That setting must be configured on the laptop"));
        }
        m_settings.sync();return done(tr("Setting saved"));
    }
    if(command=="eq") {
        const auto target=args.value("target").toString();int band,level;
        if(!fresh("settings_revision")||!integer(args.value("band"),0,9,band)||!integer(args.value("value"),-12,12,level)||(target!="karaoke"&&target!="break"))return error(tr("Invalid or stale equalizer change"));
        if(target=="karaoke"){m_settings.setEqKLevel(band,level);m_mediaBackendKar.setEqLevel(band,level);}
        else {m_settings.setEqBLevel(band,level);m_mediaBackendBm.setEqLevel(band,level);}return done();
    }
    if(command=="sound_clip"||command=="sound_stop") {
        if(command=="sound_stop"){stopSfxPlayback();return done();}
        const auto clips=m_settings.getSfxEntries();int id;
        if(!integer(args.value("id"),0,clips.size()-1,id)||args.value("name").toString()!=clips[id].name||!QFileInfo(clips[id].path).isFile())return error(tr("Sound clip is no longer available"));
        m_mediaBackendSfx.setMedia(clips[id].path);m_mediaBackendSfx.setVolume(ui->sliderVolume->value());m_mediaBackendSfx.play();return done();
    }
    if(command=="break_toggle"||command=="break_next"||command=="break_previous"||command=="break_stop"||command=="break_fade_out"||command=="break_fade_in") {
        if(command=="break_stop")buttonBmStopClicked();
        else if(command=="break_next")remoteBreakNext();
        else if(command=="break_previous")remoteBreakPrevious();
        else if(command=="break_fade_out")m_mediaBackendBm.fadeOut(false);
        else if(command=="break_fade_in"){if(m_mediaBackendKar.state()==MediaBackend::PlayingState)return error(tr("Pause karaoke before fading break music back in"));m_mediaBackendBm.fadeIn(false);}
        else if(m_mediaBackendBm.state()==MediaBackend::PlayingState){m_mediaBackendBm.pause();ui->buttonBmPause->setChecked(true);}
        else if(m_mediaBackendBm.state()==MediaBackend::PausedState){m_mediaBackendBm.play();ui->buttonBmPause->setChecked(false);}
        else if(m_tableModelPlaylistSongs.rowCount()>0)remoteStartBreakAtPosition(qMax(0,m_tableModelPlaylistSongs.currentPosition()));
        else return error(tr("Add break music to the playlist first"));
        return done();
    }
    if(command=="add_singer") {
        const auto name=args.value("singer_name").toString().simplified();int hint=TableModelRotation::ADD_BOTTOM;
        if(args.contains("position_hint")&&!integer(args.value("position_hint"),0,2,hint))return error(tr("Invalid singer placement"));
        if(!text(name,70)||m_rotModel.singerExists(name))return error(tr("Enter a unique singer name (1–70 characters)"));
        const int id=m_rotModel.singerAdd(name,hint);if(id<=0)return error(tr("Could not add singer"));
        return {{"ok",true},{"message",tr("Singer added")},{"singer_id",id}};
    }
    if(command=="clear_rotation") {
        if(!confirmed()||!fresh("rotation_revision"))return error(tr("Confirm the current rotation before clearing it"));
        if(active())return error(tr("Stop karaoke before clearing the rotation"));
        m_settings.setCurrentRotationPosition(-1);m_rotModel.clearRotation();m_rotDelegate.setCurrentSinger(-1);m_qModel.loadSinger(-1);return done(tr("Rotation cleared"));
    }
    if(command=="rename_singer"||command=="regular_singer"||command=="rotation_top") {
        const auto &singer=m_rotModel.getSinger(singerId);if(!singer.isValid())return error(tr("Singer no longer exists"));
        if(!fresh("rotation_revision"))return error(tr("The rotation changed. Refresh and retry."));
        if(command=="rename_singer") {
            const auto name=args.value("name").toString().simplified();if(!text(name,70))return error(tr("Enter a singer name (1–70 characters)"));
            if(name.compare(singer.name,Qt::CaseInsensitive)!=0&&m_rotModel.singerExists(name))return error(tr("Singer name already exists"));
            if(active()&&m_rotModel.currentSinger()==singerId)return error(tr("Rename this singer after their song finishes"));
            m_rotModel.singerSetName(singerId,name);
        } else if(command=="regular_singer") {
            if(!confirmed()||!args.value("value").isBool())return error(tr("Confirm the regular singer change"));
            if(args.value("value").toBool())m_rotModel.singerMakeRegular(singerId);else m_rotModel.singerDisableRegularTracking(singerId);
        } else {m_rotModel.setRotationTopSingerId(singerId);m_rotModel.layoutChanged();}
        return done();
    }
    if(command=="queue_move"||command=="queue_remove"||command=="queue_key"||command=="queue_played"||command=="queue_clear") {
        if(!m_rotModel.getSinger(singerId).isValid()||!fresh("queue_revision"))return error(tr("The queue changed. Refresh and retry."));
        if((command=="queue_clear"||command=="queue_remove")&&!confirmed())return error(tr("Confirm song removal"));
        TableModelQueueSongs queue(m_karaokeSongsModel);queue.loadSinger(singerId);
        if(command=="queue_clear") {
            if(active()&&m_rotModel.currentSinger()==singerId)return error(tr("Stop this singer's song before clearing their queue"));
            queue.removeAll();
        } else {
            int id;if(!integer(args.value("queue_song_id"),1,MaxId,id))return error(tr("Invalid queue entry"));
            int position=-1;for(const auto &item:state.value("queue").toArray())if(item.toObject().value("id").toInt()==id)position=item.toObject().value("position").toInt();
            if(position<0)return error(tr("Song is not in this singer's queue"));
            if(command=="queue_move") {int to;if(!integer(args.value("position"),0,queue.rowCount()-1,to))return error(tr("Invalid queue position"));queue.moveSongId(id,to);}
            else if(command=="queue_remove")queue.remove(id);
            else if(command=="queue_key") {int key;if(!integer(args.value("value"),-12,12,key))return error(tr("Invalid key change"));queue.setKey(id,key);}
            else {if(!args.value("value").isBool())return error(tr("Invalid played setting"));queue.setPlayed(id,args.value("value").toBool());}
        }
        if(m_qModel.getSingerId()==singerId)m_qModel.loadSinger(singerId);
        m_dlgRegularSingers.regularsChanged();updateRotationDuration();m_rotModel.layoutChanged();return done(tr("Queue updated"));
    }
    if(command=="break_playlist"||command=="break_add"||command=="break_remove"||command=="break_move"||command=="break_play_song") {
        int id;if(!integer(args.value("id"),1,MaxId,id))return error(tr("Invalid playlist or song ID"));
        if(command=="break_playlist") {
            for(int i=0;i<ui->comboBoxBmPlaylists->count();++i)if(m_tableModelPlaylists->index(i,0).data().toInt()==id){ui->comboBoxBmPlaylists->setCurrentIndex(i);return done();}
            return error(tr("Playlist no longer exists"));
        }
        if(!fresh("break_revision"))return error(tr("The break playlist changed. Refresh and retry."));
        if(command=="break_add") {
            QSqlQuery song;song.prepare("SELECT path FROM bmsongs WHERE songid=:id");song.bindValue(":id",id);
            if(!song.exec()||!song.next()||!QFileInfo(song.value(0).toString()).isFile())return error(tr("Break song is missing on laptop"));
            m_tableModelPlaylistSongs.addSong(id);
        } else {
            const int pos=m_tableModelPlaylistSongs.getSongPositionById(id);if(pos<0)return error(tr("Break song no longer exists"));
            if(command=="break_play_song") {if(!confirmed())return error(tr("Confirm break playback"));remoteStartBreakAtPosition(pos);return done();}
            if(command=="break_remove"){if(!confirmed())return error(tr("Confirm removal"));m_tableModelPlaylistSongs.deleteSong(pos);}
            else {int to;if(!integer(args.value("position"),0,m_tableModelPlaylistSongs.rowCount()-1,to))return error(tr("Invalid playlist position"));m_tableModelPlaylistSongs.moveSong(pos,to);}
        }
        m_tableModelPlaylistSongs.savePlaylistChanges();return done();
    }
    if(command=="play_queue"||command=="play_singer"||command=="next"||command=="stop"||command=="toggle") {
        if((command!="toggle")&&!confirmed())return error(tr("Confirm the playback change"));
        if(command=="play_queue"||command=="play_singer") {
            if(!m_rotModel.getSinger(singerId).isValid())return error(tr("Singer no longer exists"));
            int id=0;
            if(command=="play_queue") {if(!fresh("queue_revision")||!integer(args.value("queue_song_id"),1,MaxId,id))return error(tr("The queue changed. Refresh before playing."));}
            else id=m_rotModel.getSinger(singerId).nextSongQueueId();
            QSqlQuery song;song.prepare("SELECT s.path FROM queuesongs q JOIN dbsongs s ON s.songid=q.song WHERE q.qsongid=:id AND q.singer=:singer");song.bindValue(":id",id);song.bindValue(":singer",singerId);
            if(!song.exec()||!song.next()||!QFileInfo(song.value(0).toString()).isFile())return error(tr("No playable song exists for this singer"));
            QTimer::singleShot(0,this,[this,singerId,id] {
                if(m_shuttingDown)return;const auto &s=m_rotModel.getSinger(singerId);if(!s.isValid())return;
                QScopedValueRollback<bool> confirmation(m_remoteConfirmedAction,true);
                ui->tableViewRotation->selectRow(s.position);m_qModel.loadSinger(singerId);
                for(int i=0;i<m_qModel.rowCount();++i)if(m_qModel.index(i,0).data().toInt()==id){tableViewQueueDoubleClicked(m_qModel.index(i,0));break;}
            });return done(tr("Playback request accepted"));
        }
        if(command=="next"||(command=="toggle"&&!active())) {
            QSqlQuery pending;if(!pending.exec("SELECT 1 FROM queuesongs q JOIN dbsongs s ON s.songid=q.song JOIN rotationSingers r ON r.singerid=q.singer WHERE q.played=0 LIMIT 1")||!pending.next())return error(tr("No unplayed songs are queued"));
        }
        QTimer::singleShot(0,this,[this,command] {
            if(m_shuttingDown)return;QScopedValueRollback<bool> confirmation(m_remoteConfirmedAction,true);
            if(command=="stop")buttonStopClicked();
            else if(command=="next")playNextQueuedSong();
            else if(m_mediaBackendKar.state()==MediaBackend::PlayingState)m_mediaBackendKar.pause();
            else if(m_mediaBackendKar.state()==MediaBackend::PausedState)m_mediaBackendKar.play();
            else playNextQueuedSong();
        });return done(tr("Playback request accepted"));
    }
    // Existing add/select/remove/move, guest moderation and YouTube commands retain native handlers.
    if(QStringList{"add_singer","select_singer","remove_singer","move_singer","add_song","approve_guest_request","reject_guest_request","youtube_download","youtube_cancel","youtube_import"}.contains(command)) {
        if((command=="remove_singer"||command=="move_singer")&&!fresh("rotation_revision"))return error(tr("The rotation changed. Refresh and retry."));
        if(command=="remove_singer"&&!confirmed())return error(tr("Confirm singer removal"));
        if(command=="add_song"){int id;if(!integer(args.value("song_id"),1,MaxId,id))return error(tr("Invalid song ID"));}
        if(command=="approve_guest_request"||command=="reject_guest_request"){int id;if(!integer(args.value("guest_request_id"),1,MaxId,id))return error(tr("Invalid request ID"));}
        auto forwarded=args;
        // Download sessions are distinct from the host login session.
        if(command.startsWith("youtube_"))forwarded.insert("session",args.value("download_session"));
        return remoteLibraryAction(command,forwarded);
    }
    return error(tr("Unknown host command"));
}
