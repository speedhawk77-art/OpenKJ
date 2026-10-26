// SPDX-License-Identifier: GPL-3.0-or-later
// Real Chromium against the actual embedded HTML/CSS/JS and a simulated local host.
const {chromium}=require('playwright');
const fs=require('fs'),path=require('path'),http=require('http'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../src/hostconsole'),reportDir=process.argv[2]||'host-console-browser-evidence';fs.mkdirSync(reportDir,{recursive:true});
const clone=x=>JSON.parse(JSON.stringify(x)),checks=[],actions=[],errors=[];
const settings={autoplay:false,crossfade:true,break_autostart:false,break_after_current:false,karaoke_fade:true,break_fade:true,karaoke_mono:false,break_mono:false,recording:false,singer_display:true,fullscreen:true,ticker_enabled:true,ticker_text:'Welcome to karaoke',ticker_speed:25,ticker_rotation:true,multiplex:0,eq_k_bypass:false,eq_b_bypass:false,eq_k:[0,0,0,0,0,0,0,0,0,0],eq_b:[0,0,0,0,0,0,0,0,0,0]};
let revision=1,yt={session:'',busy:false,ready:false,message:'Ready'};
let singers=[{id:1,name:'Andrew',position:0,regular:true,current:true,songs:2,waiting:1},{id:2,name:'Mike',position:1,regular:false,current:false,songs:1,waiting:1}];
let queues={1:[{id:10,song_id:1,artist:'Journey',title:'Don’t Stop Believin’',position:0,played:true,key_change:0,duration_s:251,disc_id:'SC123'},{id:11,song_id:2,artist:'Queen',title:'Don’t Stop Me Now',position:1,played:false,key_change:2,duration_s:219,disc_id:'SC456'}],2:[{id:12,song_id:3,artist:'Fleetwood Mac',title:'Dreams',position:0,played:false,key_change:0,duration_s:240,disc_id:'SC789'}]};
const k={state:'Playing',title:'Don’t Stop Believin’',artist:'Journey',singer:'Andrew',elapsed_ms:84000,duration_ms:251000,volume:80,muted:false,key:0,tempo:100};
const b={state:'Stopped',title:'',artist:'',elapsed_ms:0,duration_ms:0,volume:35,muted:false};
function state(selected){return {ok:true,api_version:1,session:'host-session',playback_revision:1,rotation:singers,rotation_revision:'r'+revision,queue:queues[selected]||[],queue_revision:'q'+revision,break_revision:'b'+revision,settings_revision:'s'+revision,karaoke:k,break_music:b,playlists:[{id:1,title:'Break playlist'}],playlist_id:1,break_queue:[],library_count:76669,settings,youtube_download:yt,guest_requests:[{id:1,singer:'Sarah',artist:'Bon Jovi',title:'Livin’ on a Prayer'}],sound_clips:[{id:0,name:'Applause'}]};}
const server=http.createServer(async(req,res)=>{const url=new URL(req.url,'http://localhost');let body='';for await(const part of req)body+=part;const payload=body?JSON.parse(body):{};
 const json=(value,status=200)=>{res.writeHead(status,{'Content-Type':'application/json','Cache-Control':'no-store'});res.end(JSON.stringify(clone(value)));};
 if(url.pathname.startsWith('/host/')){if((payload.pin||url.searchParams.get('pin'))!=='123456'){json({ok:false,error:'Incorrect PIN'},403);return;}
 if(url.pathname==='/host/state'){json(state(Number(url.searchParams.get('singer_id'))));return;}
 if(url.pathname==='/host/search'){const mode=url.searchParams.get('mode'),q=url.searchParams.get('q')||'';const results=mode==='regulars'?[{id:1,artist:'Saved Singer'}]:[{id:5,artist:'Test Artist',title:q==='unsafe'?'<img src=x onerror=alert(1)>':'Don’t Stop Me Now',disc_id:'SC456',duration_s:219,plays:4,key_change:2}];json({ok:true,results,offset:Number(url.searchParams.get('offset')||0),more:q==='many'});return;}
 if(url.pathname==='/host/action'){actions.push(clone(payload));revision++;
 if(payload.cmd==='add_singer'){const id=3;singers.push({id,name:payload.singer_name,position:2,regular:false,current:false,songs:0,waiting:0});queues[id]=[];json({ok:true,singer_id:id,message:'Singer added'});return;}
 if(payload.cmd==='add_song'){const list=queues[payload.singer_id]||[];list.push({id:99,song_id:5,artist:'Test Artist',title:'Added remotely',position:list.length,played:false,key_change:0,duration_s:219});}
 if(payload.cmd==='volume')k.volume=payload.value;if(payload.cmd==='tune'){k.key=payload.key;k.tempo=payload.tempo;}
 if(payload.cmd==='youtube_download')yt={session:'download-session',busy:true,ready:false,message:'Downloading 43%'};
 if(payload.cmd==='youtube_cancel')yt={...yt,busy:false,message:'Cancelled'};
 if(payload.cmd==='youtube_import')yt={session:'download-session',busy:false,ready:false,imported:true,message:'Added to library'};
 if(payload.cmd==='setting')settings[payload.name]=payload.value;
 json({ok:true,message:'Done'});return;}}
 const name=url.pathname==='/'?'index.html':url.pathname==='/host-console.css'?'host.css':url.pathname==='/host-console.js'?'host.js':null;
 if(name){res.writeHead(200,{'Content-Type':name.endsWith('.css')?'text/css':name.endsWith('.js')?'application/javascript':'text/html'});res.end(fs.readFileSync(path.join(root,name)));}else {res.writeHead(404);res.end();}});
(async()=>{await new Promise(resolve=>server.listen(0,'127.0.0.1',resolve));const url='http://127.0.0.1:'+server.address().port;const browser=await chromium.launch({headless:true,...(process.env.OPENKJ_CHROMIUM?{executablePath:process.env.OPENKJ_CHROMIUM,args:['--no-sandbox','--disable-dev-shm-usage','--disable-gpu']}:{} )});
 const page=await browser.newPage({viewport:{width:1024,height:900},deviceScaleFactor:1});page.on('pageerror',e=>errors.push(e.message));
 const check=async(name,fn)=>{await fn();checks.push({name,passed:true});console.log('PASS',name);};
 try{
 await page.goto(url);await check('host_login_visible',async()=>assert.equal(await page.locator('#login').isVisible(),true));
 await page.locator('#pin').fill('000000');await page.locator('#loginForm button[type=submit]').click();await page.waitForTimeout(100);await check('wrong_pin_rejected',async()=>assert.equal(await page.locator('#app').isVisible(),false));
 await page.locator('#pin').fill('123456');await page.locator('#loginForm button[type=submit]').click();await page.locator('#app').waitFor({state:'visible'});
 await check('live_playback_status',async()=>assert.equal(await page.locator('#trackTitle').textContent(),'Don’t Stop Believin’'));
 await page.locator('#singerList .select-singer').first().click();await page.waitForTimeout(180);
 await check('selected_singer_queue_loaded',async()=>assert.equal(await page.locator('#queueSinger').textContent(),'Andrew'));
 await check('tablet_three_panels',async()=>{for(const id of ['libraryPanel','singersPanel','queuePanel'])assert.equal(await page.locator('#'+id).isVisible(),true);});
 await page.screenshot({path:path.join(reportDir,'tablet-1024.png'),fullPage:true});
 await page.locator('#songSearch').fill('unsafe');await page.waitForTimeout(450);
 await check('external_title_is_text',async()=>{assert.match(await page.locator('#songResults').textContent(),/<img/);assert.equal(await page.locator('#songResults img').count(),0);});
 await page.locator('#songResults .icon-button').first().click();await page.waitForTimeout(250);await check('song_add_posts_stable_singer_id',async()=>assert(actions.some(x=>x.cmd==='add_song'&&x.singer_id===1&&x.song_id===5)));
 await page.locator('[data-tune="key:1"]').click();await page.waitForTimeout(400);await check('key_change_posts_native_tune',async()=>assert(actions.some(x=>x.cmd==='tune'&&x.key===1&&x.tempo===100&&x.playback_revision===1)));
 await page.locator('#volume').fill('73');await page.locator('#volume').dispatchEvent('change');await page.waitForTimeout(250);await check('volume_posts_absolute_level',async()=>assert(actions.some(x=>x.cmd==='volume'&&x.value===73)));
 await page.locator('[data-tab=tools]').click();await page.locator('#youtubeUrl').fill('https://youtu.be/00000000001');await page.locator('#youtubeStart').click();await page.waitForTimeout(250);await check('youtube_busy_state',async()=>{assert.equal(await page.locator('#youtubeStart').isDisabled(),true);assert.equal(await page.locator('#youtubeCancel').isEnabled(),true);});
 yt={session:'download-session',busy:false,ready:true,message:'Verified',title:'Downloaded karaoke',artist:''};await page.waitForTimeout(1200);
 await page.locator('#youtubeArtist').fill('Chosen artist');await page.locator('#youtubeTitle').fill('Chosen title');await page.waitForTimeout(1100);await check('poll_preserves_pending_metadata',async()=>assert.equal(await page.locator('#youtubeTitle').inputValue(),'Chosen title'));
 await page.locator('#youtubeImport').click();await page.waitForTimeout(250);await check('youtube_import_uses_download_session',async()=>assert(actions.some(x=>x.cmd==='youtube_import'&&x.session==='host-session'&&x.download_session==='download-session'&&x.artist==='Chosen artist')));
 for(const width of [320,390,600,768,1024,1366]){await page.setViewportSize({width,height:900});await page.locator('[data-tab=library]').click();await page.waitForTimeout(80);await check('no_horizontal_overflow_'+width,async()=>{const sizes=await page.evaluate(()=>({inner:innerWidth,scroll:document.documentElement.scrollWidth}));assert(sizes.scroll<=sizes.inner+1,JSON.stringify(sizes));});if(width===390||width===768)await page.screenshot({path:path.join(reportDir,'viewport-'+width+'.png'),fullPage:true});}
 await page.setViewportSize({width:390,height:844});await page.locator('[data-tab=singers]').click();await check('phone_bottom_tabs_switch_panel',async()=>{assert.equal(await page.locator('#singersPanel').isVisible(),true);assert.equal(await page.locator('#libraryPanel').isVisible(),false);});
 await page.locator('#singerName').fill('New Singer');await page.locator('#addSingerForm button').click();await page.waitForTimeout(250);await check('new_singer_selected_on_phone',async()=>assert.equal(await page.locator('#queueSinger').textContent(),'New Singer'));
 await page.locator('[data-tab=settings]').click();await page.locator('#tickerText').fill('A new ticker draft');await page.waitForTimeout(1200);await check('settings_poll_preserves_draft',async()=>assert.equal(await page.locator('#tickerText').inputValue(),'A new ticker draft'));
 await page.locator('#saveTicker').click();await page.waitForTimeout(250);await check('ticker_setting_posts_revision',async()=>assert(actions.some(x=>x.cmd==='setting'&&x.name==='ticker_text'&&x.settings_revision&&x.value==='A new ticker draft')));
 await page.locator('#disconnect').click();await check('disconnect_returns_to_login',async()=>assert.equal(await page.locator('#login').isVisible(),true));
 await check('no_browser_javascript_errors',async()=>assert.deepEqual(errors,[]));
 fs.writeFileSync(path.join(reportDir,'browser-checks.json'),JSON.stringify({passed:true,engine:await browser.version(),checks,backend:'simulated HTTP service; native backend is tested separately',physical_phone_tested:false},null,2));
 }finally{await browser.close();server.close();}
})().catch(e=>{console.error(e);server.close();process.exitCode=1;});
