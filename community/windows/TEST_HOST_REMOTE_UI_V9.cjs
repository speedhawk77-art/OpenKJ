const {JSDOM,VirtualConsole}=require('jsdom');
// npm install --prefix <isolated-test-folder> jsdom@30.1.3; set NODE_PATH to its node_modules.
const fs=require('fs'),path=require('path'),assert=require('node:assert/strict');
const source=fs.readFileSync(path.join(__dirname,'../../src/remotecontrolserver.cpp'),'utf8').split('constexpr char kControlPage[] =')[1];
const html=[...source.matchAll(/R"OPENKJHTML\(([\s\S]*?)\)OPENKJHTML"/g)].map(x=>x[1]).join('');
const errors=[],calls=[],checks=[];
let state={session:'',busy:false,ready:false,message:'Ready'},responseCode=200;
const vc=new VirtualConsole();vc.on('jsdomError',e=>errors.push(e.message));
const dom=new JSDOM(html,{url:'http://127.0.0.1:8081',runScripts:'dangerously',virtualConsole:vc,beforeParse(w){
 w.setInterval=()=>0;w.confirm=()=>true;
 w.fetch=async(url,options)=>{calls.push({url,options});const payload=options?.body?JSON.parse(options.body):null;
 if(payload?.cmd==='youtube_download')state={session:'abc',busy:true,ready:false,message:'Downloading 35%'};
 if(payload?.cmd==='youtube_cancel')state={session:'abc',busy:false,ready:false,message:'Cancelled'};
 if(payload?.cmd==='youtube_import')state={session:'abc',busy:false,ready:false,imported:true,message:'Added'};
 return {ok:responseCode===200,status:responseCode,json:async()=>url==='/remote'?{youtube_download:state}:{youtube_download:state,rotation:[],selected_queue:[],guest_requests:[]}};
 };
}});
const w=dom.window,d=w.document,$=id=>d.getElementById(id);
function check(name,fn){fn();checks.push({name,passed:true});}
(async()=>{
 check('hidden_before_pin',()=>assert.equal($('youtubeHost').hidden,true));
 $('pin').value='123456';await w.updateStatus();
 check('visible_after_auth_status',()=>assert.equal($('youtubeHost').hidden,false));
 check('import_disabled_until_ready',()=>assert.equal($('youtubeImport').disabled,true));
 $('youtubeUrl').value='https://youtu.be/00000000001';await w.youtubeAction('youtube_download');await new Promise(setImmediate);
 check('start_posts_url_with_pin',()=>{const c=calls.find(x=>x.url==='/remote');const p=JSON.parse(c.options.body);assert.equal(p.cmd,'youtube_download');assert.equal(p.pin,'123456');assert.equal(p.url,$('youtubeUrl').value);assert.equal(c.options.method,'POST');});
 check('busy_controls_and_progress',()=>{assert.equal($('youtubeStart').disabled,true);assert.equal($('youtubeCancel').disabled,false);assert.equal($('youtubeStatus').textContent,'Downloading 35%');});
 state={session:'abc',busy:false,ready:true,title:'<img src=x onerror=alert(1)>',message:'<script>alert(1)</script>'};await w.updateStatus();
 check('ready_title_is_text',()=>{assert.equal($('youtubeTitle').value,state.title);assert.equal($('youtubeStatus').textContent,state.message);assert.equal($('youtubeStatus').querySelector('script'),null);});
 $('youtubeArtist').value='Chosen artist';$('youtubeArtist').dispatchEvent(new w.Event('input'));
 check('artist_enables_explicit_import',()=>assert.equal($('youtubeImport').disabled,false));
 $('youtubeTitle').value='My edited title';await w.updateStatus();
 check('polling_preserves_host_edits',()=>assert.equal($('youtubeTitle').value,'My edited title'));
 await w.youtubeAction('youtube_import');await new Promise(setImmediate);
 check('import_posts_current_session_and_metadata',()=>{const p=JSON.parse(calls.filter(x=>x.url==='/remote').at(-1).options.body);assert.equal(p.session,'abc');assert.equal(p.artist,'Chosen artist');assert.equal(p.title,'My edited title');});
 check('import_prefills_normal_queue_search',()=>{assert.equal($('songQuery').value,'My edited title');assert.equal($('youtubeImport').disabled,true);});
 responseCode=403;await w.updateStatus();
 check('wrong_pin_hides_host_downloads',()=>assert.equal($('youtubeHost').hidden,true));
 check('no_javascript_errors',()=>assert.deepEqual(errors,[]));
 fs.writeFileSync(process.argv[2]||'host-remote-ui-evidence.json',JSON.stringify({passed:true,engine:'jsdom 30.1.3',physical_phone_tested:false,checks},null,2));
 console.log(`${checks.length}/${checks.length} remote DOM checks passed`);w.close();
})().catch(e=>{console.error(e);w.close();process.exitCode=1;});
