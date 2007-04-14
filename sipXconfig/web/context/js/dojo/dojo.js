/*
	Copyright (c) 2004-2006, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

/*
	This is a compiled version of Dojo, built for deployment and not for
	development. To get an editable version, please visit:

		http://dojotoolkit.org

	for documentation and information on getting the source.
*/

if(typeof dojo=="undefined"){
var dj_global=this;
function dj_undef(_1,_2){
if(_2==null){
_2=dj_global;
}
return (typeof _2[_1]=="undefined");
}
if(dj_undef("djConfig")){
var djConfig={};
}
if(dj_undef("dojo")){
var dojo={};
}
dojo.version={major:0,minor:3,patch:1,flag:"",revision:Number("$Rev: 4342 $".match(/[0-9]+/)[0]),toString:function(){
with(dojo.version){
return major+"."+minor+"."+patch+flag+" ("+revision+")";
}
}};
dojo.evalProp=function(_3,_4,_5){
return (_4&&!dj_undef(_3,_4)?_4[_3]:(_5?(_4[_3]={}):undefined));
};
dojo.parseObjPath=function(_6,_7,_8){
var _9=(_7!=null?_7:dj_global);
var _a=_6.split(".");
var _b=_a.pop();
for(var i=0,l=_a.length;i<l&&_9;i++){
_9=dojo.evalProp(_a[i],_9,_8);
}
return {obj:_9,prop:_b};
};
dojo.evalObjPath=function(_d,_e){
if(typeof _d!="string"){
return dj_global;
}
if(_d.indexOf(".")==-1){
return dojo.evalProp(_d,dj_global,_e);
}
var _f=dojo.parseObjPath(_d,dj_global,_e);
if(_f){
return dojo.evalProp(_f.prop,_f.obj,_e);
}
return null;
};
dojo.errorToString=function(_10){
if(!dj_undef("message",_10)){
return _10.message;
}else{
if(!dj_undef("description",_10)){
return _10.description;
}else{
return _10;
}
}
};
dojo.raise=function(_11,_12){
if(_12){
_11=_11+": "+dojo.errorToString(_12);
}
try{
dojo.hostenv.println("FATAL: "+_11);
}
catch(e){
}
throw Error(_11);
};
dojo.debug=function(){
};
dojo.debugShallow=function(obj){
};
dojo.profile={start:function(){
},end:function(){
},stop:function(){
},dump:function(){
}};
function dj_eval(_14){
return dj_global.eval?dj_global.eval(_14):eval(_14);
}
dojo.unimplemented=function(_15,_16){
var _17="'"+_15+"' not implemented";
if(_16!=null){
_17+=" "+_16;
}
dojo.raise(_17);
};
dojo.deprecated=function(_18,_19,_1a){
var _1b="DEPRECATED: "+_18;
if(_19){
_1b+=" "+_19;
}
if(_1a){
_1b+=" -- will be removed in version: "+_1a;
}
dojo.debug(_1b);
};
dojo.inherits=function(_1c,_1d){
if(typeof _1d!="function"){
dojo.raise("dojo.inherits: superclass argument ["+_1d+"] must be a function (subclass: ["+_1c+"']");
}
_1c.prototype=new _1d();
_1c.prototype.constructor=_1c;
_1c.superclass=_1d.prototype;
_1c["super"]=_1d.prototype;
};
dojo.render=(function(){
function vscaffold(_1e,_1f){
var tmp={capable:false,support:{builtin:false,plugin:false},prefixes:_1e};
for(var _21 in _1f){
tmp[_21]=false;
}
return tmp;
}
return {name:"",ver:dojo.version,os:{win:false,linux:false,osx:false},html:vscaffold(["html"],["ie","opera","khtml","safari","moz"]),svg:vscaffold(["svg"],["corel","adobe","batik"]),vml:vscaffold(["vml"],["ie"]),swf:vscaffold(["Swf","Flash","Mm"],["mm"]),swt:vscaffold(["Swt"],["ibm"])};
})();
dojo.hostenv=(function(){
var _22={isDebug:false,allowQueryConfig:false,baseScriptUri:"",baseRelativePath:"",libraryScriptUri:"",iePreventClobber:false,ieClobberMinimal:true,preventBackButtonFix:true,searchIds:[],parseWidgets:true};
if(typeof djConfig=="undefined"){
djConfig=_22;
}else{
for(var _23 in _22){
if(typeof djConfig[_23]=="undefined"){
djConfig[_23]=_22[_23];
}
}
}
return {name_:"(unset)",version_:"(unset)",getName:function(){
return this.name_;
},getVersion:function(){
return this.version_;
},getText:function(uri){
dojo.unimplemented("getText","uri="+uri);
}};
})();
dojo.hostenv.getBaseScriptUri=function(){
if(djConfig.baseScriptUri.length){
return djConfig.baseScriptUri;
}
var uri=new String(djConfig.libraryScriptUri||djConfig.baseRelativePath);
if(!uri){
dojo.raise("Nothing returned by getLibraryScriptUri(): "+uri);
}
var _26=uri.lastIndexOf("/");
djConfig.baseScriptUri=djConfig.baseRelativePath;
return djConfig.baseScriptUri;
};
(function(){
var _27={pkgFileName:"__package__",loading_modules_:{},loaded_modules_:{},addedToLoadingCount:[],removedFromLoadingCount:[],inFlightCount:0,modulePrefixes_:{dojo:{name:"dojo",value:"src"}},setModulePrefix:function(_28,_29){
this.modulePrefixes_[_28]={name:_28,value:_29};
},getModulePrefix:function(_2a){
var mp=this.modulePrefixes_;
if((mp[_2a])&&(mp[_2a]["name"])){
return mp[_2a].value;
}
return _2a;
},getTextStack:[],loadUriStack:[],loadedUris:[],post_load_:false,modulesLoadedListeners:[],unloadListeners:[],loadNotifying:false};
for(var _2c in _27){
dojo.hostenv[_2c]=_27[_2c];
}
})();
dojo.hostenv.loadPath=function(_2d,_2e,cb){
var uri;
if((_2d.charAt(0)=="/")||(_2d.match(/^\w+:/))){
uri=_2d;
}else{
uri=this.getBaseScriptUri()+_2d;
}
if(djConfig.cacheBust&&dojo.render.html.capable){
uri+="?"+String(djConfig.cacheBust).replace(/\W+/g,"");
}
try{
return ((!_2e)?this.loadUri(uri,cb):this.loadUriAndCheck(uri,_2e,cb));
}
catch(e){
dojo.debug(e);
return false;
}
};
dojo.hostenv.loadUri=function(uri,cb){
if(this.loadedUris[uri]){
return 1;
}
var _33=this.getText(uri,null,true);
if(_33==null){
return 0;
}
this.loadedUris[uri]=true;
if(cb){
_33="("+_33+")";
}
var _34=dj_eval(_33);
if(cb){
cb(_34);
}
return 1;
};
dojo.hostenv.loadUriAndCheck=function(uri,_36,cb){
var ok=true;
try{
ok=this.loadUri(uri,cb);
}
catch(e){
dojo.debug("failed loading ",uri," with error: ",e);
}
return ((ok)&&(this.findModule(_36,false)))?true:false;
};
dojo.loaded=function(){
};
dojo.unloaded=function(){
};
dojo.hostenv.loaded=function(){
this.loadNotifying=true;
this.post_load_=true;
var mll=this.modulesLoadedListeners;
for(var x=0;x<mll.length;x++){
mll[x]();
}
this.modulesLoadedListeners=[];
this.loadNotifying=false;
dojo.loaded();
};
dojo.hostenv.unloaded=function(){
var mll=this.unloadListeners;
while(mll.length){
(mll.pop())();
}
dojo.unloaded();
};
dojo.addOnLoad=function(obj,_3d){
var dh=dojo.hostenv;
if(arguments.length==1){
dh.modulesLoadedListeners.push(obj);
}else{
if(arguments.length>1){
dh.modulesLoadedListeners.push(function(){
obj[_3d]();
});
}
}
if(dh.post_load_&&dh.inFlightCount==0&&!dh.loadNotifying){
dh.callLoaded();
}
};
dojo.addOnUnload=function(obj,_40){
var dh=dojo.hostenv;
if(arguments.length==1){
dh.unloadListeners.push(obj);
}else{
if(arguments.length>1){
dh.unloadListeners.push(function(){
obj[_40]();
});
}
}
};
dojo.hostenv.modulesLoaded=function(){
if(this.post_load_){
return;
}
if((this.loadUriStack.length==0)&&(this.getTextStack.length==0)){
if(this.inFlightCount>0){
dojo.debug("files still in flight!");
return;
}
dojo.hostenv.callLoaded();
}
};
dojo.hostenv.callLoaded=function(){
if(typeof setTimeout=="object"){
setTimeout("dojo.hostenv.loaded();",0);
}else{
dojo.hostenv.loaded();
}
};
dojo.hostenv.getModuleSymbols=function(_42){
var _43=_42.split(".");
for(var i=_43.length-1;i>0;i--){
var _45=_43.slice(0,i).join(".");
var _46=this.getModulePrefix(_45);
if(_46!=_45){
_43.splice(0,i,_46);
break;
}
}
return _43;
};
dojo.hostenv._global_omit_module_check=false;
dojo.hostenv.loadModule=function(_47,_48,_49){
if(!_47){
return;
}
_49=this._global_omit_module_check||_49;
var _4a=this.findModule(_47,false);
if(_4a){
return _4a;
}
if(dj_undef(_47,this.loading_modules_)){
this.addedToLoadingCount.push(_47);
}
this.loading_modules_[_47]=1;
var _4b=_47.replace(/\./g,"/")+".js";
var _4c=this.getModuleSymbols(_47);
var _4d=((_4c[0].charAt(0)!="/")&&(!_4c[0].match(/^\w+:/)));
var _4e=_4c[_4c.length-1];
var _4f=_47.split(".");
if(_4e=="*"){
_47=(_4f.slice(0,-1)).join(".");
while(_4c.length){
_4c.pop();
_4c.push(this.pkgFileName);
_4b=_4c.join("/")+".js";
if(_4d&&(_4b.charAt(0)=="/")){
_4b=_4b.slice(1);
}
ok=this.loadPath(_4b,((!_49)?_47:null));
if(ok){
break;
}
_4c.pop();
}
}else{
_4b=_4c.join("/")+".js";
_47=_4f.join(".");
var ok=this.loadPath(_4b,((!_49)?_47:null));
if((!ok)&&(!_48)){
_4c.pop();
while(_4c.length){
_4b=_4c.join("/")+".js";
ok=this.loadPath(_4b,((!_49)?_47:null));
if(ok){
break;
}
_4c.pop();
_4b=_4c.join("/")+"/"+this.pkgFileName+".js";
if(_4d&&(_4b.charAt(0)=="/")){
_4b=_4b.slice(1);
}
ok=this.loadPath(_4b,((!_49)?_47:null));
if(ok){
break;
}
}
}
if((!ok)&&(!_49)){
dojo.raise("Could not load '"+_47+"'; last tried '"+_4b+"'");
}
}
if(!_49&&!this["isXDomain"]){
_4a=this.findModule(_47,false);
if(!_4a){
dojo.raise("symbol '"+_47+"' is not defined after loading '"+_4b+"'");
}
}
return _4a;
};
dojo.hostenv.startPackage=function(_51){
var _52=dojo.evalObjPath((_51.split(".").slice(0,-1)).join("."));
this.loaded_modules_[(new String(_51)).toLowerCase()]=_52;
var _53=_51.split(/\./);
if(_53[_53.length-1]=="*"){
_53.pop();
}
return dojo.evalObjPath(_53.join("."),true);
};
dojo.hostenv.findModule=function(_54,_55){
var lmn=(new String(_54)).toLowerCase();
if(this.loaded_modules_[lmn]){
return this.loaded_modules_[lmn];
}
var _57=dojo.evalObjPath(_54);
if((_54)&&(typeof _57!="undefined")&&(_57)){
this.loaded_modules_[lmn]=_57;
return _57;
}
if(_55){
dojo.raise("no loaded module named '"+_54+"'");
}
return null;
};
dojo.kwCompoundRequire=function(_58){
var _59=_58["common"]||[];
var _5a=(_58[dojo.hostenv.name_])?_59.concat(_58[dojo.hostenv.name_]||[]):_59.concat(_58["default"]||[]);
for(var x=0;x<_5a.length;x++){
var _5c=_5a[x];
if(_5c.constructor==Array){
dojo.hostenv.loadModule.apply(dojo.hostenv,_5c);
}else{
dojo.hostenv.loadModule(_5c);
}
}
};
dojo.require=function(){
dojo.hostenv.loadModule.apply(dojo.hostenv,arguments);
};
dojo.requireIf=function(){
if((arguments[0]===true)||(arguments[0]=="common")||(arguments[0]&&dojo.render[arguments[0]].capable)){
var _5d=[];
for(var i=1;i<arguments.length;i++){
_5d.push(arguments[i]);
}
dojo.require.apply(dojo,_5d);
}
};
dojo.requireAfterIf=dojo.requireIf;
dojo.provide=function(){
return dojo.hostenv.startPackage.apply(dojo.hostenv,arguments);
};
dojo.setModulePrefix=function(_5f,_60){
return dojo.hostenv.setModulePrefix(_5f,_60);
};
dojo.exists=function(obj,_62){
var p=_62.split(".");
for(var i=0;i<p.length;i++){
if(!(obj[p[i]])){
return false;
}
obj=obj[p[i]];
}
return true;
};
}
if(typeof window=="undefined"){
dojo.raise("no window object");
}
(function(){
if(djConfig.allowQueryConfig){
var _65=document.location.toString();
var _66=_65.split("?",2);
if(_66.length>1){
var _67=_66[1];
var _68=_67.split("&");
for(var x in _68){
var sp=_68[x].split("=");
if((sp[0].length>9)&&(sp[0].substr(0,9)=="djConfig.")){
var opt=sp[0].substr(9);
try{
djConfig[opt]=eval(sp[1]);
}
catch(e){
djConfig[opt]=sp[1];
}
}
}
}
}
if(((djConfig["baseScriptUri"]=="")||(djConfig["baseRelativePath"]==""))&&(document&&document.getElementsByTagName)){
var _6c=document.getElementsByTagName("script");
var _6d=/(__package__|dojo|bootstrap1)\.js([\?\.]|$)/i;
for(var i=0;i<_6c.length;i++){
var src=_6c[i].getAttribute("src");
if(!src){
continue;
}
var m=src.match(_6d);
if(m){
var _71=src.substring(0,m.index);
if(src.indexOf("bootstrap1")>-1){
_71+="../";
}
if(!this["djConfig"]){
djConfig={};
}
if(djConfig["baseScriptUri"]==""){
djConfig["baseScriptUri"]=_71;
}
if(djConfig["baseRelativePath"]==""){
djConfig["baseRelativePath"]=_71;
}
break;
}
}
}
var dr=dojo.render;
var drh=dojo.render.html;
var drs=dojo.render.svg;
var dua=drh.UA=navigator.userAgent;
var dav=drh.AV=navigator.appVersion;
var t=true;
var f=false;
drh.capable=t;
drh.support.builtin=t;
dr.ver=parseFloat(drh.AV);
dr.os.mac=dav.indexOf("Macintosh")>=0;
dr.os.win=dav.indexOf("Windows")>=0;
dr.os.linux=dav.indexOf("X11")>=0;
drh.opera=dua.indexOf("Opera")>=0;
drh.khtml=(dav.indexOf("Konqueror")>=0)||(dav.indexOf("Safari")>=0);
drh.safari=dav.indexOf("Safari")>=0;
var _79=dua.indexOf("Gecko");
drh.mozilla=drh.moz=(_79>=0)&&(!drh.khtml);
if(drh.mozilla){
drh.geckoVersion=dua.substring(_79+6,_79+14);
}
drh.ie=(document.all)&&(!drh.opera);
drh.ie50=drh.ie&&dav.indexOf("MSIE 5.0")>=0;
drh.ie55=drh.ie&&dav.indexOf("MSIE 5.5")>=0;
drh.ie60=drh.ie&&dav.indexOf("MSIE 6.0")>=0;
drh.ie70=drh.ie&&dav.indexOf("MSIE 7.0")>=0;
dojo.locale=(drh.ie?navigator.userLanguage:navigator.language).toLowerCase();
dr.vml.capable=drh.ie;
drs.capable=f;
drs.support.plugin=f;
drs.support.builtin=f;
if(document.implementation&&document.implementation.hasFeature&&document.implementation.hasFeature("org.w3c.dom.svg","1.0")){
drs.capable=t;
drs.support.builtin=t;
drs.support.plugin=f;
}
})();
dojo.hostenv.startPackage("dojo.hostenv");
dojo.render.name=dojo.hostenv.name_="browser";
dojo.hostenv.searchIds=[];
dojo.hostenv._XMLHTTP_PROGIDS=["Msxml2.XMLHTTP","Microsoft.XMLHTTP","Msxml2.XMLHTTP.4.0"];
dojo.hostenv.getXmlhttpObject=function(){
var _7a=null;
var _7b=null;
try{
_7a=new XMLHttpRequest();
}
catch(e){
}
if(!_7a){
for(var i=0;i<3;++i){
var _7d=dojo.hostenv._XMLHTTP_PROGIDS[i];
try{
_7a=new ActiveXObject(_7d);
}
catch(e){
_7b=e;
}
if(_7a){
dojo.hostenv._XMLHTTP_PROGIDS=[_7d];
break;
}
}
}
if(!_7a){
return dojo.raise("XMLHTTP not available",_7b);
}
return _7a;
};
dojo.hostenv.getText=function(uri,_7f,_80){
var _81=this.getXmlhttpObject();
if(_7f){
_81.onreadystatechange=function(){
if(4==_81.readyState){
if((!_81["status"])||((200<=_81.status)&&(300>_81.status))){
_7f(_81.responseText);
}
}
};
}
_81.open("GET",uri,_7f?true:false);
try{
_81.send(null);
if(_7f){
return null;
}
if((_81["status"])&&((200>_81.status)||(300<=_81.status))){
throw Error("Unable to load "+uri+" status:"+_81.status);
}
}
catch(e){
if((_80)&&(!_7f)){
return null;
}else{
throw e;
}
}
return _81.responseText;
};
dojo.hostenv.defaultDebugContainerId="dojoDebug";
dojo.hostenv._println_buffer=[];
dojo.hostenv._println_safe=false;
dojo.hostenv.println=function(_82){
if(!dojo.hostenv._println_safe){
dojo.hostenv._println_buffer.push(_82);
}else{
try{
var _83=document.getElementById(djConfig.debugContainerId?djConfig.debugContainerId:dojo.hostenv.defaultDebugContainerId);
if(!_83){
_83=document.getElementsByTagName("body")[0]||document.body;
}
var div=document.createElement("div");
div.appendChild(document.createTextNode(_82));
_83.appendChild(div);
}
catch(e){
try{
document.write("<div>"+_82+"</div>");
}
catch(e2){
window.status=_82;
}
}
}
};
dojo.addOnLoad(function(){
dojo.hostenv._println_safe=true;
while(dojo.hostenv._println_buffer.length>0){
dojo.hostenv.println(dojo.hostenv._println_buffer.shift());
}
});
function dj_addNodeEvtHdlr(_85,_86,fp,_88){
var _89=_85["on"+_86]||function(){
};
_85["on"+_86]=function(){
fp.apply(_85,arguments);
_89.apply(_85,arguments);
};
return true;
}
dj_addNodeEvtHdlr(window,"load",function(){
if(arguments.callee.initialized){
return;
}
arguments.callee.initialized=true;
var _8a=function(){
if(dojo.render.html.ie){
dojo.hostenv.makeWidgets();
}
};
if(dojo.hostenv.inFlightCount==0){
_8a();
dojo.hostenv.modulesLoaded();
}else{
dojo.addOnLoad(_8a);
}
});
dj_addNodeEvtHdlr(window,"unload",function(){
dojo.hostenv.unloaded();
});
dojo.hostenv.makeWidgets=function(){
var _8b=[];
if(djConfig.searchIds&&djConfig.searchIds.length>0){
_8b=_8b.concat(djConfig.searchIds);
}
if(dojo.hostenv.searchIds&&dojo.hostenv.searchIds.length>0){
_8b=_8b.concat(dojo.hostenv.searchIds);
}
if((djConfig.parseWidgets)||(_8b.length>0)){
if(dojo.evalObjPath("dojo.widget.Parse")){
var _8c=new dojo.xml.Parse();
if(_8b.length>0){
for(var x=0;x<_8b.length;x++){
var _8e=document.getElementById(_8b[x]);
if(!_8e){
continue;
}
var _8f=_8c.parseElement(_8e,null,true);
dojo.widget.getParser().createComponents(_8f);
}
}else{
if(djConfig.parseWidgets){
var _8f=_8c.parseElement(document.getElementsByTagName("body")[0]||document.body,null,true);
dojo.widget.getParser().createComponents(_8f);
}
}
}
}
};
dojo.addOnLoad(function(){
if(!dojo.render.html.ie){
dojo.hostenv.makeWidgets();
}
});
try{
if(dojo.render.html.ie){
document.write("<style>v:*{ behavior:url(#default#VML); }</style>");
document.write("<xml:namespace ns=\"urn:schemas-microsoft-com:vml\" prefix=\"v\"/>");
}
}
catch(e){
}
dojo.hostenv.writeIncludes=function(){
};
dojo.byId=function(id,doc){
if(id&&(typeof id=="string"||id instanceof String)){
if(!doc){
doc=document;
}
return doc.getElementById(id);
}
return id;
};
(function(){
if(typeof dj_usingBootstrap!="undefined"){
return;
}
var _92=false;
var _93=false;
var _94=false;
if((typeof this["load"]=="function")&&((typeof this["Packages"]=="function")||(typeof this["Packages"]=="object"))){
_92=true;
}else{
if(typeof this["load"]=="function"){
_93=true;
}else{
if(window.widget){
_94=true;
}
}
}
var _95=[];
if((this["djConfig"])&&((djConfig["isDebug"])||(djConfig["debugAtAllCosts"]))){
_95.push("debug.js");
}
if((this["djConfig"])&&(djConfig["debugAtAllCosts"])&&(!_92)&&(!_94)){
_95.push("browser_debug.js");
}
if((this["djConfig"])&&(djConfig["compat"])){
_95.push("compat/"+djConfig["compat"]+".js");
}
var _96=djConfig["baseScriptUri"];
if((this["djConfig"])&&(djConfig["baseLoaderUri"])){
_96=djConfig["baseLoaderUri"];
}
for(var x=0;x<_95.length;x++){
var _98=_96+"src/"+_95[x];
if(_92||_93){
load(_98);
}else{
try{
document.write("<scr"+"ipt type='text/javascript' src='"+_98+"'></scr"+"ipt>");
}
catch(e){
var _99=document.createElement("script");
_99.src=_98;
document.getElementsByTagName("head")[0].appendChild(_99);
}
}
}
})();
dojo.fallback_locale="en";
dojo.normalizeLocale=function(_9a){
return _9a?_9a.toLowerCase():dojo.locale;
};
dojo.requireLocalization=function(_9b,_9c,_9d){
dojo.debug("EXPERIMENTAL: dojo.requireLocalization");
var _9e=dojo.hostenv.getModuleSymbols(_9b);
var _9f=_9e.concat("nls").join("/");
_9d=dojo.normalizeLocale(_9d);
var _a0=_9d.split("-");
var _a1=[];
for(var i=_a0.length;i>0;i--){
_a1.push(_a0.slice(0,i).join("-"));
}
if(_a1[_a1.length-1]!=dojo.fallback_locale){
_a1.push(dojo.fallback_locale);
}
var _a3=[_9b,"_nls",_9c].join(".");
var _a4=dojo.hostenv.startPackage(_a3);
dojo.hostenv.loaded_modules_[_a3]=_a4;
var _a5=false;
for(var i=_a1.length-1;i>=0;i--){
var loc=_a1[i];
var pkg=[_a3,loc].join(".");
var _a8=false;
if(!dojo.hostenv.findModule(pkg)){
dojo.hostenv.loaded_modules_[pkg]=null;
var _a9=[_9f,loc,_9c].join("/")+".js";
_a8=dojo.hostenv.loadPath(_a9,null,function(_aa){
_a4[loc]=_aa;
if(_a5){
for(var x in _a5){
if(!_a4[loc][x]){
_a4[loc][x]=_a5[x];
}
}
}
});
}else{
_a8=true;
}
if(_a8&&_a4[loc]){
_a5=_a4[loc];
}
}
};
dojo.provide("dojo.lang.common");
dojo.require("dojo.lang");
dojo.lang._mixin=function(obj,_ad){
var _ae={};
for(var x in _ad){
if(typeof _ae[x]=="undefined"||_ae[x]!=_ad[x]){
obj[x]=_ad[x];
}
}
if(dojo.render.html.ie&&dojo.lang.isFunction(_ad["toString"])&&_ad["toString"]!=obj["toString"]){
obj.toString=_ad.toString;
}
return obj;
};
dojo.lang.mixin=function(obj,_b1){
for(var i=1,l=arguments.length;i<l;i++){
dojo.lang._mixin(obj,arguments[i]);
}
return obj;
};
dojo.lang.extend=function(_b3,_b4){
for(var i=1,l=arguments.length;i<l;i++){
dojo.lang._mixin(_b3.prototype,arguments[i]);
}
return _b3;
};
dojo.lang.find=function(arr,val,_b8,_b9){
if(!dojo.lang.isArrayLike(arr)&&dojo.lang.isArrayLike(val)){
var a=arr;
arr=val;
val=a;
}
var _bb=dojo.lang.isString(arr);
if(_bb){
arr=arr.split("");
}
if(_b9){
var _bc=-1;
var i=arr.length-1;
var end=-1;
}else{
var _bc=1;
var i=0;
var end=arr.length;
}
if(_b8){
while(i!=end){
if(arr[i]===val){
return i;
}
i+=_bc;
}
}else{
while(i!=end){
if(arr[i]==val){
return i;
}
i+=_bc;
}
}
return -1;
};
dojo.lang.indexOf=dojo.lang.find;
dojo.lang.findLast=function(arr,val,_c1){
return dojo.lang.find(arr,val,_c1,true);
};
dojo.lang.lastIndexOf=dojo.lang.findLast;
dojo.lang.inArray=function(arr,val){
return dojo.lang.find(arr,val)>-1;
};
dojo.lang.isObject=function(wh){
if(typeof wh=="undefined"){
return false;
}
return (typeof wh=="object"||wh===null||dojo.lang.isArray(wh)||dojo.lang.isFunction(wh));
};
dojo.lang.isArray=function(wh){
return (wh instanceof Array||typeof wh=="array");
};
dojo.lang.isArrayLike=function(wh){
if(dojo.lang.isString(wh)){
return false;
}
if(dojo.lang.isFunction(wh)){
return false;
}
if(dojo.lang.isArray(wh)){
return true;
}
if(typeof wh!="undefined"&&wh&&dojo.lang.isNumber(wh.length)&&isFinite(wh.length)){
return true;
}
return false;
};
dojo.lang.isFunction=function(wh){
if(!wh){
return false;
}
return (wh instanceof Function||typeof wh=="function");
};
dojo.lang.isString=function(wh){
return (wh instanceof String||typeof wh=="string");
};
dojo.lang.isAlien=function(wh){
if(!wh){
return false;
}
return !dojo.lang.isFunction()&&/\{\s*\[native code\]\s*\}/.test(String(wh));
};
dojo.lang.isBoolean=function(wh){
return (wh instanceof Boolean||typeof wh=="boolean");
};
dojo.lang.isNumber=function(wh){
return (wh instanceof Number||typeof wh=="number");
};
dojo.lang.isUndefined=function(wh){
return ((wh==undefined)&&(typeof wh=="undefined"));
};
dojo.provide("dojo.lang");
dojo.provide("dojo.lang.Lang");
dojo.require("dojo.lang.common");
dojo.provide("dojo.lang.func");
dojo.require("dojo.lang.common");
dojo.lang.hitch=function(_cd,_ce){
if(dojo.lang.isString(_ce)){
var fcn=_cd[_ce];
}else{
var fcn=_ce;
}
return function(){
return fcn.apply(_cd,arguments);
};
};
dojo.lang.anonCtr=0;
dojo.lang.anon={};
dojo.lang.nameAnonFunc=function(_d0,_d1,_d2){
var nso=(_d1||dojo.lang.anon);
if((_d2)||((dj_global["djConfig"])&&(djConfig["slowAnonFuncLookups"]==true))){
for(var x in nso){
if(nso[x]===_d0){
return x;
}
}
}
var ret="__"+dojo.lang.anonCtr++;
while(typeof nso[ret]!="undefined"){
ret="__"+dojo.lang.anonCtr++;
}
nso[ret]=_d0;
return ret;
};
dojo.lang.forward=function(_d6){
return function(){
return this[_d6].apply(this,arguments);
};
};
dojo.lang.curry=function(ns,_d8){
var _d9=[];
ns=ns||dj_global;
if(dojo.lang.isString(_d8)){
_d8=ns[_d8];
}
for(var x=2;x<arguments.length;x++){
_d9.push(arguments[x]);
}
var _db=(_d8["__preJoinArity"]||_d8.length)-_d9.length;
function gather(_dc,_dd,_de){
var _df=_de;
var _e0=_dd.slice(0);
for(var x=0;x<_dc.length;x++){
_e0.push(_dc[x]);
}
_de=_de-_dc.length;
if(_de<=0){
var res=_d8.apply(ns,_e0);
_de=_df;
return res;
}else{
return function(){
return gather(arguments,_e0,_de);
};
}
}
return gather([],_d9,_db);
};
dojo.lang.curryArguments=function(ns,_e4,_e5,_e6){
var _e7=[];
var x=_e6||0;
for(x=_e6;x<_e5.length;x++){
_e7.push(_e5[x]);
}
return dojo.lang.curry.apply(dojo.lang,[ns,_e4].concat(_e7));
};
dojo.lang.tryThese=function(){
for(var x=0;x<arguments.length;x++){
try{
if(typeof arguments[x]=="function"){
var ret=(arguments[x]());
if(ret){
return ret;
}
}
}
catch(e){
dojo.debug(e);
}
}
};
dojo.lang.delayThese=function(_eb,cb,_ed,_ee){
if(!_eb.length){
if(typeof _ee=="function"){
_ee();
}
return;
}
if((typeof _ed=="undefined")&&(typeof cb=="number")){
_ed=cb;
cb=function(){
};
}else{
if(!cb){
cb=function(){
};
if(!_ed){
_ed=0;
}
}
}
setTimeout(function(){
(_eb.shift())();
cb();
dojo.lang.delayThese(_eb,cb,_ed,_ee);
},_ed);
};
dojo.provide("dojo.lang.array");
dojo.require("dojo.lang.common");
dojo.lang.has=function(obj,_f0){
try{
return (typeof obj[_f0]!="undefined");
}
catch(e){
return false;
}
};
dojo.lang.isEmpty=function(obj){
if(dojo.lang.isObject(obj)){
var tmp={};
var _f3=0;
for(var x in obj){
if(obj[x]&&(!tmp[x])){
_f3++;
break;
}
}
return (_f3==0);
}else{
if(dojo.lang.isArrayLike(obj)||dojo.lang.isString(obj)){
return obj.length==0;
}
}
};
dojo.lang.map=function(arr,obj,_f7){
var _f8=dojo.lang.isString(arr);
if(_f8){
arr=arr.split("");
}
if(dojo.lang.isFunction(obj)&&(!_f7)){
_f7=obj;
obj=dj_global;
}else{
if(dojo.lang.isFunction(obj)&&_f7){
var _f9=obj;
obj=_f7;
_f7=_f9;
}
}
if(Array.map){
var _fa=Array.map(arr,_f7,obj);
}else{
var _fa=[];
for(var i=0;i<arr.length;++i){
_fa.push(_f7.call(obj,arr[i]));
}
}
if(_f8){
return _fa.join("");
}else{
return _fa;
}
};
dojo.lang.forEach=function(_fc,_fd,_fe){
if(dojo.lang.isString(_fc)){
_fc=_fc.split("");
}
if(Array.forEach){
Array.forEach(_fc,_fd,_fe);
}else{
if(!_fe){
_fe=dj_global;
}
for(var i=0,l=_fc.length;i<l;i++){
_fd.call(_fe,_fc[i],i,_fc);
}
}
};
dojo.lang._everyOrSome=function(_100,arr,_102,_103){
if(dojo.lang.isString(arr)){
arr=arr.split("");
}
if(Array.every){
return Array[(_100)?"every":"some"](arr,_102,_103);
}else{
if(!_103){
_103=dj_global;
}
for(var i=0,l=arr.length;i<l;i++){
var _105=_102.call(_103,arr[i],i,arr);
if((_100)&&(!_105)){
return false;
}else{
if((!_100)&&(_105)){
return true;
}
}
}
return (_100)?true:false;
}
};
dojo.lang.every=function(arr,_107,_108){
return this._everyOrSome(true,arr,_107,_108);
};
dojo.lang.some=function(arr,_10a,_10b){
return this._everyOrSome(false,arr,_10a,_10b);
};
dojo.lang.filter=function(arr,_10d,_10e){
var _10f=dojo.lang.isString(arr);
if(_10f){
arr=arr.split("");
}
if(Array.filter){
var _110=Array.filter(arr,_10d,_10e);
}else{
if(!_10e){
if(arguments.length>=3){
dojo.raise("thisObject doesn't exist!");
}
_10e=dj_global;
}
var _110=[];
for(var i=0;i<arr.length;i++){
if(_10d.call(_10e,arr[i],i,arr)){
_110.push(arr[i]);
}
}
}
if(_10f){
return _110.join("");
}else{
return _110;
}
};
dojo.lang.unnest=function(){
var out=[];
for(var i=0;i<arguments.length;i++){
if(dojo.lang.isArrayLike(arguments[i])){
var add=dojo.lang.unnest.apply(this,arguments[i]);
out=out.concat(add);
}else{
out.push(arguments[i]);
}
}
return out;
};
dojo.lang.toArray=function(_115,_116){
var _117=[];
for(var i=_116||0;i<_115.length;i++){
_117.push(_115[i]);
}
return _117;
};
dojo.provide("dojo.dom");
dojo.require("dojo.lang.array");
dojo.dom.ELEMENT_NODE=1;
dojo.dom.ATTRIBUTE_NODE=2;
dojo.dom.TEXT_NODE=3;
dojo.dom.CDATA_SECTION_NODE=4;
dojo.dom.ENTITY_REFERENCE_NODE=5;
dojo.dom.ENTITY_NODE=6;
dojo.dom.PROCESSING_INSTRUCTION_NODE=7;
dojo.dom.COMMENT_NODE=8;
dojo.dom.DOCUMENT_NODE=9;
dojo.dom.DOCUMENT_TYPE_NODE=10;
dojo.dom.DOCUMENT_FRAGMENT_NODE=11;
dojo.dom.NOTATION_NODE=12;
dojo.dom.dojoml="http://www.dojotoolkit.org/2004/dojoml";
dojo.dom.xmlns={svg:"http://www.w3.org/2000/svg",smil:"http://www.w3.org/2001/SMIL20/",mml:"http://www.w3.org/1998/Math/MathML",cml:"http://www.xml-cml.org",xlink:"http://www.w3.org/1999/xlink",xhtml:"http://www.w3.org/1999/xhtml",xul:"http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul",xbl:"http://www.mozilla.org/xbl",fo:"http://www.w3.org/1999/XSL/Format",xsl:"http://www.w3.org/1999/XSL/Transform",xslt:"http://www.w3.org/1999/XSL/Transform",xi:"http://www.w3.org/2001/XInclude",xforms:"http://www.w3.org/2002/01/xforms",saxon:"http://icl.com/saxon",xalan:"http://xml.apache.org/xslt",xsd:"http://www.w3.org/2001/XMLSchema",dt:"http://www.w3.org/2001/XMLSchema-datatypes",xsi:"http://www.w3.org/2001/XMLSchema-instance",rdf:"http://www.w3.org/1999/02/22-rdf-syntax-ns#",rdfs:"http://www.w3.org/2000/01/rdf-schema#",dc:"http://purl.org/dc/elements/1.1/",dcq:"http://purl.org/dc/qualifiers/1.0","soap-env":"http://schemas.xmlsoap.org/soap/envelope/",wsdl:"http://schemas.xmlsoap.org/wsdl/",AdobeExtensions:"http://ns.adobe.com/AdobeSVGViewerExtensions/3.0/"};
dojo.dom.isNode=function(wh){
if(typeof Element=="object"){
try{
return wh instanceof Element;
}
catch(E){
}
}else{
return wh&&!isNaN(wh.nodeType);
}
};
dojo.dom.getTagName=function(node){
dojo.deprecated("dojo.dom.getTagName","use node.tagName instead","0.4");
var _11b=node.tagName;
if(_11b.substr(0,5).toLowerCase()!="dojo:"){
if(_11b.substr(0,4).toLowerCase()=="dojo"){
return "dojo:"+_11b.substring(4).toLowerCase();
}
var djt=node.getAttribute("dojoType")||node.getAttribute("dojotype");
if(djt){
return "dojo:"+djt.toLowerCase();
}
if((node.getAttributeNS)&&(node.getAttributeNS(this.dojoml,"type"))){
return "dojo:"+node.getAttributeNS(this.dojoml,"type").toLowerCase();
}
try{
djt=node.getAttribute("dojo:type");
}
catch(e){
}
if(djt){
return "dojo:"+djt.toLowerCase();
}
if((!dj_global["djConfig"])||(!djConfig["ignoreClassNames"])){
var _11d=node.className||node.getAttribute("class");
if((_11d)&&(_11d.indexOf)&&(_11d.indexOf("dojo-")!=-1)){
var _11e=_11d.split(" ");
for(var x=0;x<_11e.length;x++){
if((_11e[x].length>5)&&(_11e[x].indexOf("dojo-")>=0)){
return "dojo:"+_11e[x].substr(5).toLowerCase();
}
}
}
}
}
return _11b.toLowerCase();
};
dojo.dom.getUniqueId=function(){
do{
var id="dj_unique_"+(++arguments.callee._idIncrement);
}while(document.getElementById(id));
return id;
};
dojo.dom.getUniqueId._idIncrement=0;
dojo.dom.firstElement=dojo.dom.getFirstChildElement=function(_121,_122){
var node=_121.firstChild;
while(node&&node.nodeType!=dojo.dom.ELEMENT_NODE){
node=node.nextSibling;
}
if(_122&&node&&node.tagName&&node.tagName.toLowerCase()!=_122.toLowerCase()){
node=dojo.dom.nextElement(node,_122);
}
return node;
};
dojo.dom.lastElement=dojo.dom.getLastChildElement=function(_124,_125){
var node=_124.lastChild;
while(node&&node.nodeType!=dojo.dom.ELEMENT_NODE){
node=node.previousSibling;
}
if(_125&&node&&node.tagName&&node.tagName.toLowerCase()!=_125.toLowerCase()){
node=dojo.dom.prevElement(node,_125);
}
return node;
};
dojo.dom.nextElement=dojo.dom.getNextSiblingElement=function(node,_128){
if(!node){
return null;
}
do{
node=node.nextSibling;
}while(node&&node.nodeType!=dojo.dom.ELEMENT_NODE);
if(node&&_128&&_128.toLowerCase()!=node.tagName.toLowerCase()){
return dojo.dom.nextElement(node,_128);
}
return node;
};
dojo.dom.prevElement=dojo.dom.getPreviousSiblingElement=function(node,_12a){
if(!node){
return null;
}
if(_12a){
_12a=_12a.toLowerCase();
}
do{
node=node.previousSibling;
}while(node&&node.nodeType!=dojo.dom.ELEMENT_NODE);
if(node&&_12a&&_12a.toLowerCase()!=node.tagName.toLowerCase()){
return dojo.dom.prevElement(node,_12a);
}
return node;
};
dojo.dom.moveChildren=function(_12b,_12c,trim){
var _12e=0;
if(trim){
while(_12b.hasChildNodes()&&_12b.firstChild.nodeType==dojo.dom.TEXT_NODE){
_12b.removeChild(_12b.firstChild);
}
while(_12b.hasChildNodes()&&_12b.lastChild.nodeType==dojo.dom.TEXT_NODE){
_12b.removeChild(_12b.lastChild);
}
}
while(_12b.hasChildNodes()){
_12c.appendChild(_12b.firstChild);
_12e++;
}
return _12e;
};
dojo.dom.copyChildren=function(_12f,_130,trim){
var _132=_12f.cloneNode(true);
return this.moveChildren(_132,_130,trim);
};
dojo.dom.removeChildren=function(node){
var _134=node.childNodes.length;
while(node.hasChildNodes()){
node.removeChild(node.firstChild);
}
return _134;
};
dojo.dom.replaceChildren=function(node,_136){
dojo.dom.removeChildren(node);
node.appendChild(_136);
};
dojo.dom.removeNode=function(node){
if(node&&node.parentNode){
return node.parentNode.removeChild(node);
}
};
dojo.dom.getAncestors=function(node,_139,_13a){
var _13b=[];
var _13c=dojo.lang.isFunction(_139);
while(node){
if(!_13c||_139(node)){
_13b.push(node);
}
if(_13a&&_13b.length>0){
return _13b[0];
}
node=node.parentNode;
}
if(_13a){
return null;
}
return _13b;
};
dojo.dom.getAncestorsByTag=function(node,tag,_13f){
tag=tag.toLowerCase();
return dojo.dom.getAncestors(node,function(el){
return ((el.tagName)&&(el.tagName.toLowerCase()==tag));
},_13f);
};
dojo.dom.getFirstAncestorByTag=function(node,tag){
return dojo.dom.getAncestorsByTag(node,tag,true);
};
dojo.dom.isDescendantOf=function(node,_144,_145){
if(_145&&node){
node=node.parentNode;
}
while(node){
if(node==_144){
return true;
}
node=node.parentNode;
}
return false;
};
dojo.dom.innerXML=function(node){
if(node.innerXML){
return node.innerXML;
}else{
if(node.xml){
return node.xml;
}else{
if(typeof XMLSerializer!="undefined"){
return (new XMLSerializer()).serializeToString(node);
}
}
}
};
dojo.dom.createDocument=function(){
var doc=null;
if(!dj_undef("ActiveXObject")){
var _148=["MSXML2","Microsoft","MSXML","MSXML3"];
for(var i=0;i<_148.length;i++){
try{
doc=new ActiveXObject(_148[i]+".XMLDOM");
}
catch(e){
}
if(doc){
break;
}
}
}else{
if((document.implementation)&&(document.implementation.createDocument)){
doc=document.implementation.createDocument("","",null);
}
}
return doc;
};
dojo.dom.createDocumentFromText=function(str,_14b){
if(!_14b){
_14b="text/xml";
}
if(!dj_undef("DOMParser")){
var _14c=new DOMParser();
return _14c.parseFromString(str,_14b);
}else{
if(!dj_undef("ActiveXObject")){
var _14d=dojo.dom.createDocument();
if(_14d){
_14d.async=false;
_14d.loadXML(str);
return _14d;
}else{
dojo.debug("toXml didn't work?");
}
}else{
if(document.createElement){
var tmp=document.createElement("xml");
tmp.innerHTML=str;
if(document.implementation&&document.implementation.createDocument){
var _14f=document.implementation.createDocument("foo","",null);
for(var i=0;i<tmp.childNodes.length;i++){
_14f.importNode(tmp.childNodes.item(i),true);
}
return _14f;
}
return ((tmp.document)&&(tmp.document.firstChild?tmp.document.firstChild:tmp));
}
}
}
return null;
};
dojo.dom.prependChild=function(node,_152){
if(_152.firstChild){
_152.insertBefore(node,_152.firstChild);
}else{
_152.appendChild(node);
}
return true;
};
dojo.dom.insertBefore=function(node,ref,_155){
if(_155!=true&&(node===ref||node.nextSibling===ref)){
return false;
}
var _156=ref.parentNode;
_156.insertBefore(node,ref);
return true;
};
dojo.dom.insertAfter=function(node,ref,_159){
var pn=ref.parentNode;
if(ref==pn.lastChild){
if((_159!=true)&&(node===ref)){
return false;
}
pn.appendChild(node);
}else{
return this.insertBefore(node,ref.nextSibling,_159);
}
return true;
};
dojo.dom.insertAtPosition=function(node,ref,_15d){
if((!node)||(!ref)||(!_15d)){
return false;
}
switch(_15d.toLowerCase()){
case "before":
return dojo.dom.insertBefore(node,ref);
case "after":
return dojo.dom.insertAfter(node,ref);
case "first":
if(ref.firstChild){
return dojo.dom.insertBefore(node,ref.firstChild);
}else{
ref.appendChild(node);
return true;
}
break;
default:
ref.appendChild(node);
return true;
}
};
dojo.dom.insertAtIndex=function(node,_15f,_160){
var _161=_15f.childNodes;
if(!_161.length){
_15f.appendChild(node);
return true;
}
var _162=null;
for(var i=0;i<_161.length;i++){
var _164=_161.item(i)["getAttribute"]?parseInt(_161.item(i).getAttribute("dojoinsertionindex")):-1;
if(_164<_160){
_162=_161.item(i);
}
}
if(_162){
return dojo.dom.insertAfter(node,_162);
}else{
return dojo.dom.insertBefore(node,_161.item(0));
}
};
dojo.dom.textContent=function(node,text){
if(text){
dojo.dom.replaceChildren(node,document.createTextNode(text));
return text;
}else{
var _167="";
if(node==null){
return _167;
}
for(var i=0;i<node.childNodes.length;i++){
switch(node.childNodes[i].nodeType){
case 1:
case 5:
_167+=dojo.dom.textContent(node.childNodes[i]);
break;
case 3:
case 2:
case 4:
_167+=node.childNodes[i].nodeValue;
break;
default:
break;
}
}
return _167;
}
};
dojo.dom.collectionToArray=function(_169){
dojo.deprecated("dojo.dom.collectionToArray","use dojo.lang.toArray instead","0.4");
return dojo.lang.toArray(_169);
};
dojo.dom.hasParent=function(node){
return node&&node.parentNode&&dojo.dom.isNode(node.parentNode);
};
dojo.dom.isTag=function(node){
if(node&&node.tagName){
var arr=dojo.lang.toArray(arguments,1);
return arr[dojo.lang.find(node.tagName,arr)]||"";
}
return "";
};
dojo.provide("dojo.graphics.color");
dojo.require("dojo.lang.array");
dojo.graphics.color.Color=function(r,g,b,a){
if(dojo.lang.isArray(r)){
this.r=r[0];
this.g=r[1];
this.b=r[2];
this.a=r[3]||1;
}else{
if(dojo.lang.isString(r)){
var rgb=dojo.graphics.color.extractRGB(r);
this.r=rgb[0];
this.g=rgb[1];
this.b=rgb[2];
this.a=g||1;
}else{
if(r instanceof dojo.graphics.color.Color){
this.r=r.r;
this.b=r.b;
this.g=r.g;
this.a=r.a;
}else{
this.r=r;
this.g=g;
this.b=b;
this.a=a;
}
}
}
};
dojo.graphics.color.Color.fromArray=function(arr){
return new dojo.graphics.color.Color(arr[0],arr[1],arr[2],arr[3]);
};
dojo.lang.extend(dojo.graphics.color.Color,{toRgb:function(_173){
if(_173){
return this.toRgba();
}else{
return [this.r,this.g,this.b];
}
},toRgba:function(){
return [this.r,this.g,this.b,this.a];
},toHex:function(){
return dojo.graphics.color.rgb2hex(this.toRgb());
},toCss:function(){
return "rgb("+this.toRgb().join()+")";
},toString:function(){
return this.toHex();
},blend:function(_174,_175){
return dojo.graphics.color.blend(this.toRgb(),new dojo.graphics.color.Color(_174).toRgb(),_175);
}});
dojo.graphics.color.named={white:[255,255,255],black:[0,0,0],red:[255,0,0],green:[0,255,0],blue:[0,0,255],navy:[0,0,128],gray:[128,128,128],silver:[192,192,192]};
dojo.graphics.color.blend=function(a,b,_178){
if(typeof a=="string"){
return dojo.graphics.color.blendHex(a,b,_178);
}
if(!_178){
_178=0;
}else{
if(_178>1){
_178=1;
}else{
if(_178<-1){
_178=-1;
}
}
}
var c=new Array(3);
for(var i=0;i<3;i++){
var half=Math.abs(a[i]-b[i])/2;
c[i]=Math.floor(Math.min(a[i],b[i])+half+(half*_178));
}
return c;
};
dojo.graphics.color.blendHex=function(a,b,_17e){
return dojo.graphics.color.rgb2hex(dojo.graphics.color.blend(dojo.graphics.color.hex2rgb(a),dojo.graphics.color.hex2rgb(b),_17e));
};
dojo.graphics.color.extractRGB=function(_17f){
var hex="0123456789abcdef";
_17f=_17f.toLowerCase();
if(_17f.indexOf("rgb")==0){
var _181=_17f.match(/rgba*\((\d+), *(\d+), *(\d+)/i);
var ret=_181.splice(1,3);
return ret;
}else{
var _183=dojo.graphics.color.hex2rgb(_17f);
if(_183){
return _183;
}else{
return dojo.graphics.color.named[_17f]||[255,255,255];
}
}
};
dojo.graphics.color.hex2rgb=function(hex){
var _185="0123456789ABCDEF";
var rgb=new Array(3);
if(hex.indexOf("#")==0){
hex=hex.substring(1);
}
hex=hex.toUpperCase();
if(hex.replace(new RegExp("["+_185+"]","g"),"")!=""){
return null;
}
if(hex.length==3){
rgb[0]=hex.charAt(0)+hex.charAt(0);
rgb[1]=hex.charAt(1)+hex.charAt(1);
rgb[2]=hex.charAt(2)+hex.charAt(2);
}else{
rgb[0]=hex.substring(0,2);
rgb[1]=hex.substring(2,4);
rgb[2]=hex.substring(4);
}
for(var i=0;i<rgb.length;i++){
rgb[i]=_185.indexOf(rgb[i].charAt(0))*16+_185.indexOf(rgb[i].charAt(1));
}
return rgb;
};
dojo.graphics.color.rgb2hex=function(r,g,b){
if(dojo.lang.isArray(r)){
g=r[1]||0;
b=r[2]||0;
r=r[0]||0;
}
var ret=dojo.lang.map([r,g,b],function(x){
x=new Number(x);
var s=x.toString(16);
while(s.length<2){
s="0"+s;
}
return s;
});
ret.unshift("#");
return ret.join("");
};
dojo.provide("dojo.uri.Uri");
dojo.uri=new function(){
this.joinPath=function(){
var arr=[];
for(var i=0;i<arguments.length;i++){
arr.push(arguments[i]);
}
return arr.join("/").replace(/\/{2,}/g,"/").replace(/((https*|ftps*):)/i,"$1/");
};
this.dojoUri=function(uri){
return new dojo.uri.Uri(dojo.hostenv.getBaseScriptUri(),uri);
};
this.Uri=function(){
var uri=arguments[0];
for(var i=1;i<arguments.length;i++){
if(!arguments[i]){
continue;
}
var _193=new dojo.uri.Uri(arguments[i].toString());
var _194=new dojo.uri.Uri(uri.toString());
if(_193.path==""&&_193.scheme==null&&_193.authority==null&&_193.query==null){
if(_193.fragment!=null){
_194.fragment=_193.fragment;
}
_193=_194;
}else{
if(_193.scheme==null){
_193.scheme=_194.scheme;
if(_193.authority==null){
_193.authority=_194.authority;
if(_193.path.charAt(0)!="/"){
var path=_194.path.substring(0,_194.path.lastIndexOf("/")+1)+_193.path;
var segs=path.split("/");
for(var j=0;j<segs.length;j++){
if(segs[j]=="."){
if(j==segs.length-1){
segs[j]="";
}else{
segs.splice(j,1);
j--;
}
}else{
if(j>0&&!(j==1&&segs[0]=="")&&segs[j]==".."&&segs[j-1]!=".."){
if(j==segs.length-1){
segs.splice(j,1);
segs[j-1]="";
}else{
segs.splice(j-1,2);
j-=2;
}
}
}
}
_193.path=segs.join("/");
}
}
}
}
uri="";
if(_193.scheme!=null){
uri+=_193.scheme+":";
}
if(_193.authority!=null){
uri+="//"+_193.authority;
}
uri+=_193.path;
if(_193.query!=null){
uri+="?"+_193.query;
}
if(_193.fragment!=null){
uri+="#"+_193.fragment;
}
}
this.uri=uri.toString();
var _198="^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?$";
var r=this.uri.match(new RegExp(_198));
this.scheme=r[2]||(r[1]?"":null);
this.authority=r[4]||(r[3]?"":null);
this.path=r[5];
this.query=r[7]||(r[6]?"":null);
this.fragment=r[9]||(r[8]?"":null);
if(this.authority!=null){
_198="^((([^:]+:)?([^@]+))@)?([^:]*)(:([0-9]+))?$";
r=this.authority.match(new RegExp(_198));
this.user=r[3]||null;
this.password=r[4]||null;
this.host=r[5];
this.port=r[7]||null;
}
this.toString=function(){
return this.uri;
};
};
};
dojo.provide("dojo.style");
dojo.require("dojo.graphics.color");
dojo.require("dojo.uri.Uri");
dojo.require("dojo.lang.common");
(function(){
var h=dojo.render.html;
var ds=dojo.style;
var db=document["body"]||document["documentElement"];
ds.boxSizing={MARGIN_BOX:"margin-box",BORDER_BOX:"border-box",PADDING_BOX:"padding-box",CONTENT_BOX:"content-box"};
var bs=ds.boxSizing;
ds.getBoxSizing=function(node){
if((h.ie)||(h.opera)){
var cm=document["compatMode"];
if((cm=="BackCompat")||(cm=="QuirksMode")){
return bs.BORDER_BOX;
}else{
return bs.CONTENT_BOX;
}
}else{
if(arguments.length==0){
node=document.documentElement;
}
var _1a0=ds.getStyle(node,"-moz-box-sizing");
if(!_1a0){
_1a0=ds.getStyle(node,"box-sizing");
}
return (_1a0?_1a0:bs.CONTENT_BOX);
}
};
ds.isBorderBox=function(node){
return (ds.getBoxSizing(node)==bs.BORDER_BOX);
};
ds.getUnitValue=function(node,_1a3,_1a4){
var s=ds.getComputedStyle(node,_1a3);
if((!s)||((s=="auto")&&(_1a4))){
return {value:0,units:"px"};
}
if(dojo.lang.isUndefined(s)){
return ds.getUnitValue.bad;
}
var _1a6=s.match(/(\-?[\d.]+)([a-z%]*)/i);
if(!_1a6){
return ds.getUnitValue.bad;
}
return {value:Number(_1a6[1]),units:_1a6[2].toLowerCase()};
};
ds.getUnitValue.bad={value:NaN,units:""};
ds.getPixelValue=function(node,_1a8,_1a9){
var _1aa=ds.getUnitValue(node,_1a8,_1a9);
if(isNaN(_1aa.value)){
return 0;
}
if((_1aa.value)&&(_1aa.units!="px")){
return NaN;
}
return _1aa.value;
};
ds.getNumericStyle=function(){
dojo.deprecated("dojo.(style|html).getNumericStyle","in favor of dojo.(style|html).getPixelValue","0.4");
return ds.getPixelValue.apply(this,arguments);
};
ds.setPositivePixelValue=function(node,_1ac,_1ad){
if(isNaN(_1ad)){
return false;
}
node.style[_1ac]=Math.max(0,_1ad)+"px";
return true;
};
ds._sumPixelValues=function(node,_1af,_1b0){
var _1b1=0;
for(var x=0;x<_1af.length;x++){
_1b1+=ds.getPixelValue(node,_1af[x],_1b0);
}
return _1b1;
};
ds.isPositionAbsolute=function(node){
return (ds.getComputedStyle(node,"position")=="absolute");
};
ds.getBorderExtent=function(node,side){
return (ds.getStyle(node,"border-"+side+"-style")=="none"?0:ds.getPixelValue(node,"border-"+side+"-width"));
};
ds.getMarginWidth=function(node){
return ds._sumPixelValues(node,["margin-left","margin-right"],ds.isPositionAbsolute(node));
};
ds.getBorderWidth=function(node){
return ds.getBorderExtent(node,"left")+ds.getBorderExtent(node,"right");
};
ds.getPaddingWidth=function(node){
return ds._sumPixelValues(node,["padding-left","padding-right"],true);
};
ds.getPadBorderWidth=function(node){
return ds.getPaddingWidth(node)+ds.getBorderWidth(node);
};
ds.getContentBoxWidth=function(node){
node=dojo.byId(node);
return node.offsetWidth-ds.getPadBorderWidth(node);
};
ds.getBorderBoxWidth=function(node){
node=dojo.byId(node);
return node.offsetWidth;
};
ds.getMarginBoxWidth=function(node){
return ds.getInnerWidth(node)+ds.getMarginWidth(node);
};
ds.setContentBoxWidth=function(node,_1be){
node=dojo.byId(node);
if(ds.isBorderBox(node)){
_1be+=ds.getPadBorderWidth(node);
}
return ds.setPositivePixelValue(node,"width",_1be);
};
ds.setMarginBoxWidth=function(node,_1c0){
node=dojo.byId(node);
if(!ds.isBorderBox(node)){
_1c0-=ds.getPadBorderWidth(node);
}
_1c0-=ds.getMarginWidth(node);
return ds.setPositivePixelValue(node,"width",_1c0);
};
ds.getContentWidth=ds.getContentBoxWidth;
ds.getInnerWidth=ds.getBorderBoxWidth;
ds.getOuterWidth=ds.getMarginBoxWidth;
ds.setContentWidth=ds.setContentBoxWidth;
ds.setOuterWidth=ds.setMarginBoxWidth;
ds.getMarginHeight=function(node){
return ds._sumPixelValues(node,["margin-top","margin-bottom"],ds.isPositionAbsolute(node));
};
ds.getBorderHeight=function(node){
return ds.getBorderExtent(node,"top")+ds.getBorderExtent(node,"bottom");
};
ds.getPaddingHeight=function(node){
return ds._sumPixelValues(node,["padding-top","padding-bottom"],true);
};
ds.getPadBorderHeight=function(node){
return ds.getPaddingHeight(node)+ds.getBorderHeight(node);
};
ds.getContentBoxHeight=function(node){
node=dojo.byId(node);
return node.offsetHeight-ds.getPadBorderHeight(node);
};
ds.getBorderBoxHeight=function(node){
node=dojo.byId(node);
return node.offsetHeight;
};
ds.getMarginBoxHeight=function(node){
return ds.getInnerHeight(node)+ds.getMarginHeight(node);
};
ds.setContentBoxHeight=function(node,_1c9){
node=dojo.byId(node);
if(ds.isBorderBox(node)){
_1c9+=ds.getPadBorderHeight(node);
}
return ds.setPositivePixelValue(node,"height",_1c9);
};
ds.setMarginBoxHeight=function(node,_1cb){
node=dojo.byId(node);
if(!ds.isBorderBox(node)){
_1cb-=ds.getPadBorderHeight(node);
}
_1cb-=ds.getMarginHeight(node);
return ds.setPositivePixelValue(node,"height",_1cb);
};
ds.getContentHeight=ds.getContentBoxHeight;
ds.getInnerHeight=ds.getBorderBoxHeight;
ds.getOuterHeight=ds.getMarginBoxHeight;
ds.setContentHeight=ds.setContentBoxHeight;
ds.setOuterHeight=ds.setMarginBoxHeight;
ds.getAbsolutePosition=ds.abs=function(node,_1cd){
node=dojo.byId(node);
var ret=[];
ret.x=ret.y=0;
var st=dojo.html.getScrollTop();
var sl=dojo.html.getScrollLeft();
if(h.ie){
with(node.getBoundingClientRect()){
ret.x=left-2;
ret.y=top-2;
}
}else{
if(document.getBoxObjectFor){
var bo=document.getBoxObjectFor(node);
ret.x=bo.x-ds.sumAncestorProperties(node,"scrollLeft");
ret.y=bo.y-ds.sumAncestorProperties(node,"scrollTop");
}else{
if(node["offsetParent"]){
var _1d2;
if((h.safari)&&(node.style.getPropertyValue("position")=="absolute")&&(node.parentNode==db)){
_1d2=db;
}else{
_1d2=db.parentNode;
}
if(node.parentNode!=db){
var nd=node;
if(window.opera){
nd=db;
}
ret.x-=ds.sumAncestorProperties(nd,"scrollLeft");
ret.y-=ds.sumAncestorProperties(nd,"scrollTop");
}
do{
var n=node["offsetLeft"];
ret.x+=isNaN(n)?0:n;
var m=node["offsetTop"];
ret.y+=isNaN(m)?0:m;
node=node.offsetParent;
}while((node!=_1d2)&&(node!=null));
}else{
if(node["x"]&&node["y"]){
ret.x+=isNaN(node.x)?0:node.x;
ret.y+=isNaN(node.y)?0:node.y;
}
}
}
}
if(_1cd){
ret.y+=st;
ret.x+=sl;
}
ret[0]=ret.x;
ret[1]=ret.y;
return ret;
};
ds.sumAncestorProperties=function(node,prop){
node=dojo.byId(node);
if(!node){
return 0;
}
var _1d8=0;
while(node){
var val=node[prop];
if(val){
_1d8+=val-0;
if(node==document.body){
break;
}
}
node=node.parentNode;
}
return _1d8;
};
ds.getTotalOffset=function(node,type,_1dc){
return ds.abs(node,_1dc)[(type=="top")?"y":"x"];
};
ds.getAbsoluteX=ds.totalOffsetLeft=function(node,_1de){
return ds.getTotalOffset(node,"left",_1de);
};
ds.getAbsoluteY=ds.totalOffsetTop=function(node,_1e0){
return ds.getTotalOffset(node,"top",_1e0);
};
ds.styleSheet=null;
ds.insertCssRule=function(_1e1,_1e2,_1e3){
if(!ds.styleSheet){
if(document.createStyleSheet){
ds.styleSheet=document.createStyleSheet();
}else{
if(document.styleSheets[0]){
ds.styleSheet=document.styleSheets[0];
}else{
return null;
}
}
}
if(arguments.length<3){
if(ds.styleSheet.cssRules){
_1e3=ds.styleSheet.cssRules.length;
}else{
if(ds.styleSheet.rules){
_1e3=ds.styleSheet.rules.length;
}else{
return null;
}
}
}
if(ds.styleSheet.insertRule){
var rule=_1e1+" { "+_1e2+" }";
return ds.styleSheet.insertRule(rule,_1e3);
}else{
if(ds.styleSheet.addRule){
return ds.styleSheet.addRule(_1e1,_1e2,_1e3);
}else{
return null;
}
}
};
ds.removeCssRule=function(_1e5){
if(!ds.styleSheet){
dojo.debug("no stylesheet defined for removing rules");
return false;
}
if(h.ie){
if(!_1e5){
_1e5=ds.styleSheet.rules.length;
ds.styleSheet.removeRule(_1e5);
}
}else{
if(document.styleSheets[0]){
if(!_1e5){
_1e5=ds.styleSheet.cssRules.length;
}
ds.styleSheet.deleteRule(_1e5);
}
}
return true;
};
ds.insertCssFile=function(URI,doc,_1e8){
if(!URI){
return;
}
if(!doc){
doc=document;
}
var _1e9=dojo.hostenv.getText(URI);
_1e9=ds.fixPathsInCssText(_1e9,URI);
if(_1e8){
var _1ea=doc.getElementsByTagName("style");
var _1eb="";
for(var i=0;i<_1ea.length;i++){
_1eb=(_1ea[i].styleSheet&&_1ea[i].styleSheet.cssText)?_1ea[i].styleSheet.cssText:_1ea[i].innerHTML;
if(_1e9==_1eb){
return;
}
}
}
var _1ed=ds.insertCssText(_1e9);
if(_1ed&&djConfig.isDebug){
_1ed.setAttribute("dbgHref",URI);
}
return _1ed;
};
ds.insertCssText=function(_1ee,doc,URI){
if(!_1ee){
return;
}
if(!doc){
doc=document;
}
if(URI){
_1ee=ds.fixPathsInCssText(_1ee,URI);
}
var _1f1=doc.createElement("style");
_1f1.setAttribute("type","text/css");
var head=doc.getElementsByTagName("head")[0];
if(!head){
dojo.debug("No head tag in document, aborting styles");
return;
}else{
head.appendChild(_1f1);
}
if(_1f1.styleSheet){
_1f1.styleSheet.cssText=_1ee;
}else{
var _1f3=doc.createTextNode(_1ee);
_1f1.appendChild(_1f3);
}
return _1f1;
};
ds.fixPathsInCssText=function(_1f4,URI){
if(!_1f4||!URI){
return;
}
var pos=0;
var str="";
var url="";
while(pos!=-1){
pos=0;
url="";
pos=_1f4.indexOf("url(",pos);
if(pos<0){
break;
}
str+=_1f4.slice(0,pos+4);
_1f4=_1f4.substring(pos+4,_1f4.length);
url+=_1f4.match(/^[\t\s\w()\/.\\'"-:#=&?]*\)/)[0];
_1f4=_1f4.substring(url.length-1,_1f4.length);
url=url.replace(/^[\s\t]*(['"]?)([\w()\/.\\'"-:#=&?]*)\1[\s\t]*?\)/,"$2");
if(url.search(/(file|https?|ftps?):\/\//)==-1){
url=(new dojo.uri.Uri(URI,url).toString());
}
str+=url;
}
return str+_1f4;
};
ds.getBackgroundColor=function(node){
node=dojo.byId(node);
var _1fa;
do{
_1fa=ds.getStyle(node,"background-color");
if(_1fa.toLowerCase()=="rgba(0, 0, 0, 0)"){
_1fa="transparent";
}
if(node==document.getElementsByTagName("body")[0]){
node=null;
break;
}
node=node.parentNode;
}while(node&&dojo.lang.inArray(_1fa,["transparent",""]));
if(_1fa=="transparent"){
_1fa=[255,255,255,0];
}else{
_1fa=dojo.graphics.color.extractRGB(_1fa);
}
return _1fa;
};
ds.getComputedStyle=function(node,_1fc,_1fd){
node=dojo.byId(node);
var _1fc=ds.toSelectorCase(_1fc);
var _1fe=ds.toCamelCase(_1fc);
if(!node||!node.style){
return _1fd;
}else{
if(document.defaultView){
try{
var cs=document.defaultView.getComputedStyle(node,"");
if(cs){
return cs.getPropertyValue(_1fc);
}
}
catch(e){
if(node.style.getPropertyValue){
return node.style.getPropertyValue(_1fc);
}else{
return _1fd;
}
}
}else{
if(node.currentStyle){
return node.currentStyle[_1fe];
}
}
}
if(node.style.getPropertyValue){
return node.style.getPropertyValue(_1fc);
}else{
return _1fd;
}
};
ds.getStyleProperty=function(node,_201){
node=dojo.byId(node);
return (node&&node.style?node.style[ds.toCamelCase(_201)]:undefined);
};
ds.getStyle=function(node,_203){
var _204=ds.getStyleProperty(node,_203);
return (_204?_204:ds.getComputedStyle(node,_203));
};
ds.setStyle=function(node,_206,_207){
node=dojo.byId(node);
if(node&&node.style){
var _208=ds.toCamelCase(_206);
node.style[_208]=_207;
}
};
ds.toCamelCase=function(_209){
var arr=_209.split("-"),cc=arr[0];
for(var i=1;i<arr.length;i++){
cc+=arr[i].charAt(0).toUpperCase()+arr[i].substring(1);
}
return cc;
};
ds.toSelectorCase=function(_20c){
return _20c.replace(/([A-Z])/g,"-$1").toLowerCase();
};
ds.setOpacity=function setOpacity(node,_20e,_20f){
node=dojo.byId(node);
if(!_20f){
if(_20e>=1){
if(h.ie){
ds.clearOpacity(node);
return;
}else{
_20e=0.999999;
}
}else{
if(_20e<0){
_20e=0;
}
}
}
if(h.ie){
if(node.nodeName.toLowerCase()=="tr"){
var tds=node.getElementsByTagName("td");
for(var x=0;x<tds.length;x++){
tds[x].style.filter="Alpha(Opacity="+_20e*100+")";
}
}
node.style.filter="Alpha(Opacity="+_20e*100+")";
}else{
if(h.moz){
node.style.opacity=_20e;
node.style.MozOpacity=_20e;
}else{
if(h.safari){
node.style.opacity=_20e;
node.style.KhtmlOpacity=_20e;
}else{
node.style.opacity=_20e;
}
}
}
};
ds.getOpacity=function getOpacity(node){
node=dojo.byId(node);
if(h.ie){
var opac=(node.filters&&node.filters.alpha&&typeof node.filters.alpha.opacity=="number"?node.filters.alpha.opacity:100)/100;
}else{
var opac=node.style.opacity||node.style.MozOpacity||node.style.KhtmlOpacity||1;
}
return opac>=0.999999?1:Number(opac);
};
ds.clearOpacity=function clearOpacity(node){
node=dojo.byId(node);
var ns=node.style;
if(h.ie){
try{
if(node.filters&&node.filters.alpha){
ns.filter="";
}
}
catch(e){
}
}else{
if(h.moz){
ns.opacity=1;
ns.MozOpacity=1;
}else{
if(h.safari){
ns.opacity=1;
ns.KhtmlOpacity=1;
}else{
ns.opacity=1;
}
}
}
};
ds.setStyleAttributes=function(node,_217){
var _218={"opacity":dojo.style.setOpacity,"content-height":dojo.style.setContentHeight,"content-width":dojo.style.setContentWidth,"outer-height":dojo.style.setOuterHeight,"outer-width":dojo.style.setOuterWidth};
var _219=_217.replace(/(;)?\s*$/,"").split(";");
for(var i=0;i<_219.length;i++){
var _21b=_219[i].split(":");
var name=_21b[0].replace(/\s*$/,"").replace(/^\s*/,"").toLowerCase();
var _21d=_21b[1].replace(/\s*$/,"").replace(/^\s*/,"");
if(dojo.lang.has(_218,name)){
_218[name](node,_21d);
}else{
node.style[dojo.style.toCamelCase(name)]=_21d;
}
}
};
ds._toggle=function(node,_21f,_220){
node=dojo.byId(node);
_220(node,!_21f(node));
return _21f(node);
};
ds.show=function(node){
node=dojo.byId(node);
if(ds.getStyleProperty(node,"display")=="none"){
ds.setStyle(node,"display",(node.dojoDisplayCache||""));
node.dojoDisplayCache=undefined;
}
};
ds.hide=function(node){
node=dojo.byId(node);
if(typeof node["dojoDisplayCache"]=="undefined"){
var d=ds.getStyleProperty(node,"display");
if(d!="none"){
node.dojoDisplayCache=d;
}
}
ds.setStyle(node,"display","none");
};
ds.setShowing=function(node,_225){
ds[(_225?"show":"hide")](node);
};
ds.isShowing=function(node){
return (ds.getStyleProperty(node,"display")!="none");
};
ds.toggleShowing=function(node){
return ds._toggle(node,ds.isShowing,ds.setShowing);
};
ds.displayMap={tr:"",td:"",th:"",img:"inline",span:"inline",input:"inline",button:"inline"};
ds.suggestDisplayByTagName=function(node){
node=dojo.byId(node);
if(node&&node.tagName){
var tag=node.tagName.toLowerCase();
return (tag in ds.displayMap?ds.displayMap[tag]:"block");
}
};
ds.setDisplay=function(node,_22b){
ds.setStyle(node,"display",(dojo.lang.isString(_22b)?_22b:(_22b?ds.suggestDisplayByTagName(node):"none")));
};
ds.isDisplayed=function(node){
return (ds.getComputedStyle(node,"display")!="none");
};
ds.toggleDisplay=function(node){
return ds._toggle(node,ds.isDisplayed,ds.setDisplay);
};
ds.setVisibility=function(node,_22f){
ds.setStyle(node,"visibility",(dojo.lang.isString(_22f)?_22f:(_22f?"visible":"hidden")));
};
ds.isVisible=function(node){
return (ds.getComputedStyle(node,"visibility")!="hidden");
};
ds.toggleVisibility=function(node){
return ds._toggle(node,ds.isVisible,ds.setVisibility);
};
ds.toCoordinateArray=function(_232,_233){
if(dojo.lang.isArray(_232)){
while(_232.length<4){
_232.push(0);
}
while(_232.length>4){
_232.pop();
}
var ret=_232;
}else{
var node=dojo.byId(_232);
var pos=ds.getAbsolutePosition(node,_233);
var ret=[pos.x,pos.y,ds.getBorderBoxWidth(node),ds.getBorderBoxHeight(node)];
}
ret.x=ret[0];
ret.y=ret[1];
ret.w=ret[2];
ret.h=ret[3];
return ret;
};
})();
dojo.provide("dojo.string.common");
dojo.require("dojo.string");
dojo.string.trim=function(str,wh){
if(!str.replace){
return str;
}
if(!str.length){
return str;
}
var re=(wh>0)?(/^\s+/):(wh<0)?(/\s+$/):(/^\s+|\s+$/g);
return str.replace(re,"");
};
dojo.string.trimStart=function(str){
return dojo.string.trim(str,1);
};
dojo.string.trimEnd=function(str){
return dojo.string.trim(str,-1);
};
dojo.string.repeat=function(str,_23d,_23e){
var out="";
for(var i=0;i<_23d;i++){
out+=str;
if(_23e&&i<_23d-1){
out+=_23e;
}
}
return out;
};
dojo.string.pad=function(str,len,c,dir){
var out=String(str);
if(!c){
c="0";
}
if(!dir){
dir=1;
}
while(out.length<len){
if(dir>0){
out=c+out;
}else{
out+=c;
}
}
return out;
};
dojo.string.padLeft=function(str,len,c){
return dojo.string.pad(str,len,c,1);
};
dojo.string.padRight=function(str,len,c){
return dojo.string.pad(str,len,c,-1);
};
dojo.provide("dojo.string");
dojo.require("dojo.string.common");
dojo.provide("dojo.html");
dojo.require("dojo.lang.func");
dojo.require("dojo.dom");
dojo.require("dojo.style");
dojo.require("dojo.string");
dojo.lang.mixin(dojo.html,dojo.dom);
dojo.lang.mixin(dojo.html,dojo.style);
dojo.html.clearSelection=function(){
try{
if(window["getSelection"]){
if(dojo.render.html.safari){
window.getSelection().collapse();
}else{
window.getSelection().removeAllRanges();
}
}else{
if(document.selection){
if(document.selection.empty){
document.selection.empty();
}else{
if(document.selection.clear){
document.selection.clear();
}
}
}
}
return true;
}
catch(e){
dojo.debug(e);
return false;
}
};
dojo.html.disableSelection=function(_24c){
_24c=dojo.byId(_24c)||document.body;
var h=dojo.render.html;
if(h.mozilla){
_24c.style.MozUserSelect="none";
}else{
if(h.safari){
_24c.style.KhtmlUserSelect="none";
}else{
if(h.ie){
_24c.unselectable="on";
}else{
return false;
}
}
}
return true;
};
dojo.html.enableSelection=function(_24e){
_24e=dojo.byId(_24e)||document.body;
var h=dojo.render.html;
if(h.mozilla){
_24e.style.MozUserSelect="";
}else{
if(h.safari){
_24e.style.KhtmlUserSelect="";
}else{
if(h.ie){
_24e.unselectable="off";
}else{
return false;
}
}
}
return true;
};
dojo.html.selectElement=function(_250){
_250=dojo.byId(_250);
if(document.selection&&document.body.createTextRange){
var _251=document.body.createTextRange();
_251.moveToElementText(_250);
_251.select();
}else{
if(window["getSelection"]){
var _252=window.getSelection();
if(_252["selectAllChildren"]){
_252.selectAllChildren(_250);
}
}
}
};
dojo.html.selectInputText=function(_253){
_253=dojo.byId(_253);
if(document.selection&&document.body.createTextRange){
var _254=_253.createTextRange();
_254.moveStart("character",0);
_254.moveEnd("character",_253.value.length);
_254.select();
}else{
if(window["getSelection"]){
var _255=window.getSelection();
_253.setSelectionRange(0,_253.value.length);
}
}
_253.focus();
};
dojo.html.isSelectionCollapsed=function(){
if(document["selection"]){
return document.selection.createRange().text=="";
}else{
if(window["getSelection"]){
var _256=window.getSelection();
if(dojo.lang.isString(_256)){
return _256=="";
}else{
return _256.isCollapsed;
}
}
}
};
dojo.html.getEventTarget=function(evt){
if(!evt){
evt=window.event||{};
}
var t=(evt.srcElement?evt.srcElement:(evt.target?evt.target:null));
while((t)&&(t.nodeType!=1)){
t=t.parentNode;
}
return t;
};
dojo.html.getDocumentWidth=function(){
dojo.deprecated("dojo.html.getDocument*","replaced by dojo.html.getViewport*","0.4");
return dojo.html.getViewportWidth();
};
dojo.html.getDocumentHeight=function(){
dojo.deprecated("dojo.html.getDocument*","replaced by dojo.html.getViewport*","0.4");
return dojo.html.getViewportHeight();
};
dojo.html.getDocumentSize=function(){
dojo.deprecated("dojo.html.getDocument*","replaced of dojo.html.getViewport*","0.4");
return dojo.html.getViewportSize();
};
dojo.html.getViewportWidth=function(){
var w=0;
if(window.innerWidth){
w=window.innerWidth;
}
if(dojo.exists(document,"documentElement.clientWidth")){
var w2=document.documentElement.clientWidth;
if(!w||w2&&w2<w){
w=w2;
}
return w;
}
if(document.body){
return document.body.clientWidth;
}
return 0;
};
dojo.html.getViewportHeight=function(){
if(window.innerHeight){
return window.innerHeight;
}
if(dojo.exists(document,"documentElement.clientHeight")){
return document.documentElement.clientHeight;
}
if(document.body){
return document.body.clientHeight;
}
return 0;
};
dojo.html.getViewportSize=function(){
var ret=[dojo.html.getViewportWidth(),dojo.html.getViewportHeight()];
ret.w=ret[0];
ret.h=ret[1];
return ret;
};
dojo.html.getScrollTop=function(){
return window.pageYOffset||document.documentElement.scrollTop||document.body.scrollTop||0;
};
dojo.html.getScrollLeft=function(){
return window.pageXOffset||document.documentElement.scrollLeft||document.body.scrollLeft||0;
};
dojo.html.getScrollOffset=function(){
var off=[dojo.html.getScrollLeft(),dojo.html.getScrollTop()];
off.x=off[0];
off.y=off[1];
return off;
};
dojo.html.getParentOfType=function(node,type){
dojo.deprecated("dojo.html.getParentOfType","replaced by dojo.html.getParentByType*","0.4");
return dojo.html.getParentByType(node,type);
};
dojo.html.getParentByType=function(node,type){
var _261=dojo.byId(node);
type=type.toLowerCase();
while((_261)&&(_261.nodeName.toLowerCase()!=type)){
if(_261==(document["body"]||document["documentElement"])){
return null;
}
_261=_261.parentNode;
}
return _261;
};
dojo.html.getAttribute=function(node,attr){
node=dojo.byId(node);
if((!node)||(!node.getAttribute)){
return null;
}
var ta=typeof attr=="string"?attr:new String(attr);
var v=node.getAttribute(ta.toUpperCase());
if((v)&&(typeof v=="string")&&(v!="")){
return v;
}
if(v&&v.value){
return v.value;
}
if((node.getAttributeNode)&&(node.getAttributeNode(ta))){
return (node.getAttributeNode(ta)).value;
}else{
if(node.getAttribute(ta)){
return node.getAttribute(ta);
}else{
if(node.getAttribute(ta.toLowerCase())){
return node.getAttribute(ta.toLowerCase());
}
}
}
return null;
};
dojo.html.hasAttribute=function(node,attr){
node=dojo.byId(node);
return dojo.html.getAttribute(node,attr)?true:false;
};
dojo.html.getClass=function(node){
node=dojo.byId(node);
if(!node){
return "";
}
var cs="";
if(node.className){
cs=node.className;
}else{
if(dojo.html.hasAttribute(node,"class")){
cs=dojo.html.getAttribute(node,"class");
}
}
return dojo.string.trim(cs);
};
dojo.html.getClasses=function(node){
var c=dojo.html.getClass(node);
return (c=="")?[]:c.split(/\s+/g);
};
dojo.html.hasClass=function(node,_26d){
return dojo.lang.inArray(dojo.html.getClasses(node),_26d);
};
dojo.html.prependClass=function(node,_26f){
_26f+=" "+dojo.html.getClass(node);
return dojo.html.setClass(node,_26f);
};
dojo.html.addClass=function(node,_271){
if(dojo.html.hasClass(node,_271)){
return false;
}
_271=dojo.string.trim(dojo.html.getClass(node)+" "+_271);
return dojo.html.setClass(node,_271);
};
dojo.html.setClass=function(node,_273){
node=dojo.byId(node);
var cs=new String(_273);
try{
if(typeof node.className=="string"){
node.className=cs;
}else{
if(node.setAttribute){
node.setAttribute("class",_273);
node.className=cs;
}else{
return false;
}
}
}
catch(e){
dojo.debug("dojo.html.setClass() failed",e);
}
return true;
};
dojo.html.removeClass=function(node,_276,_277){
var _276=dojo.string.trim(new String(_276));
try{
var cs=dojo.html.getClasses(node);
var nca=[];
if(_277){
for(var i=0;i<cs.length;i++){
if(cs[i].indexOf(_276)==-1){
nca.push(cs[i]);
}
}
}else{
for(var i=0;i<cs.length;i++){
if(cs[i]!=_276){
nca.push(cs[i]);
}
}
}
dojo.html.setClass(node,nca.join(" "));
}
catch(e){
dojo.debug("dojo.html.removeClass() failed",e);
}
return true;
};
dojo.html.replaceClass=function(node,_27c,_27d){
dojo.html.removeClass(node,_27d);
dojo.html.addClass(node,_27c);
};
dojo.html.classMatchType={ContainsAll:0,ContainsAny:1,IsOnly:2};
dojo.html.getElementsByClass=function(_27e,_27f,_280,_281,_282){
_27f=dojo.byId(_27f)||document;
var _283=_27e.split(/\s+/g);
var _284=[];
if(_281!=1&&_281!=2){
_281=0;
}
var _285=new RegExp("(\\s|^)(("+_283.join(")|(")+"))(\\s|$)");
var _286=[];
if(!_282&&document.evaluate){
var _287="//"+(_280||"*")+"[contains(";
if(_281!=dojo.html.classMatchType.ContainsAny){
_287+="concat(' ',@class,' '), ' "+_283.join(" ') and contains(concat(' ',@class,' '), ' ")+" ')]";
}else{
_287+="concat(' ',@class,' '), ' "+_283.join(" ')) or contains(concat(' ',@class,' '), ' ")+" ')]";
}
var _288=document.evaluate(_287,_27f,null,XPathResult.ANY_TYPE,null);
var _289=_288.iterateNext();
while(_289){
try{
_286.push(_289);
_289=_288.iterateNext();
}
catch(e){
break;
}
}
return _286;
}else{
if(!_280){
_280="*";
}
_286=_27f.getElementsByTagName(_280);
var node,i=0;
outer:
while(node=_286[i++]){
var _28b=dojo.html.getClasses(node);
if(_28b.length==0){
continue outer;
}
var _28c=0;
for(var j=0;j<_28b.length;j++){
if(_285.test(_28b[j])){
if(_281==dojo.html.classMatchType.ContainsAny){
_284.push(node);
continue outer;
}else{
_28c++;
}
}else{
if(_281==dojo.html.classMatchType.IsOnly){
continue outer;
}
}
}
if(_28c==_283.length){
if((_281==dojo.html.classMatchType.IsOnly)&&(_28c==_28b.length)){
_284.push(node);
}else{
if(_281==dojo.html.classMatchType.ContainsAll){
_284.push(node);
}
}
}
}
return _284;
}
};
dojo.html.getElementsByClassName=dojo.html.getElementsByClass;
dojo.html.getCursorPosition=function(e){
e=e||window.event;
var _28f={x:0,y:0};
if(e.pageX||e.pageY){
_28f.x=e.pageX;
_28f.y=e.pageY;
}else{
var de=document.documentElement;
var db=document.body;
_28f.x=e.clientX+((de||db)["scrollLeft"])-((de||db)["clientLeft"]);
_28f.y=e.clientY+((de||db)["scrollTop"])-((de||db)["clientTop"]);
}
return _28f;
};
dojo.html.overElement=function(_292,e){
_292=dojo.byId(_292);
var _294=dojo.html.getCursorPosition(e);
with(dojo.html){
var top=getAbsoluteY(_292,true);
var _296=top+getInnerHeight(_292);
var left=getAbsoluteX(_292,true);
var _298=left+getInnerWidth(_292);
}
return (_294.x>=left&&_294.x<=_298&&_294.y>=top&&_294.y<=_296);
};
dojo.html.setActiveStyleSheet=function(_299){
var i=0,a,els=document.getElementsByTagName("link");
while(a=els[i++]){
if(a.getAttribute("rel").indexOf("style")!=-1&&a.getAttribute("title")){
a.disabled=true;
if(a.getAttribute("title")==_299){
a.disabled=false;
}
}
}
};
dojo.html.getActiveStyleSheet=function(){
var i=0,a,els=document.getElementsByTagName("link");
while(a=els[i++]){
if(a.getAttribute("rel").indexOf("style")!=-1&&a.getAttribute("title")&&!a.disabled){
return a.getAttribute("title");
}
}
return null;
};
dojo.html.getPreferredStyleSheet=function(){
var i=0,a,els=document.getElementsByTagName("link");
while(a=els[i++]){
if(a.getAttribute("rel").indexOf("style")!=-1&&a.getAttribute("rel").indexOf("alt")==-1&&a.getAttribute("title")){
return a.getAttribute("title");
}
}
return null;
};
dojo.html.body=function(){
return document.body||document.getElementsByTagName("body")[0];
};
dojo.html.isTag=function(node){
node=dojo.byId(node);
if(node&&node.tagName){
var arr=dojo.lang.map(dojo.lang.toArray(arguments,1),function(a){
return String(a).toLowerCase();
});
return arr[dojo.lang.find(node.tagName.toLowerCase(),arr)]||"";
}
return "";
};
dojo.html.copyStyle=function(_2a0,_2a1){
if(dojo.lang.isUndefined(_2a1.style.cssText)){
_2a0.setAttribute("style",_2a1.getAttribute("style"));
}else{
_2a0.style.cssText=_2a1.style.cssText;
}
dojo.html.addClass(_2a0,dojo.html.getClass(_2a1));
};
dojo.html._callExtrasDeprecated=function(_2a2,args){
var _2a4="dojo.html.extras";
dojo.deprecated("dojo.html."+_2a2,"moved to "+_2a4,"0.4");
dojo["require"](_2a4);
return dojo.html[_2a2].apply(dojo.html,args);
};
dojo.html.createNodesFromText=function(){
return dojo.html._callExtrasDeprecated("createNodesFromText",arguments);
};
dojo.html.gravity=function(){
return dojo.html._callExtrasDeprecated("gravity",arguments);
};
dojo.html.placeOnScreen=function(){
return dojo.html._callExtrasDeprecated("placeOnScreen",arguments);
};
dojo.html.placeOnScreenPoint=function(){
return dojo.html._callExtrasDeprecated("placeOnScreenPoint",arguments);
};
dojo.html.renderedTextContent=function(){
return dojo.html._callExtrasDeprecated("renderedTextContent",arguments);
};
dojo.html.BackgroundIframe=function(){
return dojo.html._callExtrasDeprecated("BackgroundIframe",arguments);
};
dojo.provide("dojo.lfx.Animation");
dojo.provide("dojo.lfx.Line");
dojo.require("dojo.lang.func");
dojo.lfx.Line=function(_2a5,end){
this.start=_2a5;
this.end=end;
if(dojo.lang.isArray(_2a5)){
var diff=[];
dojo.lang.forEach(this.start,function(s,i){
diff[i]=this.end[i]-s;
},this);
this.getValue=function(n){
var res=[];
dojo.lang.forEach(this.start,function(s,i){
res[i]=(diff[i]*n)+s;
},this);
return res;
};
}else{
var diff=end-_2a5;
this.getValue=function(n){
return (diff*n)+this.start;
};
}
};
dojo.lfx.easeIn=function(n){
return Math.pow(n,3);
};
dojo.lfx.easeOut=function(n){
return (1-Math.pow(1-n,3));
};
dojo.lfx.easeInOut=function(n){
return ((3*Math.pow(n,2))-(2*Math.pow(n,3)));
};
dojo.lfx.IAnimation=function(){
};
dojo.lang.extend(dojo.lfx.IAnimation,{curve:null,duration:1000,easing:null,repeatCount:0,rate:25,handler:null,beforeBegin:null,onBegin:null,onAnimate:null,onEnd:null,onPlay:null,onPause:null,onStop:null,play:null,pause:null,stop:null,fire:function(evt,args){
if(this[evt]){
this[evt].apply(this,(args||[]));
}
},_active:false,_paused:false});
dojo.lfx.Animation=function(_2b4,_2b5,_2b6,_2b7,_2b8,rate){
dojo.lfx.IAnimation.call(this);
if(dojo.lang.isNumber(_2b4)||(!_2b4&&_2b5.getValue)){
rate=_2b8;
_2b8=_2b7;
_2b7=_2b6;
_2b6=_2b5;
_2b5=_2b4;
_2b4=null;
}else{
if(_2b4.getValue||dojo.lang.isArray(_2b4)){
rate=_2b7;
_2b8=_2b6;
_2b7=_2b5;
_2b6=_2b4;
_2b5=null;
_2b4=null;
}
}
if(dojo.lang.isArray(_2b6)){
this.curve=new dojo.lfx.Line(_2b6[0],_2b6[1]);
}else{
this.curve=_2b6;
}
if(_2b5!=null&&_2b5>0){
this.duration=_2b5;
}
if(_2b8){
this.repeatCount=_2b8;
}
if(rate){
this.rate=rate;
}
if(_2b4){
this.handler=_2b4.handler;
this.beforeBegin=_2b4.beforeBegin;
this.onBegin=_2b4.onBegin;
this.onEnd=_2b4.onEnd;
this.onPlay=_2b4.onPlay;
this.onPause=_2b4.onPause;
this.onStop=_2b4.onStop;
this.onAnimate=_2b4.onAnimate;
}
if(_2b7&&dojo.lang.isFunction(_2b7)){
this.easing=_2b7;
}
};
dojo.inherits(dojo.lfx.Animation,dojo.lfx.IAnimation);
dojo.lang.extend(dojo.lfx.Animation,{_startTime:null,_endTime:null,_timer:null,_percent:0,_startRepeatCount:0,play:function(_2ba,_2bb){
if(_2bb){
clearTimeout(this._timer);
this._active=false;
this._paused=false;
this._percent=0;
}else{
if(this._active&&!this._paused){
return this;
}
}
this.fire("handler",["beforeBegin"]);
this.fire("beforeBegin");
if(_2ba>0){
setTimeout(dojo.lang.hitch(this,function(){
this.play(null,_2bb);
}),_2ba);
return this;
}
this._startTime=new Date().valueOf();
if(this._paused){
this._startTime-=(this.duration*this._percent/100);
}
this._endTime=this._startTime+this.duration;
this._active=true;
this._paused=false;
var step=this._percent/100;
var _2bd=this.curve.getValue(step);
if(this._percent==0){
if(!this._startRepeatCount){
this._startRepeatCount=this.repeatCount;
}
this.fire("handler",["begin",_2bd]);
this.fire("onBegin",[_2bd]);
}
this.fire("handler",["play",_2bd]);
this.fire("onPlay",[_2bd]);
this._cycle();
return this;
},pause:function(){
clearTimeout(this._timer);
if(!this._active){
return this;
}
this._paused=true;
var _2be=this.curve.getValue(this._percent/100);
this.fire("handler",["pause",_2be]);
this.fire("onPause",[_2be]);
return this;
},gotoPercent:function(pct,_2c0){
clearTimeout(this._timer);
this._active=true;
this._paused=true;
this._percent=pct;
if(_2c0){
this.play();
}
},stop:function(_2c1){
clearTimeout(this._timer);
var step=this._percent/100;
if(_2c1){
step=1;
}
var _2c3=this.curve.getValue(step);
this.fire("handler",["stop",_2c3]);
this.fire("onStop",[_2c3]);
this._active=false;
this._paused=false;
return this;
},status:function(){
if(this._active){
return this._paused?"paused":"playing";
}else{
return "stopped";
}
},_cycle:function(){
clearTimeout(this._timer);
if(this._active){
var curr=new Date().valueOf();
var step=(curr-this._startTime)/(this._endTime-this._startTime);
if(step>=1){
step=1;
this._percent=100;
}else{
this._percent=step*100;
}
if((this.easing)&&(dojo.lang.isFunction(this.easing))){
step=this.easing(step);
}
var _2c6=this.curve.getValue(step);
this.fire("handler",["animate",_2c6]);
this.fire("onAnimate",[_2c6]);
if(step<1){
this._timer=setTimeout(dojo.lang.hitch(this,"_cycle"),this.rate);
}else{
this._active=false;
this.fire("handler",["end"]);
this.fire("onEnd");
if(this.repeatCount>0){
this.repeatCount--;
this.play(null,true);
}else{
if(this.repeatCount==-1){
this.play(null,true);
}else{
if(this._startRepeatCount){
this.repeatCount=this._startRepeatCount;
this._startRepeatCount=0;
}
}
}
}
}
return this;
}});
dojo.lfx.Combine=function(){
dojo.lfx.IAnimation.call(this);
this._anims=[];
this._animsEnded=0;
var _2c7=arguments;
if(_2c7.length==1&&(dojo.lang.isArray(_2c7[0])||dojo.lang.isArrayLike(_2c7[0]))){
_2c7=_2c7[0];
}
var _2c8=this;
dojo.lang.forEach(_2c7,function(anim){
_2c8._anims.push(anim);
var _2ca=(anim["onEnd"])?dojo.lang.hitch(anim,"onEnd"):function(){
};
anim.onEnd=function(){
_2ca();
_2c8._onAnimsEnded();
};
});
};
dojo.inherits(dojo.lfx.Combine,dojo.lfx.IAnimation);
dojo.lang.extend(dojo.lfx.Combine,{_animsEnded:0,play:function(_2cb,_2cc){
if(!this._anims.length){
return this;
}
this.fire("beforeBegin");
if(_2cb>0){
setTimeout(dojo.lang.hitch(this,function(){
this.play(null,_2cc);
}),_2cb);
return this;
}
if(_2cc||this._anims[0].percent==0){
this.fire("onBegin");
}
this.fire("onPlay");
this._animsCall("play",null,_2cc);
return this;
},pause:function(){
this.fire("onPause");
this._animsCall("pause");
return this;
},stop:function(_2cd){
this.fire("onStop");
this._animsCall("stop",_2cd);
return this;
},_onAnimsEnded:function(){
this._animsEnded++;
if(this._animsEnded>=this._anims.length){
this.fire("onEnd");
}
return this;
},_animsCall:function(_2ce){
var args=[];
if(arguments.length>1){
for(var i=1;i<arguments.length;i++){
args.push(arguments[i]);
}
}
var _2d1=this;
dojo.lang.forEach(this._anims,function(anim){
anim[_2ce](args);
},_2d1);
return this;
}});
dojo.lfx.Chain=function(){
dojo.lfx.IAnimation.call(this);
this._anims=[];
this._currAnim=-1;
var _2d3=arguments;
if(_2d3.length==1&&(dojo.lang.isArray(_2d3[0])||dojo.lang.isArrayLike(_2d3[0]))){
_2d3=_2d3[0];
}
var _2d4=this;
dojo.lang.forEach(_2d3,function(anim,i,_2d7){
_2d4._anims.push(anim);
var _2d8=(anim["onEnd"])?dojo.lang.hitch(anim,"onEnd"):function(){
};
if(i<_2d7.length-1){
anim.onEnd=function(){
_2d8();
_2d4._playNext();
};
}else{
anim.onEnd=function(){
_2d8();
_2d4.fire("onEnd");
};
}
},_2d4);
};
dojo.inherits(dojo.lfx.Chain,dojo.lfx.IAnimation);
dojo.lang.extend(dojo.lfx.Chain,{_currAnim:-1,play:function(_2d9,_2da){
if(!this._anims.length){
return this;
}
if(_2da||!this._anims[this._currAnim]){
this._currAnim=0;
}
var _2db=this._anims[this._currAnim];
this.fire("beforeBegin");
if(_2d9>0){
setTimeout(dojo.lang.hitch(this,function(){
this.play(null,_2da);
}),_2d9);
return this;
}
if(_2db){
if(this._currAnim==0){
this.fire("handler",["begin",this._currAnim]);
this.fire("onBegin",[this._currAnim]);
}
this.fire("onPlay",[this._currAnim]);
_2db.play(null,_2da);
}
return this;
},pause:function(){
if(this._anims[this._currAnim]){
this._anims[this._currAnim].pause();
this.fire("onPause",[this._currAnim]);
}
return this;
},playPause:function(){
if(this._anims.length==0){
return this;
}
if(this._currAnim==-1){
this._currAnim=0;
}
var _2dc=this._anims[this._currAnim];
if(_2dc){
if(!_2dc._active||_2dc._paused){
this.play();
}else{
this.pause();
}
}
return this;
},stop:function(){
var _2dd=this._anims[this._currAnim];
if(_2dd){
_2dd.stop();
this.fire("onStop",[this._currAnim]);
}
return _2dd;
},_playNext:function(){
if(this._currAnim==-1||this._anims.length==0){
return this;
}
this._currAnim++;
if(this._anims[this._currAnim]){
this._anims[this._currAnim].play(null,true);
}
return this;
}});
dojo.lfx.combine=function(){
var _2de=arguments;
if(dojo.lang.isArray(arguments[0])){
_2de=arguments[0];
}
return new dojo.lfx.Combine(_2de);
};
dojo.lfx.chain=function(){
var _2df=arguments;
if(dojo.lang.isArray(arguments[0])){
_2df=arguments[0];
}
return new dojo.lfx.Chain(_2df);
};
dojo.provide("dojo.lfx.html");
dojo.require("dojo.lfx.Animation");
dojo.require("dojo.html");
dojo.lfx.html._byId=function(_2e0){
if(!_2e0){
return [];
}
if(dojo.lang.isArray(_2e0)){
if(!_2e0.alreadyChecked){
var n=[];
dojo.lang.forEach(_2e0,function(node){
n.push(dojo.byId(node));
});
n.alreadyChecked=true;
return n;
}else{
return _2e0;
}
}else{
var n=[];
n.push(dojo.byId(_2e0));
n.alreadyChecked=true;
return n;
}
};
dojo.lfx.html.propertyAnimation=function(_2e3,_2e4,_2e5,_2e6){
_2e3=dojo.lfx.html._byId(_2e3);
if(_2e3.length==1){
dojo.lang.forEach(_2e4,function(prop){
if(typeof prop["start"]=="undefined"){
if(prop.property!="opacity"){
prop.start=parseInt(dojo.style.getComputedStyle(_2e3[0],prop.property));
}else{
prop.start=dojo.style.getOpacity(_2e3[0]);
}
}
});
}
var _2e8=function(_2e9){
var _2ea=new Array(_2e9.length);
for(var i=0;i<_2e9.length;i++){
_2ea[i]=Math.round(_2e9[i]);
}
return _2ea;
};
var _2ec=function(n,_2ee){
n=dojo.byId(n);
if(!n||!n.style){
return;
}
for(var s in _2ee){
if(s=="opacity"){
dojo.style.setOpacity(n,_2ee[s]);
}else{
n.style[s]=_2ee[s];
}
}
};
var _2f0=function(_2f1){
this._properties=_2f1;
this.diffs=new Array(_2f1.length);
dojo.lang.forEach(_2f1,function(prop,i){
if(dojo.lang.isArray(prop.start)){
this.diffs[i]=null;
}else{
if(prop.start instanceof dojo.graphics.color.Color){
prop.startRgb=prop.start.toRgb();
prop.endRgb=prop.end.toRgb();
}else{
this.diffs[i]=prop.end-prop.start;
}
}
},this);
this.getValue=function(n){
var ret={};
dojo.lang.forEach(this._properties,function(prop,i){
var _2f8=null;
if(dojo.lang.isArray(prop.start)){
}else{
if(prop.start instanceof dojo.graphics.color.Color){
_2f8=(prop.units||"rgb")+"(";
for(var j=0;j<prop.startRgb.length;j++){
_2f8+=Math.round(((prop.endRgb[j]-prop.startRgb[j])*n)+prop.startRgb[j])+(j<prop.startRgb.length-1?",":"");
}
_2f8+=")";
}else{
_2f8=((this.diffs[i])*n)+prop.start+(prop.property!="opacity"?prop.units||"px":"");
}
}
ret[dojo.style.toCamelCase(prop.property)]=_2f8;
},this);
return ret;
};
};
var anim=new dojo.lfx.Animation({onAnimate:function(_2fb){
dojo.lang.forEach(_2e3,function(node){
_2ec(node,_2fb);
});
}},_2e5,new _2f0(_2e4),_2e6);
return anim;
};
dojo.lfx.html._makeFadeable=function(_2fd){
var _2fe=function(node){
if(dojo.render.html.ie){
if((node.style.zoom.length==0)&&(dojo.style.getStyle(node,"zoom")=="normal")){
node.style.zoom="1";
}
if((node.style.width.length==0)&&(dojo.style.getStyle(node,"width")=="auto")){
node.style.width="auto";
}
}
};
if(dojo.lang.isArrayLike(_2fd)){
dojo.lang.forEach(_2fd,_2fe);
}else{
_2fe(_2fd);
}
};
dojo.lfx.html.fadeIn=function(_300,_301,_302,_303){
_300=dojo.lfx.html._byId(_300);
dojo.lfx.html._makeFadeable(_300);
var anim=dojo.lfx.propertyAnimation(_300,[{property:"opacity",start:dojo.style.getOpacity(_300[0]),end:1}],_301,_302);
if(_303){
var _305=(anim["onEnd"])?dojo.lang.hitch(anim,"onEnd"):function(){
};
anim.onEnd=function(){
_305();
_303(_300,anim);
};
}
return anim;
};
dojo.lfx.html.fadeOut=function(_306,_307,_308,_309){
_306=dojo.lfx.html._byId(_306);
dojo.lfx.html._makeFadeable(_306);
var anim=dojo.lfx.propertyAnimation(_306,[{property:"opacity",start:dojo.style.getOpacity(_306[0]),end:0}],_307,_308);
if(_309){
var _30b=(anim["onEnd"])?dojo.lang.hitch(anim,"onEnd"):function(){
};
anim.onEnd=function(){
_30b();
_309(_306,anim);
};
}
return anim;
};
dojo.lfx.html.fadeShow=function(_30c,_30d,_30e,_30f){
var anim=dojo.lfx.html.fadeIn(_30c,_30d,_30e,_30f);
var _311=(anim["beforeBegin"])?dojo.lang.hitch(anim,"beforeBegin"):function(){
};
anim.beforeBegin=function(){
_311();
if(dojo.lang.isArrayLike(_30c)){
dojo.lang.forEach(_30c,dojo.style.show);
}else{
dojo.style.show(_30c);
}
};
return anim;
};
dojo.lfx.html.fadeHide=function(_312,_313,_314,_315){
var anim=dojo.lfx.html.fadeOut(_312,_313,_314,function(){
if(dojo.lang.isArrayLike(_312)){
dojo.lang.forEach(_312,dojo.style.hide);
}else{
dojo.style.hide(_312);
}
if(_315){
_315(_312,anim);
}
});
return anim;
};
dojo.lfx.html.wipeIn=function(_317,_318,_319,_31a){
_317=dojo.lfx.html._byId(_317);
var _31b=[];
dojo.lang.forEach(_317,function(node){
var _31d=dojo.style.getStyle(node,"overflow");
if(_31d=="visible"){
node.style.overflow="hidden";
}
node.style.height="0px";
dojo.style.show(node);
var anim=dojo.lfx.propertyAnimation(node,[{property:"height",start:0,end:node.scrollHeight}],_318,_319);
var _31f=(anim["onEnd"])?dojo.lang.hitch(anim,"onEnd"):function(){
};
anim.onEnd=function(){
_31f();
node.style.overflow=_31d;
node.style.height="auto";
if(_31a){
_31a(node,anim);
}
};
_31b.push(anim);
});
if(_317.length>1){
return dojo.lfx.combine(_31b);
}else{
return _31b[0];
}
};
dojo.lfx.html.wipeOut=function(_320,_321,_322,_323){
_320=dojo.lfx.html._byId(_320);
var _324=[];
dojo.lang.forEach(_320,function(node){
var _326=dojo.style.getStyle(node,"overflow");
if(_326=="visible"){
node.style.overflow="hidden";
}
dojo.style.show(node);
var anim=dojo.lfx.propertyAnimation(node,[{property:"height",start:dojo.style.getContentBoxHeight(node),end:0}],_321,_322);
var _328=(anim["onEnd"])?dojo.lang.hitch(anim,"onEnd"):function(){
};
anim.onEnd=function(){
_328();
dojo.style.hide(node);
node.style.overflow=_326;
if(_323){
_323(node,anim);
}
};
_324.push(anim);
});
if(_320.length>1){
return dojo.lfx.combine(_324);
}else{
return _324[0];
}
};
dojo.lfx.html.slideTo=function(_329,_32a,_32b,_32c,_32d){
_329=dojo.lfx.html._byId(_329);
var _32e=[];
dojo.lang.forEach(_329,function(node){
var top=null;
var left=null;
var init=(function(){
var _333=node;
return function(){
top=_333.offsetTop;
left=_333.offsetLeft;
if(!dojo.style.isPositionAbsolute(_333)){
var ret=dojo.style.abs(_333,true);
dojo.style.setStyleAttributes(_333,"position:absolute;top:"+ret.y+"px;left:"+ret.x+"px;");
top=ret.y;
left=ret.x;
}
};
})();
init();
var anim=dojo.lfx.propertyAnimation(node,[{property:"top",start:top,end:_32a[0]},{property:"left",start:left,end:_32a[1]}],_32b,_32c);
var _336=(anim["beforeBegin"])?dojo.lang.hitch(anim,"beforeBegin"):function(){
};
anim.beforeBegin=function(){
_336();
init();
};
if(_32d){
var _337=(anim["onEnd"])?dojo.lang.hitch(anim,"onEnd"):function(){
};
anim.onEnd=function(){
_337();
_32d(_329,anim);
};
}
_32e.push(anim);
});
if(_329.length>1){
return dojo.lfx.combine(_32e);
}else{
return _32e[0];
}
};
dojo.lfx.html.slideBy=function(_338,_339,_33a,_33b,_33c){
_338=dojo.lfx.html._byId(_338);
var _33d=[];
dojo.lang.forEach(_338,function(node){
var top=null;
var left=null;
var init=(function(){
var _342=node;
return function(){
top=node.offsetTop;
left=node.offsetLeft;
if(!dojo.style.isPositionAbsolute(_342)){
var ret=dojo.style.abs(_342);
dojo.style.setStyleAttributes(_342,"position:absolute;top:"+ret.y+"px;left:"+ret.x+"px;");
top=ret.y;
left=ret.x;
}
};
})();
init();
var anim=dojo.lfx.propertyAnimation(node,[{property:"top",start:top,end:top+_339[0]},{property:"left",start:left,end:left+_339[1]}],_33a,_33b);
var _345=(anim["beforeBegin"])?dojo.lang.hitch(anim,"beforeBegin"):function(){
};
anim.beforeBegin=function(){
_345();
init();
};
if(_33c){
var _346=(anim["onEnd"])?dojo.lang.hitch(anim,"onEnd"):function(){
};
anim.onEnd=function(){
_346();
_33c(_338,anim);
};
}
_33d.push(anim);
});
if(_338.length>1){
return dojo.lfx.combine(_33d);
}else{
return _33d[0];
}
};
dojo.lfx.html.explode=function(_347,_348,_349,_34a,_34b){
_347=dojo.byId(_347);
_348=dojo.byId(_348);
var _34c=dojo.style.toCoordinateArray(_347,true);
var _34d=document.createElement("div");
dojo.html.copyStyle(_34d,_348);
with(_34d.style){
position="absolute";
display="none";
}
document.body.appendChild(_34d);
with(_348.style){
visibility="hidden";
display="block";
}
var _34e=dojo.style.toCoordinateArray(_348,true);
with(_348.style){
display="none";
visibility="visible";
}
var anim=new dojo.lfx.propertyAnimation(_34d,[{property:"height",start:_34c[3],end:_34e[3]},{property:"width",start:_34c[2],end:_34e[2]},{property:"top",start:_34c[1],end:_34e[1]},{property:"left",start:_34c[0],end:_34e[0]},{property:"opacity",start:0.3,end:1}],_349,_34a);
anim.beforeBegin=function(){
dojo.style.setDisplay(_34d,"block");
};
anim.onEnd=function(){
dojo.style.setDisplay(_348,"block");
_34d.parentNode.removeChild(_34d);
};
if(_34b){
var _350=(anim["onEnd"])?dojo.lang.hitch(anim,"onEnd"):function(){
};
anim.onEnd=function(){
_350();
_34b(_348,anim);
};
}
return anim;
};
dojo.lfx.html.implode=function(_351,end,_353,_354,_355){
_351=dojo.byId(_351);
end=dojo.byId(end);
var _356=dojo.style.toCoordinateArray(_351,true);
var _357=dojo.style.toCoordinateArray(end,true);
var _358=document.createElement("div");
dojo.html.copyStyle(_358,_351);
dojo.style.setOpacity(_358,0.3);
with(_358.style){
position="absolute";
display="none";
}
document.body.appendChild(_358);
var anim=new dojo.lfx.propertyAnimation(_358,[{property:"height",start:_356[3],end:_357[3]},{property:"width",start:_356[2],end:_357[2]},{property:"top",start:_356[1],end:_357[1]},{property:"left",start:_356[0],end:_357[0]},{property:"opacity",start:1,end:0.3}],_353,_354);
anim.beforeBegin=function(){
dojo.style.hide(_351);
dojo.style.show(_358);
};
anim.onEnd=function(){
_358.parentNode.removeChild(_358);
};
if(_355){
var _35a=(anim["onEnd"])?dojo.lang.hitch(anim,"onEnd"):function(){
};
anim.onEnd=function(){
_35a();
_355(_351,anim);
};
}
return anim;
};
dojo.lfx.html.highlight=function(_35b,_35c,_35d,_35e,_35f){
_35b=dojo.lfx.html._byId(_35b);
var _360=[];
dojo.lang.forEach(_35b,function(node){
var _362=dojo.style.getBackgroundColor(node);
var bg=dojo.style.getStyle(node,"background-color").toLowerCase();
var _364=dojo.style.getStyle(node,"background-image");
var _365=(bg=="transparent"||bg=="rgba(0, 0, 0, 0)");
while(_362.length>3){
_362.pop();
}
var rgb=new dojo.graphics.color.Color(_35c);
var _367=new dojo.graphics.color.Color(_362);
var anim=dojo.lfx.propertyAnimation(node,[{property:"background-color",start:rgb,end:_367}],_35d,_35e);
var _369=(anim["beforeBegin"])?dojo.lang.hitch(anim,"beforeBegin"):function(){
};
anim.beforeBegin=function(){
_369();
if(_364){
node.style.backgroundImage="none";
}
node.style.backgroundColor="rgb("+rgb.toRgb().join(",")+")";
};
var _36a=(anim["onEnd"])?dojo.lang.hitch(anim,"onEnd"):function(){
};
anim.onEnd=function(){
_36a();
if(_364){
node.style.backgroundImage=_364;
}
if(_365){
node.style.backgroundColor="transparent";
}
if(_35f){
_35f(node,anim);
}
};
_360.push(anim);
});
if(_35b.length>1){
return dojo.lfx.combine(_360);
}else{
return _360[0];
}
};
dojo.lfx.html.unhighlight=function(_36b,_36c,_36d,_36e,_36f){
_36b=dojo.lfx.html._byId(_36b);
var _370=[];
dojo.lang.forEach(_36b,function(node){
var _372=new dojo.graphics.color.Color(dojo.style.getBackgroundColor(node));
var rgb=new dojo.graphics.color.Color(_36c);
var _374=dojo.style.getStyle(node,"background-image");
var anim=dojo.lfx.propertyAnimation(node,[{property:"background-color",start:_372,end:rgb}],_36d,_36e);
var _376=(anim["beforeBegin"])?dojo.lang.hitch(anim,"beforeBegin"):function(){
};
anim.beforeBegin=function(){
_376();
if(_374){
node.style.backgroundImage="none";
}
node.style.backgroundColor="rgb("+_372.toRgb().join(",")+")";
};
var _377=(anim["onEnd"])?dojo.lang.hitch(anim,"onEnd"):function(){
};
anim.onEnd=function(){
_377();
if(_36f){
_36f(node,anim);
}
};
_370.push(anim);
});
if(_36b.length>1){
return dojo.lfx.combine(_370);
}else{
return _370[0];
}
};
dojo.lang.mixin(dojo.lfx,dojo.lfx.html);
dojo.kwCompoundRequire({browser:["dojo.lfx.html"],dashboard:["dojo.lfx.html"]});
dojo.provide("dojo.lfx.*");
dojo.provide("dojo.lang.extras");
dojo.require("dojo.lang.common");
dojo.lang.setTimeout=function(func,_379){
var _37a=window,argsStart=2;
if(!dojo.lang.isFunction(func)){
_37a=func;
func=_379;
_379=arguments[2];
argsStart++;
}
if(dojo.lang.isString(func)){
func=_37a[func];
}
var args=[];
for(var i=argsStart;i<arguments.length;i++){
args.push(arguments[i]);
}
return setTimeout(function(){
func.apply(_37a,args);
},_379);
};
dojo.lang.getNameInObj=function(ns,item){
if(!ns){
ns=dj_global;
}
for(var x in ns){
if(ns[x]===item){
return new String(x);
}
}
return null;
};
dojo.lang.shallowCopy=function(obj){
var ret={},key;
for(key in obj){
if(dojo.lang.isUndefined(ret[key])){
ret[key]=obj[key];
}
}
return ret;
};
dojo.lang.firstValued=function(){
for(var i=0;i<arguments.length;i++){
if(typeof arguments[i]!="undefined"){
return arguments[i];
}
}
return undefined;
};
dojo.lang.getObjPathValue=function(_383,_384,_385){
with(dojo.parseObjPath(_383,_384,_385)){
return dojo.evalProp(prop,obj,_385);
}
};
dojo.lang.setObjPathValue=function(_386,_387,_388,_389){
if(arguments.length<4){
_389=true;
}
with(dojo.parseObjPath(_386,_388,_389)){
if(obj&&(_389||(prop in obj))){
obj[prop]=_387;
}
}
};
dojo.provide("dojo.event");
dojo.require("dojo.lang.array");
dojo.require("dojo.lang.extras");
dojo.require("dojo.lang.func");
dojo.event=new function(){
this.canTimeout=dojo.lang.isFunction(dj_global["setTimeout"])||dojo.lang.isAlien(dj_global["setTimeout"]);
function interpolateArgs(args,_38b){
var dl=dojo.lang;
var ao={srcObj:dj_global,srcFunc:null,adviceObj:dj_global,adviceFunc:null,aroundObj:null,aroundFunc:null,adviceType:(args.length>2)?args[0]:"after",precedence:"last",once:false,delay:null,rate:0,adviceMsg:false};
switch(args.length){
case 0:
return;
case 1:
return;
case 2:
ao.srcFunc=args[0];
ao.adviceFunc=args[1];
break;
case 3:
if((dl.isObject(args[0]))&&(dl.isString(args[1]))&&(dl.isString(args[2]))){
ao.adviceType="after";
ao.srcObj=args[0];
ao.srcFunc=args[1];
ao.adviceFunc=args[2];
}else{
if((dl.isString(args[1]))&&(dl.isString(args[2]))){
ao.srcFunc=args[1];
ao.adviceFunc=args[2];
}else{
if((dl.isObject(args[0]))&&(dl.isString(args[1]))&&(dl.isFunction(args[2]))){
ao.adviceType="after";
ao.srcObj=args[0];
ao.srcFunc=args[1];
var _38e=dl.nameAnonFunc(args[2],ao.adviceObj,_38b);
ao.adviceFunc=_38e;
}else{
if((dl.isFunction(args[0]))&&(dl.isObject(args[1]))&&(dl.isString(args[2]))){
ao.adviceType="after";
ao.srcObj=dj_global;
var _38e=dl.nameAnonFunc(args[0],ao.srcObj,_38b);
ao.srcFunc=_38e;
ao.adviceObj=args[1];
ao.adviceFunc=args[2];
}
}
}
}
break;
case 4:
if((dl.isObject(args[0]))&&(dl.isObject(args[2]))){
ao.adviceType="after";
ao.srcObj=args[0];
ao.srcFunc=args[1];
ao.adviceObj=args[2];
ao.adviceFunc=args[3];
}else{
if((dl.isString(args[0]))&&(dl.isString(args[1]))&&(dl.isObject(args[2]))){
ao.adviceType=args[0];
ao.srcObj=dj_global;
ao.srcFunc=args[1];
ao.adviceObj=args[2];
ao.adviceFunc=args[3];
}else{
if((dl.isString(args[0]))&&(dl.isFunction(args[1]))&&(dl.isObject(args[2]))){
ao.adviceType=args[0];
ao.srcObj=dj_global;
var _38e=dl.nameAnonFunc(args[1],dj_global,_38b);
ao.srcFunc=_38e;
ao.adviceObj=args[2];
ao.adviceFunc=args[3];
}else{
if((dl.isString(args[0]))&&(dl.isObject(args[1]))&&(dl.isString(args[2]))&&(dl.isFunction(args[3]))){
ao.srcObj=args[1];
ao.srcFunc=args[2];
var _38e=dl.nameAnonFunc(args[3],dj_global,_38b);
ao.adviceObj=dj_global;
ao.adviceFunc=_38e;
}else{
if(dl.isObject(args[1])){
ao.srcObj=args[1];
ao.srcFunc=args[2];
ao.adviceObj=dj_global;
ao.adviceFunc=args[3];
}else{
if(dl.isObject(args[2])){
ao.srcObj=dj_global;
ao.srcFunc=args[1];
ao.adviceObj=args[2];
ao.adviceFunc=args[3];
}else{
ao.srcObj=ao.adviceObj=ao.aroundObj=dj_global;
ao.srcFunc=args[1];
ao.adviceFunc=args[2];
ao.aroundFunc=args[3];
}
}
}
}
}
}
break;
case 6:
ao.srcObj=args[1];
ao.srcFunc=args[2];
ao.adviceObj=args[3];
ao.adviceFunc=args[4];
ao.aroundFunc=args[5];
ao.aroundObj=dj_global;
break;
default:
ao.srcObj=args[1];
ao.srcFunc=args[2];
ao.adviceObj=args[3];
ao.adviceFunc=args[4];
ao.aroundObj=args[5];
ao.aroundFunc=args[6];
ao.once=args[7];
ao.delay=args[8];
ao.rate=args[9];
ao.adviceMsg=args[10];
break;
}
if(dl.isFunction(ao.aroundFunc)){
var _38e=dl.nameAnonFunc(ao.aroundFunc,ao.aroundObj,_38b);
ao.aroundFunc=_38e;
}
if(dl.isFunction(ao.srcFunc)){
ao.srcFunc=dl.getNameInObj(ao.srcObj,ao.srcFunc);
}
if(dl.isFunction(ao.adviceFunc)){
ao.adviceFunc=dl.getNameInObj(ao.adviceObj,ao.adviceFunc);
}
if((ao.aroundObj)&&(dl.isFunction(ao.aroundFunc))){
ao.aroundFunc=dl.getNameInObj(ao.aroundObj,ao.aroundFunc);
}
if(!ao.srcObj){
dojo.raise("bad srcObj for srcFunc: "+ao.srcFunc);
}
if(!ao.adviceObj){
dojo.raise("bad adviceObj for adviceFunc: "+ao.adviceFunc);
}
return ao;
}
this.connect=function(){
if(arguments.length==1){
var ao=arguments[0];
}else{
var ao=interpolateArgs(arguments,true);
}
if(dojo.lang.isArray(ao.srcObj)&&ao.srcObj!=""){
var _390={};
for(var x in ao){
_390[x]=ao[x];
}
var mjps=[];
dojo.lang.forEach(ao.srcObj,function(src){
if((dojo.render.html.capable)&&(dojo.lang.isString(src))){
src=dojo.byId(src);
}
_390.srcObj=src;
mjps.push(dojo.event.connect.call(dojo.event,_390));
});
return mjps;
}
var mjp=dojo.event.MethodJoinPoint.getForMethod(ao.srcObj,ao.srcFunc);
if(ao.adviceFunc){
var mjp2=dojo.event.MethodJoinPoint.getForMethod(ao.adviceObj,ao.adviceFunc);
}
mjp.kwAddAdvice(ao);
return mjp;
};
this.log=function(a1,a2){
var _398;
if((arguments.length==1)&&(typeof a1=="object")){
_398=a1;
}else{
_398={srcObj:a1,srcFunc:a2};
}
_398.adviceFunc=function(){
var _399=[];
for(var x=0;x<arguments.length;x++){
_399.push(arguments[x]);
}
dojo.debug("("+_398.srcObj+")."+_398.srcFunc,":",_399.join(", "));
};
this.kwConnect(_398);
};
this.connectBefore=function(){
var args=["before"];
for(var i=0;i<arguments.length;i++){
args.push(arguments[i]);
}
return this.connect.apply(this,args);
};
this.connectAround=function(){
var args=["around"];
for(var i=0;i<arguments.length;i++){
args.push(arguments[i]);
}
return this.connect.apply(this,args);
};
this.connectOnce=function(){
var ao=interpolateArgs(arguments,true);
ao.once=true;
return this.connect(ao);
};
this._kwConnectImpl=function(_3a0,_3a1){
var fn=(_3a1)?"disconnect":"connect";
if(typeof _3a0["srcFunc"]=="function"){
_3a0.srcObj=_3a0["srcObj"]||dj_global;
var _3a3=dojo.lang.nameAnonFunc(_3a0.srcFunc,_3a0.srcObj,true);
_3a0.srcFunc=_3a3;
}
if(typeof _3a0["adviceFunc"]=="function"){
_3a0.adviceObj=_3a0["adviceObj"]||dj_global;
var _3a3=dojo.lang.nameAnonFunc(_3a0.adviceFunc,_3a0.adviceObj,true);
_3a0.adviceFunc=_3a3;
}
return dojo.event[fn]((_3a0["type"]||_3a0["adviceType"]||"after"),_3a0["srcObj"]||dj_global,_3a0["srcFunc"],_3a0["adviceObj"]||_3a0["targetObj"]||dj_global,_3a0["adviceFunc"]||_3a0["targetFunc"],_3a0["aroundObj"],_3a0["aroundFunc"],_3a0["once"],_3a0["delay"],_3a0["rate"],_3a0["adviceMsg"]||false);
};
this.kwConnect=function(_3a4){
return this._kwConnectImpl(_3a4,false);
};
this.disconnect=function(){
var ao=interpolateArgs(arguments,true);
if(!ao.adviceFunc){
return;
}
var mjp=dojo.event.MethodJoinPoint.getForMethod(ao.srcObj,ao.srcFunc);
return mjp.removeAdvice(ao.adviceObj,ao.adviceFunc,ao.adviceType,ao.once);
};
this.kwDisconnect=function(_3a7){
return this._kwConnectImpl(_3a7,true);
};
};
dojo.event.MethodInvocation=function(_3a8,obj,args){
this.jp_=_3a8;
this.object=obj;
this.args=[];
for(var x=0;x<args.length;x++){
this.args[x]=args[x];
}
this.around_index=-1;
};
dojo.event.MethodInvocation.prototype.proceed=function(){
this.around_index++;
if(this.around_index>=this.jp_.around.length){
return this.jp_.object[this.jp_.methodname].apply(this.jp_.object,this.args);
}else{
var ti=this.jp_.around[this.around_index];
var mobj=ti[0]||dj_global;
var meth=ti[1];
return mobj[meth].call(mobj,this);
}
};
dojo.event.MethodJoinPoint=function(obj,_3b0){
this.object=obj||dj_global;
this.methodname=_3b0;
this.methodfunc=this.object[_3b0];
this.before=[];
this.after=[];
this.around=[];
};
dojo.event.MethodJoinPoint.getForMethod=function(obj,_3b2){
if(!obj){
obj=dj_global;
}
if(!obj[_3b2]){
obj[_3b2]=function(){
};
if(!obj[_3b2]){
dojo.raise("Cannot set do-nothing method on that object "+_3b2);
}
}else{
if((!dojo.lang.isFunction(obj[_3b2]))&&(!dojo.lang.isAlien(obj[_3b2]))){
return null;
}
}
var _3b3=_3b2+"$joinpoint";
var _3b4=_3b2+"$joinpoint$method";
var _3b5=obj[_3b3];
if(!_3b5){
var _3b6=false;
if(dojo.event["browser"]){
if((obj["attachEvent"])||(obj["nodeType"])||(obj["addEventListener"])){
_3b6=true;
dojo.event.browser.addClobberNodeAttrs(obj,[_3b3,_3b4,_3b2]);
}
}
var _3b7=obj[_3b2].length;
obj[_3b4]=obj[_3b2];
_3b5=obj[_3b3]=new dojo.event.MethodJoinPoint(obj,_3b4);
obj[_3b2]=function(){
var args=[];
if((_3b6)&&(!arguments.length)){
var evt=null;
try{
if(obj.ownerDocument){
evt=obj.ownerDocument.parentWindow.event;
}else{
if(obj.documentElement){
evt=obj.documentElement.ownerDocument.parentWindow.event;
}else{
evt=window.event;
}
}
}
catch(e){
evt=window.event;
}
if(evt){
args.push(dojo.event.browser.fixEvent(evt,this));
}
}else{
for(var x=0;x<arguments.length;x++){
if((x==0)&&(_3b6)&&(dojo.event.browser.isEvent(arguments[x]))){
args.push(dojo.event.browser.fixEvent(arguments[x],this));
}else{
args.push(arguments[x]);
}
}
}
return _3b5.run.apply(_3b5,args);
};
obj[_3b2].__preJoinArity=_3b7;
}
return _3b5;
};
dojo.lang.extend(dojo.event.MethodJoinPoint,{unintercept:function(){
this.object[this.methodname]=this.methodfunc;
this.before=[];
this.after=[];
this.around=[];
},disconnect:dojo.lang.forward("unintercept"),run:function(){
var obj=this.object||dj_global;
var args=arguments;
var _3bd=[];
for(var x=0;x<args.length;x++){
_3bd[x]=args[x];
}
var _3bf=function(marr){
if(!marr){
dojo.debug("Null argument to unrollAdvice()");
return;
}
var _3c1=marr[0]||dj_global;
var _3c2=marr[1];
if(!_3c1[_3c2]){
dojo.raise("function \""+_3c2+"\" does not exist on \""+_3c1+"\"");
}
var _3c3=marr[2]||dj_global;
var _3c4=marr[3];
var msg=marr[6];
var _3c6;
var to={args:[],jp_:this,object:obj,proceed:function(){
return _3c1[_3c2].apply(_3c1,to.args);
}};
to.args=_3bd;
var _3c8=parseInt(marr[4]);
var _3c9=((!isNaN(_3c8))&&(marr[4]!==null)&&(typeof marr[4]!="undefined"));
if(marr[5]){
var rate=parseInt(marr[5]);
var cur=new Date();
var _3cc=false;
if((marr["last"])&&((cur-marr.last)<=rate)){
if(dojo.event.canTimeout){
if(marr["delayTimer"]){
clearTimeout(marr.delayTimer);
}
var tod=parseInt(rate*2);
var mcpy=dojo.lang.shallowCopy(marr);
marr.delayTimer=setTimeout(function(){
mcpy[5]=0;
_3bf(mcpy);
},tod);
}
return;
}else{
marr.last=cur;
}
}
if(_3c4){
_3c3[_3c4].call(_3c3,to);
}else{
if((_3c9)&&((dojo.render.html)||(dojo.render.svg))){
dj_global["setTimeout"](function(){
if(msg){
_3c1[_3c2].call(_3c1,to);
}else{
_3c1[_3c2].apply(_3c1,args);
}
},_3c8);
}else{
if(msg){
_3c1[_3c2].call(_3c1,to);
}else{
_3c1[_3c2].apply(_3c1,args);
}
}
}
};
if(this.before.length>0){
dojo.lang.forEach(this.before,_3bf);
}
var _3cf;
if(this.around.length>0){
var mi=new dojo.event.MethodInvocation(this,obj,args);
_3cf=mi.proceed();
}else{
if(this.methodfunc){
_3cf=this.object[this.methodname].apply(this.object,args);
}
}
if(this.after.length>0){
dojo.lang.forEach(this.after,_3bf);
}
return (this.methodfunc)?_3cf:null;
},getArr:function(kind){
var arr=this.after;
if((typeof kind=="string")&&(kind.indexOf("before")!=-1)){
arr=this.before;
}else{
if(kind=="around"){
arr=this.around;
}
}
return arr;
},kwAddAdvice:function(args){
this.addAdvice(args["adviceObj"],args["adviceFunc"],args["aroundObj"],args["aroundFunc"],args["adviceType"],args["precedence"],args["once"],args["delay"],args["rate"],args["adviceMsg"]);
},addAdvice:function(_3d4,_3d5,_3d6,_3d7,_3d8,_3d9,once,_3db,rate,_3dd){
var arr=this.getArr(_3d8);
if(!arr){
dojo.raise("bad this: "+this);
}
var ao=[_3d4,_3d5,_3d6,_3d7,_3db,rate,_3dd];
if(once){
if(this.hasAdvice(_3d4,_3d5,_3d8,arr)>=0){
return;
}
}
if(_3d9=="first"){
arr.unshift(ao);
}else{
arr.push(ao);
}
},hasAdvice:function(_3e0,_3e1,_3e2,arr){
if(!arr){
arr=this.getArr(_3e2);
}
var ind=-1;
for(var x=0;x<arr.length;x++){
var aao=(typeof _3e1=="object")?(new String(_3e1)).toString():_3e1;
var a1o=(typeof arr[x][1]=="object")?(new String(arr[x][1])).toString():arr[x][1];
if((arr[x][0]==_3e0)&&(a1o==aao)){
ind=x;
}
}
return ind;
},removeAdvice:function(_3e8,_3e9,_3ea,once){
var arr=this.getArr(_3ea);
var ind=this.hasAdvice(_3e8,_3e9,_3ea,arr);
if(ind==-1){
return false;
}
while(ind!=-1){
arr.splice(ind,1);
if(once){
break;
}
ind=this.hasAdvice(_3e8,_3e9,_3ea,arr);
}
return true;
}});
dojo.require("dojo.event");
dojo.provide("dojo.event.topic");
dojo.event.topic=new function(){
this.topics={};
this.getTopic=function(_3ee){
if(!this.topics[_3ee]){
this.topics[_3ee]=new this.TopicImpl(_3ee);
}
return this.topics[_3ee];
};
this.registerPublisher=function(_3ef,obj,_3f1){
var _3ef=this.getTopic(_3ef);
_3ef.registerPublisher(obj,_3f1);
};
this.subscribe=function(_3f2,obj,_3f4){
var _3f2=this.getTopic(_3f2);
_3f2.subscribe(obj,_3f4);
};
this.unsubscribe=function(_3f5,obj,_3f7){
var _3f5=this.getTopic(_3f5);
_3f5.unsubscribe(obj,_3f7);
};
this.destroy=function(_3f8){
this.getTopic(_3f8).destroy();
delete this.topics[_3f8];
};
this.publishApply=function(_3f9,args){
var _3f9=this.getTopic(_3f9);
_3f9.sendMessage.apply(_3f9,args);
};
this.publish=function(_3fb,_3fc){
var _3fb=this.getTopic(_3fb);
var args=[];
for(var x=1;x<arguments.length;x++){
args.push(arguments[x]);
}
_3fb.sendMessage.apply(_3fb,args);
};
};
dojo.event.topic.TopicImpl=function(_3ff){
this.topicName=_3ff;
this.subscribe=function(_400,_401){
var tf=_401||_400;
var to=(!_401)?dj_global:_400;
dojo.event.kwConnect({srcObj:this,srcFunc:"sendMessage",adviceObj:to,adviceFunc:tf});
};
this.unsubscribe=function(_404,_405){
var tf=(!_405)?_404:_405;
var to=(!_405)?null:_404;
dojo.event.kwDisconnect({srcObj:this,srcFunc:"sendMessage",adviceObj:to,adviceFunc:tf});
};
this.destroy=function(){
dojo.event.MethodJoinPoint.getForMethod(this,"sendMessage").disconnect();
};
this.registerPublisher=function(_408,_409){
dojo.event.connect(_408,_409,this,"sendMessage");
};
this.sendMessage=function(_40a){
};
};
dojo.provide("dojo.event.browser");
dojo.require("dojo.event");
dojo._ie_clobber=new function(){
this.clobberNodes=[];
function nukeProp(node,prop){
try{
node[prop]=null;
}
catch(e){
}
try{
delete node[prop];
}
catch(e){
}
try{
node.removeAttribute(prop);
}
catch(e){
}
}
this.clobber=function(_40d){
var na;
var tna;
if(_40d){
tna=_40d.all||_40d.getElementsByTagName("*");
na=[_40d];
for(var x=0;x<tna.length;x++){
if(tna[x]["__doClobber__"]){
na.push(tna[x]);
}
}
}else{
try{
window.onload=null;
}
catch(e){
}
na=(this.clobberNodes.length)?this.clobberNodes:document.all;
}
tna=null;
var _411={};
for(var i=na.length-1;i>=0;i=i-1){
var el=na[i];
if(el["__clobberAttrs__"]){
for(var j=0;j<el.__clobberAttrs__.length;j++){
nukeProp(el,el.__clobberAttrs__[j]);
}
nukeProp(el,"__clobberAttrs__");
nukeProp(el,"__doClobber__");
}
}
na=null;
};
};
if(dojo.render.html.ie){
dojo.addOnUnload(function(){
dojo._ie_clobber.clobber();
try{
if((dojo["widget"])&&(dojo.widget["manager"])){
dojo.widget.manager.destroyAll();
}
}
catch(e){
}
try{
window.onload=null;
}
catch(e){
}
try{
window.onunload=null;
}
catch(e){
}
dojo._ie_clobber.clobberNodes=[];
});
}
dojo.event.browser=new function(){
var _415=0;
this.clean=function(node){
if(dojo.render.html.ie){
dojo._ie_clobber.clobber(node);
}
};
this.addClobberNode=function(node){
if(!dojo.render.html.ie){
return;
}
if(!node["__doClobber__"]){
node.__doClobber__=true;
dojo._ie_clobber.clobberNodes.push(node);
node.__clobberAttrs__=[];
}
};
this.addClobberNodeAttrs=function(node,_419){
if(!dojo.render.html.ie){
return;
}
this.addClobberNode(node);
for(var x=0;x<_419.length;x++){
node.__clobberAttrs__.push(_419[x]);
}
};
this.removeListener=function(node,_41c,fp,_41e){
if(!_41e){
var _41e=false;
}
_41c=_41c.toLowerCase();
if(_41c.substr(0,2)=="on"){
_41c=_41c.substr(2);
}
if(node.removeEventListener){
node.removeEventListener(_41c,fp,_41e);
}
};
this.addListener=function(node,_420,fp,_422,_423){
if(!node){
return;
}
if(!_422){
var _422=false;
}
_420=_420.toLowerCase();
if(_420.substr(0,2)!="on"){
_420="on"+_420;
}
if(!_423){
var _424=function(evt){
if(!evt){
evt=window.event;
}
var ret=fp(dojo.event.browser.fixEvent(evt,this));
if(_422){
dojo.event.browser.stopEvent(evt);
}
return ret;
};
}else{
_424=fp;
}
if(node.addEventListener){
node.addEventListener(_420.substr(2),_424,_422);
return _424;
}else{
if(typeof node[_420]=="function"){
var _427=node[_420];
node[_420]=function(e){
_427(e);
return _424(e);
};
}else{
node[_420]=_424;
}
if(dojo.render.html.ie){
this.addClobberNodeAttrs(node,[_420]);
}
return _424;
}
};
this.isEvent=function(obj){
return (typeof obj!="undefined")&&(typeof Event!="undefined")&&(obj.eventPhase);
};
this.currentEvent=null;
this.callListener=function(_42a,_42b){
if(typeof _42a!="function"){
dojo.raise("listener not a function: "+_42a);
}
dojo.event.browser.currentEvent.currentTarget=_42b;
return _42a.call(_42b,dojo.event.browser.currentEvent);
};
this.stopPropagation=function(){
dojo.event.browser.currentEvent.cancelBubble=true;
};
this.preventDefault=function(){
dojo.event.browser.currentEvent.returnValue=false;
};
this.keys={KEY_BACKSPACE:8,KEY_TAB:9,KEY_ENTER:13,KEY_SHIFT:16,KEY_CTRL:17,KEY_ALT:18,KEY_PAUSE:19,KEY_CAPS_LOCK:20,KEY_ESCAPE:27,KEY_SPACE:32,KEY_PAGE_UP:33,KEY_PAGE_DOWN:34,KEY_END:35,KEY_HOME:36,KEY_LEFT_ARROW:37,KEY_UP_ARROW:38,KEY_RIGHT_ARROW:39,KEY_DOWN_ARROW:40,KEY_INSERT:45,KEY_DELETE:46,KEY_LEFT_WINDOW:91,KEY_RIGHT_WINDOW:92,KEY_SELECT:93,KEY_F1:112,KEY_F2:113,KEY_F3:114,KEY_F4:115,KEY_F5:116,KEY_F6:117,KEY_F7:118,KEY_F8:119,KEY_F9:120,KEY_F10:121,KEY_F11:122,KEY_F12:123,KEY_NUM_LOCK:144,KEY_SCROLL_LOCK:145};
this.revKeys=[];
for(var key in this.keys){
this.revKeys[this.keys[key]]=key;
}
this.fixEvent=function(evt,_42e){
if((!evt)&&(window["event"])){
var evt=window.event;
}
if((evt["type"])&&(evt["type"].indexOf("key")==0)){
evt.keys=this.revKeys;
for(var key in this.keys){
evt[key]=this.keys[key];
}
if((dojo.render.html.ie)&&(evt["type"]=="keypress")){
evt.charCode=evt.keyCode;
}
}
if(dojo.render.html.ie){
if(!evt.target){
evt.target=evt.srcElement;
}
if(!evt.currentTarget){
evt.currentTarget=(_42e?_42e:evt.srcElement);
}
if(!evt.layerX){
evt.layerX=evt.offsetX;
}
if(!evt.layerY){
evt.layerY=evt.offsetY;
}
var _430=((dojo.render.html.ie55)||(document["compatMode"]=="BackCompat"))?document.body:document.documentElement;
if(!evt.pageX){
evt.pageX=evt.clientX+(_430.scrollLeft||0);
}
if(!evt.pageY){
evt.pageY=evt.clientY+(_430.scrollTop||0);
}
if(evt.type=="mouseover"){
evt.relatedTarget=evt.fromElement;
}
if(evt.type=="mouseout"){
evt.relatedTarget=evt.toElement;
}
this.currentEvent=evt;
evt.callListener=this.callListener;
evt.stopPropagation=this.stopPropagation;
evt.preventDefault=this.preventDefault;
}
return evt;
};
this.stopEvent=function(ev){
if(window.event){
ev.returnValue=false;
ev.cancelBubble=true;
}else{
ev.preventDefault();
ev.stopPropagation();
}
};
};
dojo.kwCompoundRequire({common:["dojo.event","dojo.event.topic"],browser:["dojo.event.browser"],dashboard:["dojo.event.browser"]});
dojo.provide("dojo.event.*");
dojo.provide("dojo.logging.Logger");
dojo.provide("dojo.log");
dojo.require("dojo.lang");
dojo.logging.Record=function(lvl,msg){
this.level=lvl;
this.message=msg;
this.time=new Date();
};
dojo.logging.LogFilter=function(_434){
this.passChain=_434||"";
this.filter=function(_435){
return true;
};
};
dojo.logging.Logger=function(){
this.cutOffLevel=0;
this.propagate=true;
this.parent=null;
this.data=[];
this.filters=[];
this.handlers=[];
};
dojo.lang.extend(dojo.logging.Logger,{argsToArr:function(args){
var ret=[];
for(var x=0;x<args.length;x++){
ret.push(args[x]);
}
return ret;
},setLevel:function(lvl){
this.cutOffLevel=parseInt(lvl);
},isEnabledFor:function(lvl){
return parseInt(lvl)>=this.cutOffLevel;
},getEffectiveLevel:function(){
if((this.cutOffLevel==0)&&(this.parent)){
return this.parent.getEffectiveLevel();
}
return this.cutOffLevel;
},addFilter:function(flt){
this.filters.push(flt);
return this.filters.length-1;
},removeFilterByIndex:function(_43c){
if(this.filters[_43c]){
delete this.filters[_43c];
return true;
}
return false;
},removeFilter:function(_43d){
for(var x=0;x<this.filters.length;x++){
if(this.filters[x]===_43d){
delete this.filters[x];
return true;
}
}
return false;
},removeAllFilters:function(){
this.filters=[];
},filter:function(rec){
for(var x=0;x<this.filters.length;x++){
if((this.filters[x]["filter"])&&(!this.filters[x].filter(rec))||(rec.level<this.cutOffLevel)){
return false;
}
}
return true;
},addHandler:function(hdlr){
this.handlers.push(hdlr);
return this.handlers.length-1;
},handle:function(rec){
if((!this.filter(rec))||(rec.level<this.cutOffLevel)){
return false;
}
for(var x=0;x<this.handlers.length;x++){
if(this.handlers[x]["handle"]){
this.handlers[x].handle(rec);
}
}
return true;
},log:function(lvl,msg){
if((this.propagate)&&(this.parent)&&(this.parent.rec.level>=this.cutOffLevel)){
this.parent.log(lvl,msg);
return false;
}
this.handle(new dojo.logging.Record(lvl,msg));
return true;
},debug:function(msg){
return this.logType("DEBUG",this.argsToArr(arguments));
},info:function(msg){
return this.logType("INFO",this.argsToArr(arguments));
},warning:function(msg){
return this.logType("WARNING",this.argsToArr(arguments));
},error:function(msg){
return this.logType("ERROR",this.argsToArr(arguments));
},critical:function(msg){
return this.logType("CRITICAL",this.argsToArr(arguments));
},exception:function(msg,e,_44d){
if(e){
var _44e=[e.name,(e.description||e.message)];
if(e.fileName){
_44e.push(e.fileName);
_44e.push("line "+e.lineNumber);
}
msg+=" "+_44e.join(" : ");
}
this.logType("ERROR",msg);
if(!_44d){
throw e;
}
},logType:function(type,args){
var na=[dojo.logging.log.getLevel(type)];
if(typeof args=="array"){
na=na.concat(args);
}else{
if((typeof args=="object")&&(args["length"])){
na=na.concat(this.argsToArr(args));
}else{
na=na.concat(this.argsToArr(arguments).slice(1));
}
}
return this.log.apply(this,na);
}});
void (function(){
var _452=dojo.logging.Logger.prototype;
_452.warn=_452.warning;
_452.err=_452.error;
_452.crit=_452.critical;
})();
dojo.logging.LogHandler=function(_453){
this.cutOffLevel=(_453)?_453:0;
this.formatter=null;
this.data=[];
this.filters=[];
};
dojo.logging.LogHandler.prototype.setFormatter=function(fmtr){
dojo.unimplemented("setFormatter");
};
dojo.logging.LogHandler.prototype.flush=function(){
dojo.unimplemented("flush");
};
dojo.logging.LogHandler.prototype.close=function(){
dojo.unimplemented("close");
};
dojo.logging.LogHandler.prototype.handleError=function(){
dojo.unimplemented("handleError");
};
dojo.logging.LogHandler.prototype.handle=function(_455){
if((this.filter(_455))&&(_455.level>=this.cutOffLevel)){
this.emit(_455);
}
};
dojo.logging.LogHandler.prototype.emit=function(_456){
dojo.unimplemented("emit");
};
void (function(){
var _457=["setLevel","addFilter","removeFilterByIndex","removeFilter","removeAllFilters","filter"];
var tgt=dojo.logging.LogHandler.prototype;
var src=dojo.logging.Logger.prototype;
for(var x=0;x<_457.length;x++){
tgt[_457[x]]=src[_457[x]];
}
})();
dojo.logging.log=new dojo.logging.Logger();
dojo.logging.log.levels=[{"name":"DEBUG","level":1},{"name":"INFO","level":2},{"name":"WARNING","level":3},{"name":"ERROR","level":4},{"name":"CRITICAL","level":5}];
dojo.logging.log.loggers={};
dojo.logging.log.getLogger=function(name){
if(!this.loggers[name]){
this.loggers[name]=new dojo.logging.Logger();
this.loggers[name].parent=this;
}
return this.loggers[name];
};
dojo.logging.log.getLevelName=function(lvl){
for(var x=0;x<this.levels.length;x++){
if(this.levels[x].level==lvl){
return this.levels[x].name;
}
}
return null;
};
dojo.logging.log.addLevelName=function(name,lvl){
if(this.getLevelName(name)){
this.err("could not add log level "+name+" because a level with that name already exists");
return false;
}
this.levels.append({"name":name,"level":parseInt(lvl)});
return true;
};
dojo.logging.log.getLevel=function(name){
for(var x=0;x<this.levels.length;x++){
if(this.levels[x].name.toUpperCase()==name.toUpperCase()){
return this.levels[x].level;
}
}
return null;
};
dojo.logging.MemoryLogHandler=function(_462,_463,_464,_465){
dojo.logging.LogHandler.call(this,_462);
this.numRecords=(typeof djConfig["loggingNumRecords"]!="undefined")?djConfig["loggingNumRecords"]:((_463)?_463:-1);
this.postType=(typeof djConfig["loggingPostType"]!="undefined")?djConfig["loggingPostType"]:(_464||-1);
this.postInterval=(typeof djConfig["loggingPostInterval"]!="undefined")?djConfig["loggingPostInterval"]:(_464||-1);
};
dojo.logging.MemoryLogHandler.prototype=new dojo.logging.LogHandler();
dojo.logging.MemoryLogHandler.prototype.emit=function(_466){
this.data.push(_466);
if(this.numRecords!=-1){
while(this.data.length>this.numRecords){
this.data.shift();
}
}
};
dojo.logging.logQueueHandler=new dojo.logging.MemoryLogHandler(0,50,0,10000);
dojo.logging.logQueueHandler.emit=function(_467){
var _468=String(dojo.log.getLevelName(_467.level)+": "+_467.time.toLocaleTimeString())+": "+_467.message;
if(!dj_undef("debug",dj_global)){
dojo.debug(_468);
}else{
if((typeof dj_global["print"]=="function")&&(!dojo.render.html.capable)){
print(_468);
}
}
this.data.push(_467);
if(this.numRecords!=-1){
while(this.data.length>this.numRecords){
this.data.shift();
}
}
};
dojo.logging.log.addHandler(dojo.logging.logQueueHandler);
dojo.log=dojo.logging.log;
dojo.kwCompoundRequire({common:["dojo.logging.Logger",false,false],rhino:["dojo.logging.RhinoLogger"]});
dojo.provide("dojo.logging.*");
dojo.provide("dojo.io.IO");
dojo.require("dojo.string");
dojo.require("dojo.lang.extras");
dojo.io.transports=[];
dojo.io.hdlrFuncNames=["load","error","timeout"];
dojo.io.Request=function(url,_46a,_46b,_46c){
if((arguments.length==1)&&(arguments[0].constructor==Object)){
this.fromKwArgs(arguments[0]);
}else{
this.url=url;
if(_46a){
this.mimetype=_46a;
}
if(_46b){
this.transport=_46b;
}
if(arguments.length>=4){
this.changeUrl=_46c;
}
}
};
dojo.lang.extend(dojo.io.Request,{url:"",mimetype:"text/plain",method:"GET",content:undefined,transport:undefined,changeUrl:undefined,formNode:undefined,sync:false,bindSuccess:false,useCache:false,preventCache:false,load:function(type,data,evt){
},error:function(type,_471){
},timeout:function(type){
},handle:function(){
},timeoutSeconds:0,abort:function(){
},fromKwArgs:function(_473){
if(_473["url"]){
_473.url=_473.url.toString();
}
if(_473["formNode"]){
_473.formNode=dojo.byId(_473.formNode);
}
if(!_473["method"]&&_473["formNode"]&&_473["formNode"].method){
_473.method=_473["formNode"].method;
}
if(!_473["handle"]&&_473["handler"]){
_473.handle=_473.handler;
}
if(!_473["load"]&&_473["loaded"]){
_473.load=_473.loaded;
}
if(!_473["changeUrl"]&&_473["changeURL"]){
_473.changeUrl=_473.changeURL;
}
_473.encoding=dojo.lang.firstValued(_473["encoding"],djConfig["bindEncoding"],"");
_473.sendTransport=dojo.lang.firstValued(_473["sendTransport"],djConfig["ioSendTransport"],false);
var _474=dojo.lang.isFunction;
for(var x=0;x<dojo.io.hdlrFuncNames.length;x++){
var fn=dojo.io.hdlrFuncNames[x];
if(_474(_473[fn])){
continue;
}
if(_474(_473["handle"])){
_473[fn]=_473.handle;
}
}
dojo.lang.mixin(this,_473);
}});
dojo.io.Error=function(msg,type,num){
this.message=msg;
this.type=type||"unknown";
this.number=num||0;
};
dojo.io.transports.addTransport=function(name){
this.push(name);
this[name]=dojo.io[name];
};
dojo.io.bind=function(_47b){
if(!(_47b instanceof dojo.io.Request)){
try{
_47b=new dojo.io.Request(_47b);
}
catch(e){
dojo.debug(e);
}
}
var _47c="";
if(_47b["transport"]){
_47c=_47b["transport"];
if(!this[_47c]){
return _47b;
}
}else{
for(var x=0;x<dojo.io.transports.length;x++){
var tmp=dojo.io.transports[x];
if((this[tmp])&&(this[tmp].canHandle(_47b))){
_47c=tmp;
}
}
if(_47c==""){
return _47b;
}
}
this[_47c].bind(_47b);
_47b.bindSuccess=true;
return _47b;
};
dojo.io.queueBind=function(_47f){
if(!(_47f instanceof dojo.io.Request)){
try{
_47f=new dojo.io.Request(_47f);
}
catch(e){
dojo.debug(e);
}
}
var _480=_47f.load;
_47f.load=function(){
dojo.io._queueBindInFlight=false;
var ret=_480.apply(this,arguments);
dojo.io._dispatchNextQueueBind();
return ret;
};
var _482=_47f.error;
_47f.error=function(){
dojo.io._queueBindInFlight=false;
var ret=_482.apply(this,arguments);
dojo.io._dispatchNextQueueBind();
return ret;
};
dojo.io._bindQueue.push(_47f);
dojo.io._dispatchNextQueueBind();
return _47f;
};
dojo.io._dispatchNextQueueBind=function(){
if(!dojo.io._queueBindInFlight){
dojo.io._queueBindInFlight=true;
if(dojo.io._bindQueue.length>0){
dojo.io.bind(dojo.io._bindQueue.shift());
}else{
dojo.io._queueBindInFlight=false;
}
}
};
dojo.io._bindQueue=[];
dojo.io._queueBindInFlight=false;
dojo.io.argsFromMap=function(map,_485,last){
var enc=/utf/i.test(_485||"")?encodeURIComponent:dojo.string.encodeAscii;
var _488=[];
var _489=new Object();
for(var name in map){
var _48b=function(elt){
var val=enc(name)+"="+enc(elt);
_488[(last==name)?"push":"unshift"](val);
};
if(!_489[name]){
var _48e=map[name];
if(dojo.lang.isArray(_48e)){
dojo.lang.forEach(_48e,_48b);
}else{
_48b(_48e);
}
}
}
return _488.join("&");
};
dojo.io.setIFrameSrc=function(_48f,src,_491){
try{
var r=dojo.render.html;
if(!_491){
if(r.safari){
_48f.location=src;
}else{
frames[_48f.name].location=src;
}
}else{
var idoc;
if(r.ie){
idoc=_48f.contentWindow.document;
}else{
if(r.safari){
idoc=_48f.document;
}else{
idoc=_48f.contentWindow;
}
}
if(!idoc){
_48f.location=src;
return;
}else{
idoc.location.replace(src);
}
}
}
catch(e){
dojo.debug(e);
dojo.debug("setIFrameSrc: "+e);
}
};
dojo.provide("dojo.string.extras");
dojo.require("dojo.string.common");
dojo.require("dojo.lang");
dojo.string.substituteParams=function(_494,hash){
var map=(typeof hash=="object")?hash:dojo.lang.toArray(arguments,1);
return _494.replace(/\%\{(\w+)\}/g,function(_497,key){
return map[key]||dojo.raise("Substitution not found: "+key);
});
};
dojo.string.paramString=function(str,_49a,_49b){
dojo.deprecated("dojo.string.paramString","use dojo.string.substituteParams instead","0.4");
for(var name in _49a){
var re=new RegExp("\\%\\{"+name+"\\}","g");
str=str.replace(re,_49a[name]);
}
if(_49b){
str=str.replace(/%\{([^\}\s]+)\}/g,"");
}
return str;
};
dojo.string.capitalize=function(str){
if(!dojo.lang.isString(str)){
return "";
}
if(arguments.length==0){
str=this;
}
var _49f=str.split(" ");
for(var i=0;i<_49f.length;i++){
_49f[i]=_49f[i].charAt(0).toUpperCase()+_49f[i].substring(1);
}
return _49f.join(" ");
};
dojo.string.isBlank=function(str){
if(!dojo.lang.isString(str)){
return true;
}
return (dojo.string.trim(str).length==0);
};
dojo.string.encodeAscii=function(str){
if(!dojo.lang.isString(str)){
return str;
}
var ret="";
var _4a4=escape(str);
var _4a5,re=/%u([0-9A-F]{4})/i;
while((_4a5=_4a4.match(re))){
var num=Number("0x"+_4a5[1]);
var _4a7=escape("&#"+num+";");
ret+=_4a4.substring(0,_4a5.index)+_4a7;
_4a4=_4a4.substring(_4a5.index+_4a5[0].length);
}
ret+=_4a4.replace(/\+/g,"%2B");
return ret;
};
dojo.string.escape=function(type,str){
var args=dojo.lang.toArray(arguments,1);
switch(type.toLowerCase()){
case "xml":
case "html":
case "xhtml":
return dojo.string.escapeXml.apply(this,args);
case "sql":
return dojo.string.escapeSql.apply(this,args);
case "regexp":
case "regex":
return dojo.string.escapeRegExp.apply(this,args);
case "javascript":
case "jscript":
case "js":
return dojo.string.escapeJavaScript.apply(this,args);
case "ascii":
return dojo.string.encodeAscii.apply(this,args);
default:
return str;
}
};
dojo.string.escapeXml=function(str,_4ac){
str=str.replace(/&/gm,"&amp;").replace(/</gm,"&lt;").replace(/>/gm,"&gt;").replace(/"/gm,"&quot;");
if(!_4ac){
str=str.replace(/'/gm,"&#39;");
}
return str;
};
dojo.string.escapeSql=function(str){
return str.replace(/'/gm,"''");
};
dojo.string.escapeRegExp=function(str){
return str.replace(/\\/gm,"\\\\").replace(/([\f\b\n\t\r[\^$|?*+(){}])/gm,"\\$1");
};
dojo.string.escapeJavaScript=function(str){
return str.replace(/(["'\f\b\n\t\r])/gm,"\\$1");
};
dojo.string.escapeString=function(str){
return ("\""+str.replace(/(["\\])/g,"\\$1")+"\"").replace(/[\f]/g,"\\f").replace(/[\b]/g,"\\b").replace(/[\n]/g,"\\n").replace(/[\t]/g,"\\t").replace(/[\r]/g,"\\r");
};
dojo.string.summary=function(str,len){
if(!len||str.length<=len){
return str;
}else{
return str.substring(0,len).replace(/\.+$/,"")+"...";
}
};
dojo.string.endsWith=function(str,end,_4b5){
if(_4b5){
str=str.toLowerCase();
end=end.toLowerCase();
}
if((str.length-end.length)<0){
return false;
}
return str.lastIndexOf(end)==str.length-end.length;
};
dojo.string.endsWithAny=function(str){
for(var i=1;i<arguments.length;i++){
if(dojo.string.endsWith(str,arguments[i])){
return true;
}
}
return false;
};
dojo.string.startsWith=function(str,_4b9,_4ba){
if(_4ba){
str=str.toLowerCase();
_4b9=_4b9.toLowerCase();
}
return str.indexOf(_4b9)==0;
};
dojo.string.startsWithAny=function(str){
for(var i=1;i<arguments.length;i++){
if(dojo.string.startsWith(str,arguments[i])){
return true;
}
}
return false;
};
dojo.string.has=function(str){
for(var i=1;i<arguments.length;i++){
if(str.indexOf(arguments[i])>-1){
return true;
}
}
return false;
};
dojo.string.normalizeNewlines=function(text,_4c0){
if(_4c0=="\n"){
text=text.replace(/\r\n/g,"\n");
text=text.replace(/\r/g,"\n");
}else{
if(_4c0=="\r"){
text=text.replace(/\r\n/g,"\r");
text=text.replace(/\n/g,"\r");
}else{
text=text.replace(/([^\r])\n/g,"$1\r\n");
text=text.replace(/\r([^\n])/g,"\r\n$1");
}
}
return text;
};
dojo.string.splitEscaped=function(str,_4c2){
var _4c3=[];
for(var i=0,prevcomma=0;i<str.length;i++){
if(str.charAt(i)=="\\"){
i++;
continue;
}
if(str.charAt(i)==_4c2){
_4c3.push(str.substring(prevcomma,i));
prevcomma=i+1;
}
}
_4c3.push(str.substr(prevcomma));
return _4c3;
};
dojo.provide("dojo.undo.browser");
dojo.require("dojo.io");
try{
if((!djConfig["preventBackButtonFix"])&&(!dojo.hostenv.post_load_)){
document.write("<iframe style='border: 0px; width: 1px; height: 1px; position: absolute; bottom: 0px; right: 0px; visibility: visible;' name='djhistory' id='djhistory' src='"+(dojo.hostenv.getBaseScriptUri()+"iframe_history.html")+"'></iframe>");
}
}
catch(e){
}
if(dojo.render.html.opera){
dojo.debug("Opera is not supported with dojo.undo.browser, so back/forward detection will not work.");
}
dojo.undo.browser={initialHref:window.location.href,initialHash:window.location.hash,moveForward:false,historyStack:[],forwardStack:[],historyIframe:null,bookmarkAnchor:null,locationTimer:null,setInitialState:function(args){
this.initialState={"url":this.initialHref,"kwArgs":args,"urlHash":this.initialHash};
},addToHistory:function(args){
var hash=null;
if(!this.historyIframe){
this.historyIframe=window.frames["djhistory"];
}
if(!this.bookmarkAnchor){
this.bookmarkAnchor=document.createElement("a");
(document.body||document.getElementsByTagName("body")[0]).appendChild(this.bookmarkAnchor);
this.bookmarkAnchor.style.display="none";
}
if((!args["changeUrl"])||(dojo.render.html.ie)){
var url=dojo.hostenv.getBaseScriptUri()+"iframe_history.html?"+(new Date()).getTime();
this.moveForward=true;
dojo.io.setIFrameSrc(this.historyIframe,url,false);
}
if(args["changeUrl"]){
this.changingUrl=true;
hash="#"+((args["changeUrl"]!==true)?args["changeUrl"]:(new Date()).getTime());
setTimeout("window.location.href = '"+hash+"'; dojo.undo.browser.changingUrl = false;",1);
this.bookmarkAnchor.href=hash;
if(dojo.render.html.ie){
var _4c9=args["back"]||args["backButton"]||args["handle"];
var tcb=function(_4cb){
if(window.location.hash!=""){
setTimeout("window.location.href = '"+hash+"';",1);
}
_4c9.apply(this,[_4cb]);
};
if(args["back"]){
args.back=tcb;
}else{
if(args["backButton"]){
args.backButton=tcb;
}else{
if(args["handle"]){
args.handle=tcb;
}
}
}
this.forwardStack=[];
var _4cc=args["forward"]||args["forwardButton"]||args["handle"];
var tfw=function(_4ce){
if(window.location.hash!=""){
window.location.href=hash;
}
if(_4cc){
_4cc.apply(this,[_4ce]);
}
};
if(args["forward"]){
args.forward=tfw;
}else{
if(args["forwardButton"]){
args.forwardButton=tfw;
}else{
if(args["handle"]){
args.handle=tfw;
}
}
}
}else{
if(dojo.render.html.moz){
if(!this.locationTimer){
this.locationTimer=setInterval("dojo.undo.browser.checkLocation();",200);
}
}
}
}
this.historyStack.push({"url":url,"kwArgs":args,"urlHash":hash});
},checkLocation:function(){
if(!this.changingUrl){
var hsl=this.historyStack.length;
if((window.location.hash==this.initialHash||window.location.href==this.initialHref)&&(hsl==1)){
this.handleBackButton();
return;
}
if(this.forwardStack.length>0){
if(this.forwardStack[this.forwardStack.length-1].urlHash==window.location.hash){
this.handleForwardButton();
return;
}
}
if((hsl>=2)&&(this.historyStack[hsl-2])){
if(this.historyStack[hsl-2].urlHash==window.location.hash){
this.handleBackButton();
return;
}
}
}
},iframeLoaded:function(evt,_4d1){
if(!dojo.render.html.opera){
var _4d2=this._getUrlQuery(_4d1.href);
if(_4d2==null){
if(this.historyStack.length==1){
this.handleBackButton();
}
return;
}
if(this.moveForward){
this.moveForward=false;
return;
}
if(this.historyStack.length>=2&&_4d2==this._getUrlQuery(this.historyStack[this.historyStack.length-2].url)){
this.handleBackButton();
}else{
if(this.forwardStack.length>0&&_4d2==this._getUrlQuery(this.forwardStack[this.forwardStack.length-1].url)){
this.handleForwardButton();
}
}
}
},handleBackButton:function(){
var _4d3=this.historyStack.pop();
if(!_4d3){
return;
}
var last=this.historyStack[this.historyStack.length-1];
if(!last&&this.historyStack.length==0){
last=this.initialState;
}
if(last){
if(last.kwArgs["back"]){
last.kwArgs["back"]();
}else{
if(last.kwArgs["backButton"]){
last.kwArgs["backButton"]();
}else{
if(last.kwArgs["handle"]){
last.kwArgs.handle("back");
}
}
}
}
this.forwardStack.push(_4d3);
},handleForwardButton:function(){
var last=this.forwardStack.pop();
if(!last){
return;
}
if(last.kwArgs["forward"]){
last.kwArgs.forward();
}else{
if(last.kwArgs["forwardButton"]){
last.kwArgs.forwardButton();
}else{
if(last.kwArgs["handle"]){
last.kwArgs.handle("forward");
}
}
}
this.historyStack.push(last);
},_getUrlQuery:function(url){
var _4d7=url.split("?");
if(_4d7.length<2){
return null;
}else{
return _4d7[1];
}
}};
dojo.provide("dojo.io.BrowserIO");
dojo.require("dojo.io");
dojo.require("dojo.lang.array");
dojo.require("dojo.lang.func");
dojo.require("dojo.string.extras");
dojo.require("dojo.dom");
dojo.require("dojo.undo.browser");
dojo.io.checkChildrenForFile=function(node){
var _4d9=false;
var _4da=node.getElementsByTagName("input");
dojo.lang.forEach(_4da,function(_4db){
if(_4d9){
return;
}
if(_4db.getAttribute("type")=="file"){
_4d9=true;
}
});
return _4d9;
};
dojo.io.formHasFile=function(_4dc){
return dojo.io.checkChildrenForFile(_4dc);
};
dojo.io.updateNode=function(node,_4de){
node=dojo.byId(node);
var args=_4de;
if(dojo.lang.isString(_4de)){
args={url:_4de};
}
args.mimetype="text/html";
args.load=function(t,d,e){
while(node.firstChild){
if(dojo["event"]){
try{
dojo.event.browser.clean(node.firstChild);
}
catch(e){
}
}
node.removeChild(node.firstChild);
}
node.innerHTML=d;
};
dojo.io.bind(args);
};
dojo.io.formFilter=function(node){
var type=(node.type||"").toLowerCase();
return !node.disabled&&node.name&&!dojo.lang.inArray(type,["file","submit","image","reset","button"]);
};
dojo.io.encodeForm=function(_4e5,_4e6,_4e7){
if((!_4e5)||(!_4e5.tagName)||(!_4e5.tagName.toLowerCase()=="form")){
dojo.raise("Attempted to encode a non-form element.");
}
if(!_4e7){
_4e7=dojo.io.formFilter;
}
var enc=/utf/i.test(_4e6||"")?encodeURIComponent:dojo.string.encodeAscii;
var _4e9=[];
for(var i=0;i<_4e5.elements.length;i++){
var elm=_4e5.elements[i];
if(!elm||elm.tagName.toLowerCase()=="fieldset"||!_4e7(elm)){
continue;
}
var name=enc(elm.name);
var type=elm.type.toLowerCase();
if(type=="select-multiple"){
for(var j=0;j<elm.options.length;j++){
if(elm.options[j].selected){
_4e9.push(name+"="+enc(elm.options[j].value));
}
}
}else{
if(dojo.lang.inArray(type,["radio","checkbox"])){
if(elm.checked){
_4e9.push(name+"="+enc(elm.value));
}
}else{
_4e9.push(name+"="+enc(elm.value));
}
}
}
var _4ef=_4e5.getElementsByTagName("input");
for(var i=0;i<_4ef.length;i++){
var _4f0=_4ef[i];
if(_4f0.type.toLowerCase()=="image"&&_4f0.form==_4e5&&_4e7(_4f0)){
var name=enc(_4f0.name);
_4e9.push(name+"="+enc(_4f0.value));
_4e9.push(name+".x=0");
_4e9.push(name+".y=0");
}
}
return _4e9.join("&")+"&";
};
dojo.io.FormBind=function(args){
this.bindArgs={};
if(args&&args.formNode){
this.init(args);
}else{
if(args){
this.init({formNode:args});
}
}
};
dojo.lang.extend(dojo.io.FormBind,{form:null,bindArgs:null,clickedButton:null,init:function(args){
var form=dojo.byId(args.formNode);
if(!form||!form.tagName||form.tagName.toLowerCase()!="form"){
throw new Error("FormBind: Couldn't apply, invalid form");
}else{
if(this.form==form){
return;
}else{
if(this.form){
throw new Error("FormBind: Already applied to a form");
}
}
}
dojo.lang.mixin(this.bindArgs,args);
this.form=form;
this.connect(form,"onsubmit","submit");
for(var i=0;i<form.elements.length;i++){
var node=form.elements[i];
if(node&&node.type&&dojo.lang.inArray(node.type.toLowerCase(),["submit","button"])){
this.connect(node,"onclick","click");
}
}
var _4f6=form.getElementsByTagName("input");
for(var i=0;i<_4f6.length;i++){
var _4f7=_4f6[i];
if(_4f7.type.toLowerCase()=="image"&&_4f7.form==form){
this.connect(_4f7,"onclick","click");
}
}
},onSubmit:function(form){
return true;
},submit:function(e){
e.preventDefault();
if(this.onSubmit(this.form)){
dojo.io.bind(dojo.lang.mixin(this.bindArgs,{formFilter:dojo.lang.hitch(this,"formFilter")}));
}
},click:function(e){
var node=e.currentTarget;
if(node.disabled){
return;
}
this.clickedButton=node;
},formFilter:function(node){
var type=(node.type||"").toLowerCase();
var _4fe=false;
if(node.disabled||!node.name){
_4fe=false;
}else{
if(dojo.lang.inArray(type,["submit","button","image"])){
if(!this.clickedButton){
this.clickedButton=node;
}
_4fe=node==this.clickedButton;
}else{
_4fe=!dojo.lang.inArray(type,["file","submit","reset","button"]);
}
}
return _4fe;
},connect:function(_4ff,_500,_501){
if(dojo.evalObjPath("dojo.event.connect")){
dojo.event.connect(_4ff,_500,this,_501);
}else{
var fcn=dojo.lang.hitch(this,_501);
_4ff[_500]=function(e){
if(!e){
e=window.event;
}
if(!e.currentTarget){
e.currentTarget=e.srcElement;
}
if(!e.preventDefault){
e.preventDefault=function(){
window.event.returnValue=false;
};
}
fcn(e);
};
}
}});
dojo.io.XMLHTTPTransport=new function(){
var _504=this;
var _505={};
this.useCache=false;
this.preventCache=false;
function getCacheKey(url,_507,_508){
return url+"|"+_507+"|"+_508.toLowerCase();
}
function addToCache(url,_50a,_50b,http){
_505[getCacheKey(url,_50a,_50b)]=http;
}
function getFromCache(url,_50e,_50f){
return _505[getCacheKey(url,_50e,_50f)];
}
this.clearCache=function(){
_505={};
};
function doLoad(_510,http,url,_513,_514){
if(((http.status>=200)&&(http.status<300))||(http.status==304)||(location.protocol=="file:"&&(http.status==0||http.status==undefined))||(location.protocol=="chrome:"&&(http.status==0||http.status==undefined))){
var ret;
if(_510.method.toLowerCase()=="head"){
var _516=http.getAllResponseHeaders();
ret={};
ret.toString=function(){
return _516;
};
var _517=_516.split(/[\r\n]+/g);
for(var i=0;i<_517.length;i++){
var pair=_517[i].match(/^([^:]+)\s*:\s*(.+)$/i);
if(pair){
ret[pair[1]]=pair[2];
}
}
}else{
if(_510.mimetype=="text/javascript"){
try{
ret=dj_eval(http.responseText);
}
catch(e){
dojo.debug(e);
dojo.debug(http.responseText);
ret=null;
}
}else{
if(_510.mimetype=="text/json"){
try{
ret=dj_eval("("+http.responseText+")");
}
catch(e){
dojo.debug(e);
dojo.debug(http.responseText);
ret=false;
}
}else{
if((_510.mimetype=="application/xml")||(_510.mimetype=="text/xml")){
ret=http.responseXML;
if(!ret||typeof ret=="string"||!http.getResponseHeader("Content-Type")){
ret=dojo.dom.createDocumentFromText(http.responseText);
}
}else{
ret=http.responseText;
}
}
}
}
if(_514){
addToCache(url,_513,_510.method,http);
}
_510[(typeof _510.load=="function")?"load":"handle"]("load",ret,http,_510);
}else{
var _51a=new dojo.io.Error("XMLHttpTransport Error: "+http.status+" "+http.statusText);
_510[(typeof _510.error=="function")?"error":"handle"]("error",_51a,http,_510);
}
}
function setHeaders(http,_51c){
if(_51c["headers"]){
for(var _51d in _51c["headers"]){
if(_51d.toLowerCase()=="content-type"&&!_51c["contentType"]){
_51c["contentType"]=_51c["headers"][_51d];
}else{
http.setRequestHeader(_51d,_51c["headers"][_51d]);
}
}
}
}
this.inFlight=[];
this.inFlightTimer=null;
this.startWatchingInFlight=function(){
if(!this.inFlightTimer){
this.inFlightTimer=setInterval("dojo.io.XMLHTTPTransport.watchInFlight();",10);
}
};
this.watchInFlight=function(){
var now=null;
for(var x=this.inFlight.length-1;x>=0;x--){
var tif=this.inFlight[x];
if(!tif){
this.inFlight.splice(x,1);
continue;
}
if(4==tif.http.readyState){
this.inFlight.splice(x,1);
doLoad(tif.req,tif.http,tif.url,tif.query,tif.useCache);
}else{
if(tif.startTime){
if(!now){
now=(new Date()).getTime();
}
if(tif.startTime+(tif.req.timeoutSeconds*1000)<now){
if(typeof tif.http.abort=="function"){
tif.http.abort();
}
this.inFlight.splice(x,1);
tif.req[(typeof tif.req.timeout=="function")?"timeout":"handle"]("timeout",null,tif.http,tif.req);
}
}
}
}
if(this.inFlight.length==0){
clearInterval(this.inFlightTimer);
this.inFlightTimer=null;
}
};
var _521=dojo.hostenv.getXmlhttpObject()?true:false;
this.canHandle=function(_522){
return _521&&dojo.lang.inArray((_522["mimetype"].toLowerCase()||""),["text/plain","text/html","application/xml","text/xml","text/javascript","text/json"])&&!(_522["formNode"]&&dojo.io.formHasFile(_522["formNode"]));
};
this.multipartBoundary="45309FFF-BD65-4d50-99C9-36986896A96F";
this.bind=function(_523){
if(!_523["url"]){
if(!_523["formNode"]&&(_523["backButton"]||_523["back"]||_523["changeUrl"]||_523["watchForURL"])&&(!djConfig.preventBackButtonFix)){
dojo.deprecated("Using dojo.io.XMLHTTPTransport.bind() to add to browser history without doing an IO request","Use dojo.undo.browser.addToHistory() instead.","0.4");
dojo.undo.browser.addToHistory(_523);
return true;
}
}
var url=_523.url;
var _525="";
if(_523["formNode"]){
var ta=_523.formNode.getAttribute("action");
if((ta)&&(!_523["url"])){
url=ta;
}
var tp=_523.formNode.getAttribute("method");
if((tp)&&(!_523["method"])){
_523.method=tp;
}
_525+=dojo.io.encodeForm(_523.formNode,_523.encoding,_523["formFilter"]);
}
if(url.indexOf("#")>-1){
dojo.debug("Warning: dojo.io.bind: stripping hash values from url:",url);
url=url.split("#")[0];
}
if(_523["file"]){
_523.method="post";
}
if(!_523["method"]){
_523.method="get";
}
if(_523.method.toLowerCase()=="get"){
_523.multipart=false;
}else{
if(_523["file"]){
_523.multipart=true;
}else{
if(!_523["multipart"]){
_523.multipart=false;
}
}
}
if(_523["backButton"]||_523["back"]||_523["changeUrl"]){
dojo.undo.browser.addToHistory(_523);
}
var _528=_523["content"]||{};
if(_523.sendTransport){
_528["dojo.transport"]="xmlhttp";
}
do{
if(_523.postContent){
_525=_523.postContent;
break;
}
if(_528){
_525+=dojo.io.argsFromMap(_528,_523.encoding);
}
if(_523.method.toLowerCase()=="get"||!_523.multipart){
break;
}
var t=[];
if(_525.length){
var q=_525.split("&");
for(var i=0;i<q.length;++i){
if(q[i].length){
var p=q[i].split("=");
t.push("--"+this.multipartBoundary,"Content-Disposition: form-data; name=\""+p[0]+"\"","",p[1]);
}
}
}
if(_523.file){
if(dojo.lang.isArray(_523.file)){
for(var i=0;i<_523.file.length;++i){
var o=_523.file[i];
t.push("--"+this.multipartBoundary,"Content-Disposition: form-data; name=\""+o.name+"\"; filename=\""+("fileName" in o?o.fileName:o.name)+"\"","Content-Type: "+("contentType" in o?o.contentType:"application/octet-stream"),"",o.content);
}
}else{
var o=_523.file;
t.push("--"+this.multipartBoundary,"Content-Disposition: form-data; name=\""+o.name+"\"; filename=\""+("fileName" in o?o.fileName:o.name)+"\"","Content-Type: "+("contentType" in o?o.contentType:"application/octet-stream"),"",o.content);
}
}
if(t.length){
t.push("--"+this.multipartBoundary+"--","");
_525=t.join("\r\n");
}
}while(false);
var _52e=_523["sync"]?false:true;
var _52f=_523["preventCache"]||(this.preventCache==true&&_523["preventCache"]!=false);
var _530=_523["useCache"]==true||(this.useCache==true&&_523["useCache"]!=false);
if(!_52f&&_530){
var _531=getFromCache(url,_525,_523.method);
if(_531){
doLoad(_523,_531,url,_525,false);
return;
}
}
var http=dojo.hostenv.getXmlhttpObject(_523);
var _533=false;
if(_52e){
var _534=this.inFlight.push({"req":_523,"http":http,"url":url,"query":_525,"useCache":_530,"startTime":_523.timeoutSeconds?(new Date()).getTime():0});
this.startWatchingInFlight();
}
if(_523.method.toLowerCase()=="post"){
http.open("POST",url,_52e);
setHeaders(http,_523);
http.setRequestHeader("Content-Type",_523.multipart?("multipart/form-data; boundary="+this.multipartBoundary):(_523.contentType||"application/x-www-form-urlencoded"));
try{
http.send(_525);
}
catch(e){
if(typeof http.abort=="function"){
http.abort();
}
doLoad(_523,{status:404},url,_525,_530);
}
}else{
var _535=url;
if(_525!=""){
_535+=(_535.indexOf("?")>-1?"&":"?")+_525;
}
if(_52f){
_535+=(dojo.string.endsWithAny(_535,"?","&")?"":(_535.indexOf("?")>-1?"&":"?"))+"dojo.preventCache="+new Date().valueOf();
}
http.open(_523.method.toUpperCase(),_535,_52e);
setHeaders(http,_523);
try{
http.send(null);
}
catch(e){
if(typeof http.abort=="function"){
http.abort();
}
doLoad(_523,{status:404},url,_525,_530);
}
}
if(!_52e){
doLoad(_523,http,url,_525,_530);
}
_523.abort=function(){
return http.abort();
};
return;
};
dojo.io.transports.addTransport("XMLHTTPTransport");
};
dojo.provide("dojo.io.cookie");
dojo.io.cookie.setCookie=function(name,_537,days,path,_53a,_53b){
var _53c=-1;
if(typeof days=="number"&&days>=0){
var d=new Date();
d.setTime(d.getTime()+(days*24*60*60*1000));
_53c=d.toGMTString();
}
_537=escape(_537);
document.cookie=name+"="+_537+";"+(_53c!=-1?" expires="+_53c+";":"")+(path?"path="+path:"")+(_53a?"; domain="+_53a:"")+(_53b?"; secure":"");
};
dojo.io.cookie.set=dojo.io.cookie.setCookie;
dojo.io.cookie.getCookie=function(name){
var idx=document.cookie.lastIndexOf(name+"=");
if(idx==-1){
return null;
}
var _540=document.cookie.substring(idx+name.length+1);
var end=_540.indexOf(";");
if(end==-1){
end=_540.length;
}
_540=_540.substring(0,end);
_540=unescape(_540);
return _540;
};
dojo.io.cookie.get=dojo.io.cookie.getCookie;
dojo.io.cookie.deleteCookie=function(name){
dojo.io.cookie.setCookie(name,"-",0);
};
dojo.io.cookie.setObjectCookie=function(name,obj,days,path,_547,_548,_549){
if(arguments.length==5){
_549=_547;
_547=null;
_548=null;
}
var _54a=[],cookie,value="";
if(!_549){
cookie=dojo.io.cookie.getObjectCookie(name);
}
if(days>=0){
if(!cookie){
cookie={};
}
for(var prop in obj){
if(prop==null){
delete cookie[prop];
}else{
if(typeof obj[prop]=="string"||typeof obj[prop]=="number"){
cookie[prop]=obj[prop];
}
}
}
prop=null;
for(var prop in cookie){
_54a.push(escape(prop)+"="+escape(cookie[prop]));
}
value=_54a.join("&");
}
dojo.io.cookie.setCookie(name,value,days,path,_547,_548);
};
dojo.io.cookie.getObjectCookie=function(name){
var _54d=null,cookie=dojo.io.cookie.getCookie(name);
if(cookie){
_54d={};
var _54e=cookie.split("&");
for(var i=0;i<_54e.length;i++){
var pair=_54e[i].split("=");
var _551=pair[1];
if(isNaN(_551)){
_551=unescape(pair[1]);
}
_54d[unescape(pair[0])]=_551;
}
}
return _54d;
};
dojo.io.cookie.isSupported=function(){
if(typeof navigator.cookieEnabled!="boolean"){
dojo.io.cookie.setCookie("__TestingYourBrowserForCookieSupport__","CookiesAllowed",90,null);
var _552=dojo.io.cookie.getCookie("__TestingYourBrowserForCookieSupport__");
navigator.cookieEnabled=(_552=="CookiesAllowed");
if(navigator.cookieEnabled){
this.deleteCookie("__TestingYourBrowserForCookieSupport__");
}
}
return navigator.cookieEnabled;
};
if(!dojo.io.cookies){
dojo.io.cookies=dojo.io.cookie;
}
dojo.kwCompoundRequire({common:["dojo.io"],rhino:["dojo.io.RhinoIO"],browser:["dojo.io.BrowserIO","dojo.io.cookie"],dashboard:["dojo.io.BrowserIO","dojo.io.cookie"]});
dojo.provide("dojo.io.*");
dojo.kwCompoundRequire({common:["dojo.uri.Uri",false,false]});
dojo.provide("dojo.uri.*");
dojo.provide("dojo.io.IframeIO");
dojo.require("dojo.io.BrowserIO");
dojo.require("dojo.uri.*");
dojo.io.createIFrame=function(_553,_554){
if(window[_553]){
return window[_553];
}
if(window.frames[_553]){
return window.frames[_553];
}
var r=dojo.render.html;
var _556=null;
var turi=dojo.uri.dojoUri("iframe_history.html?noInit=true");
var _558=((r.ie)&&(dojo.render.os.win))?"<iframe name='"+_553+"' src='"+turi+"' onload='"+_554+"'>":"iframe";
_556=document.createElement(_558);
with(_556){
name=_553;
setAttribute("name",_553);
id=_553;
}
(document.body||document.getElementsByTagName("body")[0]).appendChild(_556);
window[_553]=_556;
with(_556.style){
position="absolute";
left=top="0px";
height=width="1px";
visibility="hidden";
}
if(!r.ie){
dojo.io.setIFrameSrc(_556,turi,true);
_556.onload=new Function(_554);
}
return _556;
};
dojo.io.iframeContentWindow=function(_559){
var win=_559.contentWindow||dojo.io.iframeContentDocument(_559).defaultView||dojo.io.iframeContentDocument(_559).__parent__||(_559.name&&document.frames[_559.name])||null;
return win;
};
dojo.io.iframeContentDocument=function(_55b){
var doc=_55b.contentDocument||((_55b.contentWindow)&&(_55b.contentWindow.document))||((_55b.name)&&(document.frames[_55b.name])&&(document.frames[_55b.name].document))||null;
return doc;
};
dojo.io.IframeTransport=new function(){
var _55d=this;
this.currentRequest=null;
this.requestQueue=[];
this.iframeName="dojoIoIframe";
this.fireNextRequest=function(){
if((this.currentRequest)||(this.requestQueue.length==0)){
return;
}
var cr=this.currentRequest=this.requestQueue.shift();
cr._contentToClean=[];
var fn=cr["formNode"];
var _560=cr["content"]||{};
if(cr.sendTransport){
_560["dojo.transport"]="iframe";
}
if(fn){
if(_560){
for(var x in _560){
if(!fn[x]){
var tn;
if(dojo.render.html.ie){
tn=document.createElement("<input type='hidden' name='"+x+"' value='"+_560[x]+"'>");
fn.appendChild(tn);
}else{
tn=document.createElement("input");
fn.appendChild(tn);
tn.type="hidden";
tn.name=x;
tn.value=_560[x];
}
cr._contentToClean.push(x);
}else{
fn[x].value=_560[x];
}
}
}
if(cr["url"]){
cr._originalAction=fn.getAttribute("action");
fn.setAttribute("action",cr.url);
}
if(!fn.getAttribute("method")){
fn.setAttribute("method",(cr["method"])?cr["method"]:"post");
}
cr._originalTarget=fn.getAttribute("target");
fn.setAttribute("target",this.iframeName);
fn.target=this.iframeName;
fn.submit();
}else{
var _563=dojo.io.argsFromMap(this.currentRequest.content);
var _564=(cr.url.indexOf("?")>-1?"&":"?")+_563;
dojo.io.setIFrameSrc(this.iframe,_564,true);
}
};
this.canHandle=function(_565){
return ((dojo.lang.inArray(_565["mimetype"],["text/plain","text/html","text/javascript","text/json"]))&&((_565["formNode"])&&(dojo.io.checkChildrenForFile(_565["formNode"])))&&(dojo.lang.inArray(_565["method"].toLowerCase(),["post","get"]))&&(!((_565["sync"])&&(_565["sync"]==true))));
};
this.bind=function(_566){
if(!this["iframe"]){
this.setUpIframe();
}
this.requestQueue.push(_566);
this.fireNextRequest();
return;
};
this.setUpIframe=function(){
this.iframe=dojo.io.createIFrame(this.iframeName,"dojo.io.IframeTransport.iframeOnload();");
};
this.iframeOnload=function(){
if(!_55d.currentRequest){
_55d.fireNextRequest();
return;
}
var req=_55d.currentRequest;
var _568=req._contentToClean;
for(var i=0;i<_568.length;i++){
var key=_568[i];
if(dojo.render.html.safari){
var _56b=req.formNode;
for(var j=0;j<_56b.childNodes.length;j++){
var _56d=_56b.childNodes[j];
if(_56d.name==key){
var _56e=_56d.parentNode;
_56e.removeChild(_56d);
break;
}
}
}else{
var _56f=req.formNode[key];
req.formNode.removeChild(_56f);
req.formNode[key]=null;
}
}
if(req["_originalAction"]){
req.formNode.setAttribute("action",req._originalAction);
}
req.formNode.setAttribute("target",req._originalTarget);
req.formNode.target=req._originalTarget;
var ifd=dojo.io.iframeContentDocument(_55d.iframe);
var _571;
var _572=false;
try{
var cmt=req.mimetype;
if((cmt=="text/javascript")||(cmt=="text/json")){
var js=ifd.getElementsByTagName("textarea")[0].value;
if(cmt=="text/json"){
js="("+js+")";
}
_571=dj_eval(js);
}else{
if(cmt=="text/html"){
_571=ifd;
}else{
_571=ifd.getElementsByTagName("textarea")[0].value;
}
}
_572=true;
}
catch(e){
var _575=new dojo.io.Error("IframeTransport Error");
if(dojo.lang.isFunction(req["error"])){
req.error("error",_575,req);
}
}
try{
if(_572&&dojo.lang.isFunction(req["load"])){
req.load("load",_571,req);
}
}
catch(e){
throw e;
}
finally{
_55d.currentRequest=null;
_55d.fireNextRequest();
}
};
dojo.io.transports.addTransport("IframeTransport");
};
dojo.provide("dojo.date");
dojo.date.setDayOfYear=function(_576,_577){
_576.setMonth(0);
_576.setDate(_577);
return _576;
};
dojo.date.getDayOfYear=function(_578){
var _579=new Date(_578.getFullYear(),0,1);
return Math.floor((_578.getTime()-_579.getTime())/86400000);
};
dojo.date.setWeekOfYear=function(_57a,week,_57c){
if(arguments.length==1){
_57c=0;
}
dojo.unimplemented("dojo.date.setWeekOfYear");
};
dojo.date.getWeekOfYear=function(_57d,_57e){
if(arguments.length==1){
_57e=0;
}
var _57f=new Date(_57d.getFullYear(),0,1);
var day=_57f.getDay();
_57f.setDate(_57f.getDate()-day+_57e-(day>_57e?7:0));
return Math.floor((_57d.getTime()-_57f.getTime())/604800000);
};
dojo.date.setIsoWeekOfYear=function(_581,week,_583){
if(arguments.length==1){
_583=1;
}
dojo.unimplemented("dojo.date.setIsoWeekOfYear");
};
dojo.date.getIsoWeekOfYear=function(_584,_585){
if(arguments.length==1){
_585=1;
}
dojo.unimplemented("dojo.date.getIsoWeekOfYear");
};
dojo.date.setIso8601=function(_586,_587){
var _588=(_587.indexOf("T")==-1)?_587.split(" "):_587.split("T");
dojo.date.setIso8601Date(_586,_588[0]);
if(_588.length==2){
dojo.date.setIso8601Time(_586,_588[1]);
}
return _586;
};
dojo.date.fromIso8601=function(_589){
return dojo.date.setIso8601(new Date(0,0),_589);
};
dojo.date.setIso8601Date=function(_58a,_58b){
var _58c="^([0-9]{4})((-?([0-9]{2})(-?([0-9]{2}))?)|"+"(-?([0-9]{3}))|(-?W([0-9]{2})(-?([1-7]))?))?$";
var d=_58b.match(new RegExp(_58c));
if(!d){
dojo.debug("invalid date string: "+_58b);
return false;
}
var year=d[1];
var _58f=d[4];
var date=d[6];
var _591=d[8];
var week=d[10];
var _593=(d[12])?d[12]:1;
_58a.setYear(year);
if(_591){
dojo.date.setDayOfYear(_58a,Number(_591));
}else{
if(week){
_58a.setMonth(0);
_58a.setDate(1);
var gd=_58a.getDay();
var day=(gd)?gd:7;
var _596=Number(_593)+(7*Number(week));
if(day<=4){
_58a.setDate(_596+1-day);
}else{
_58a.setDate(_596+8-day);
}
}else{
if(_58f){
_58a.setDate(1);
_58a.setMonth(_58f-1);
}
if(date){
_58a.setDate(date);
}
}
}
return _58a;
};
dojo.date.fromIso8601Date=function(_597){
return dojo.date.setIso8601Date(new Date(0,0),_597);
};
dojo.date.setIso8601Time=function(_598,_599){
var _59a="Z|(([-+])([0-9]{2})(:?([0-9]{2}))?)$";
var d=_599.match(new RegExp(_59a));
var _59c=0;
if(d){
if(d[0]!="Z"){
_59c=(Number(d[3])*60)+Number(d[5]);
_59c*=((d[2]=="-")?1:-1);
}
_59c-=_598.getTimezoneOffset();
_599=_599.substr(0,_599.length-d[0].length);
}
var _59d="^([0-9]{2})(:?([0-9]{2})(:?([0-9]{2})(.([0-9]+))?)?)?$";
var d=_599.match(new RegExp(_59d));
if(!d){
dojo.debug("invalid time string: "+_599);
return false;
}
var _59e=d[1];
var mins=Number((d[3])?d[3]:0);
var secs=(d[5])?d[5]:0;
var ms=d[7]?(Number("0."+d[7])*1000):0;
_598.setHours(_59e);
_598.setMinutes(mins);
_598.setSeconds(secs);
_598.setMilliseconds(ms);
return _598;
};
dojo.date.fromIso8601Time=function(_5a2){
return dojo.date.setIso8601Time(new Date(0,0),_5a2);
};
dojo.date.shortTimezones=["IDLW","BET","HST","MART","AKST","PST","MST","CST","EST","AST","NFT","BST","FST","AT","GMT","CET","EET","MSK","IRT","GST","AFT","AGTT","IST","NPT","ALMT","MMT","JT","AWST","JST","ACST","AEST","LHST","VUT","NFT","NZT","CHAST","PHOT","LINT"];
dojo.date.timezoneOffsets=[-720,-660,-600,-570,-540,-480,-420,-360,-300,-240,-210,-180,-120,-60,0,60,120,180,210,240,270,300,330,345,360,390,420,480,540,570,600,630,660,690,720,765,780,840];
dojo.date.months=["January","February","March","April","May","June","July","August","September","October","November","December"];
dojo.date.shortMonths=["Jan","Feb","Mar","Apr","May","June","July","Aug","Sep","Oct","Nov","Dec"];
dojo.date.days=["Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"];
dojo.date.shortDays=["Sun","Mon","Tues","Wed","Thur","Fri","Sat"];
dojo.date.getDaysInMonth=function(_5a3){
var _5a4=_5a3.getMonth();
var days=[31,28,31,30,31,30,31,31,30,31,30,31];
if(_5a4==1&&dojo.date.isLeapYear(_5a3)){
return 29;
}else{
return days[_5a4];
}
};
dojo.date.isLeapYear=function(_5a6){
var year=_5a6.getFullYear();
return (year%400==0)?true:(year%100==0)?false:(year%4==0)?true:false;
};
dojo.date.getDayName=function(_5a8){
return dojo.date.days[_5a8.getDay()];
};
dojo.date.getDayShortName=function(_5a9){
return dojo.date.shortDays[_5a9.getDay()];
};
dojo.date.getMonthName=function(_5aa){
return dojo.date.months[_5aa.getMonth()];
};
dojo.date.getMonthShortName=function(_5ab){
return dojo.date.shortMonths[_5ab.getMonth()];
};
dojo.date.getTimezoneName=function(_5ac){
var _5ad=-(_5ac.getTimezoneOffset());
for(var i=0;i<dojo.date.timezoneOffsets.length;i++){
if(dojo.date.timezoneOffsets[i]==_5ad){
return dojo.date.shortTimezones[i];
}
}
function $(s){
s=String(s);
while(s.length<2){
s="0"+s;
}
return s;
}
return (_5ad<0?"-":"+")+$(Math.floor(Math.abs(_5ad)/60))+":"+$(Math.abs(_5ad)%60);
};
dojo.date.getOrdinal=function(_5b0){
var date=_5b0.getDate();
if(date%100!=11&&date%10==1){
return "st";
}else{
if(date%100!=12&&date%10==2){
return "nd";
}else{
if(date%100!=13&&date%10==3){
return "rd";
}else{
return "th";
}
}
}
};
dojo.date.format=dojo.date.strftime=function(_5b2,_5b3){
var _5b4=null;
function _(s,n){
s=String(s);
n=(n||2)-s.length;
while(n-->0){
s=(_5b4==null?"0":_5b4)+s;
}
return s;
}
function $(_5b7){
switch(_5b7){
case "a":
return dojo.date.getDayShortName(_5b2);
break;
case "A":
return dojo.date.getDayName(_5b2);
break;
case "b":
case "h":
return dojo.date.getMonthShortName(_5b2);
break;
case "B":
return dojo.date.getMonthName(_5b2);
break;
case "c":
return _5b2.toLocaleString();
break;
case "C":
return _(Math.floor(_5b2.getFullYear()/100));
break;
case "d":
return _(_5b2.getDate());
break;
case "D":
return $("m")+"/"+$("d")+"/"+$("y");
break;
case "e":
if(_5b4==null){
_5b4=" ";
}
return _(_5b2.getDate(),2);
break;
case "g":
break;
case "G":
break;
case "F":
return $("Y")+"-"+$("m")+"-"+$("d");
break;
case "H":
return _(_5b2.getHours());
break;
case "I":
return _(_5b2.getHours()%12||12);
break;
case "j":
return _(dojo.date.getDayOfYear(_5b2),3);
break;
case "m":
return _(_5b2.getMonth()+1);
break;
case "M":
return _(_5b2.getMinutes());
break;
case "n":
return "\n";
break;
case "p":
return _5b2.getHours()<12?"am":"pm";
break;
case "r":
return $("I")+":"+$("M")+":"+$("S")+" "+$("p");
break;
case "R":
return $("H")+":"+$("M");
break;
case "S":
return _(_5b2.getSeconds());
break;
case "t":
return "\t";
break;
case "T":
return $("H")+":"+$("M")+":"+$("S");
break;
case "u":
return String(_5b2.getDay()||7);
break;
case "U":
return _(dojo.date.getWeekOfYear(_5b2));
break;
case "V":
return _(dojo.date.getIsoWeekOfYear(_5b2));
break;
case "W":
return _(dojo.date.getWeekOfYear(_5b2,1));
break;
case "w":
return String(_5b2.getDay());
break;
case "x":
break;
case "X":
break;
case "y":
return _(_5b2.getFullYear()%100);
break;
case "Y":
return String(_5b2.getFullYear());
break;
case "z":
var _5b8=_5b2.getTimezoneOffset();
return (_5b8<0?"-":"+")+_(Math.floor(Math.abs(_5b8)/60))+":"+_(Math.abs(_5b8)%60);
break;
case "Z":
return dojo.date.getTimezoneName(_5b2);
break;
case "%":
return "%";
break;
}
}
var _5b9="";
var i=0,index=0,switchCase;
while((index=_5b3.indexOf("%",i))!=-1){
_5b9+=_5b3.substring(i,index++);
switch(_5b3.charAt(index++)){
case "_":
_5b4=" ";
break;
case "-":
_5b4="";
break;
case "0":
_5b4="0";
break;
case "^":
switchCase="upper";
break;
case "#":
switchCase="swap";
break;
default:
_5b4=null;
index--;
break;
}
var _5bb=$(_5b3.charAt(index++));
if(switchCase=="upper"||(switchCase=="swap"&&/[a-z]/.test(_5bb))){
_5bb=_5bb.toUpperCase();
}else{
if(switchCase=="swap"&&!/[a-z]/.test(_5bb)){
_5bb=_5bb.toLowerCase();
}
}
var _5bc=null;
_5b9+=_5bb;
i=index;
}
_5b9+=_5b3.substring(i);
return _5b9;
};
dojo.date.compareTypes={DATE:1,TIME:2};
dojo.date.compare=function(_5bd,_5be,_5bf){
var dA=_5bd;
var dB=_5be||new Date();
var now=new Date();
var opt=_5bf||(dojo.date.compareTypes.DATE|dojo.date.compareTypes.TIME);
var d1=new Date(((opt&dojo.date.compareTypes.DATE)?(dA.getFullYear()):now.getFullYear()),((opt&dojo.date.compareTypes.DATE)?(dA.getMonth()):now.getMonth()),((opt&dojo.date.compareTypes.DATE)?(dA.getDate()):now.getDate()),((opt&dojo.date.compareTypes.TIME)?(dA.getHours()):0),((opt&dojo.date.compareTypes.TIME)?(dA.getMinutes()):0),((opt&dojo.date.compareTypes.TIME)?(dA.getSeconds()):0));
var d2=new Date(((opt&dojo.date.compareTypes.DATE)?(dB.getFullYear()):now.getFullYear()),((opt&dojo.date.compareTypes.DATE)?(dB.getMonth()):now.getMonth()),((opt&dojo.date.compareTypes.DATE)?(dB.getDate()):now.getDate()),((opt&dojo.date.compareTypes.TIME)?(dB.getHours()):0),((opt&dojo.date.compareTypes.TIME)?(dB.getMinutes()):0),((opt&dojo.date.compareTypes.TIME)?(dB.getSeconds()):0));
if(d1.valueOf()>d2.valueOf()){
return 1;
}
if(d1.valueOf()<d2.valueOf()){
return -1;
}
return 0;
};
dojo.date.dateParts={YEAR:0,MONTH:1,DAY:2,HOUR:3,MINUTE:4,SECOND:5,MILLISECOND:6};
dojo.date.add=function(d,unit,_5c8){
var n=(_5c8)?_5c8:1;
var v;
switch(unit){
case dojo.date.dateParts.YEAR:
v=new Date(d.getFullYear()+n,d.getMonth(),d.getDate(),d.getHours(),d.getMinutes(),d.getSeconds(),d.getMilliseconds());
break;
case dojo.date.dateParts.MONTH:
v=new Date(d.getFullYear(),d.getMonth()+n,d.getDate(),d.getHours(),d.getMinutes(),d.getSeconds(),d.getMilliseconds());
break;
case dojo.date.dateParts.HOUR:
v=new Date(d.getFullYear(),d.getMonth(),d.getDate(),d.getHours()+n,d.getMinutes(),d.getSeconds(),d.getMilliseconds());
break;
case dojo.date.dateParts.MINUTE:
v=new Date(d.getFullYear(),d.getMonth(),d.getDate(),d.getHours(),d.getMinutes()+n,d.getSeconds(),d.getMilliseconds());
break;
case dojo.date.dateParts.SECOND:
v=new Date(d.getFullYear(),d.getMonth(),d.getDate(),d.getHours(),d.getMinutes(),d.getSeconds()+n,d.getMilliseconds());
break;
case dojo.date.dateParts.MILLISECOND:
v=new Date(d.getFullYear(),d.getMonth(),d.getDate(),d.getHours(),d.getMinutes(),d.getSeconds(),d.getMilliseconds()+n);
break;
default:
v=new Date(d.getFullYear(),d.getMonth(),d.getDate()+n,d.getHours(),d.getMinutes(),d.getSeconds(),d.getMilliseconds());
}
return v;
};
dojo.date.toString=function(date,_5cc){
dojo.deprecated("dojo.date.toString","use dojo.date.format instead","0.4");
if(_5cc.indexOf("#d")>-1){
_5cc=_5cc.replace(/#dddd/g,dojo.date.getDayOfWeekName(date));
_5cc=_5cc.replace(/#ddd/g,dojo.date.getShortDayOfWeekName(date));
_5cc=_5cc.replace(/#dd/g,(date.getDate().toString().length==1?"0":"")+date.getDate());
_5cc=_5cc.replace(/#d/g,date.getDate());
}
if(_5cc.indexOf("#M")>-1){
_5cc=_5cc.replace(/#MMMM/g,dojo.date.getMonthName(date));
_5cc=_5cc.replace(/#MMM/g,dojo.date.getShortMonthName(date));
_5cc=_5cc.replace(/#MM/g,((date.getMonth()+1).toString().length==1?"0":"")+(date.getMonth()+1));
_5cc=_5cc.replace(/#M/g,date.getMonth()+1);
}
if(_5cc.indexOf("#y")>-1){
var _5cd=date.getFullYear().toString();
_5cc=_5cc.replace(/#yyyy/g,_5cd);
_5cc=_5cc.replace(/#yy/g,_5cd.substring(2));
_5cc=_5cc.replace(/#y/g,_5cd.substring(3));
}
if(_5cc.indexOf("#")==-1){
return _5cc;
}
if(_5cc.indexOf("#h")>-1){
var _5ce=date.getHours();
_5ce=(_5ce>12?_5ce-12:(_5ce==0)?12:_5ce);
_5cc=_5cc.replace(/#hh/g,(_5ce.toString().length==1?"0":"")+_5ce);
_5cc=_5cc.replace(/#h/g,_5ce);
}
if(_5cc.indexOf("#H")>-1){
_5cc=_5cc.replace(/#HH/g,(date.getHours().toString().length==1?"0":"")+date.getHours());
_5cc=_5cc.replace(/#H/g,date.getHours());
}
if(_5cc.indexOf("#m")>-1){
_5cc=_5cc.replace(/#mm/g,(date.getMinutes().toString().length==1?"0":"")+date.getMinutes());
_5cc=_5cc.replace(/#m/g,date.getMinutes());
}
if(_5cc.indexOf("#s")>-1){
_5cc=_5cc.replace(/#ss/g,(date.getSeconds().toString().length==1?"0":"")+date.getSeconds());
_5cc=_5cc.replace(/#s/g,date.getSeconds());
}
if(_5cc.indexOf("#T")>-1){
_5cc=_5cc.replace(/#TT/g,date.getHours()>=12?"PM":"AM");
_5cc=_5cc.replace(/#T/g,date.getHours()>=12?"P":"A");
}
if(_5cc.indexOf("#t")>-1){
_5cc=_5cc.replace(/#tt/g,date.getHours()>=12?"pm":"am");
_5cc=_5cc.replace(/#t/g,date.getHours()>=12?"p":"a");
}
return _5cc;
};
dojo.date.daysInMonth=function(_5cf,year){
dojo.deprecated("daysInMonth(month, year)","replaced by getDaysInMonth(dateObject)","0.4");
return dojo.date.getDaysInMonth(new Date(year,_5cf,1));
};
dojo.date.toLongDateString=function(date){
dojo.deprecated("dojo.date.toLongDateString","use dojo.date.format(date, \"%B %e, %Y\") instead","0.4");
return dojo.date.format(date,"%B %e, %Y");
};
dojo.date.toShortDateString=function(date){
dojo.deprecated("dojo.date.toShortDateString","use dojo.date.format(date, \"%b %e, %Y\") instead","0.4");
return dojo.date.format(date,"%b %e, %Y");
};
dojo.date.toMilitaryTimeString=function(date){
dojo.deprecated("dojo.date.toMilitaryTimeString","use dojo.date.format(date, \"%T\")","0.4");
return dojo.date.format(date,"%T");
};
dojo.date.toRelativeString=function(date){
var now=new Date();
var diff=(now-date)/1000;
var end=" ago";
var _5d8=false;
if(diff<0){
_5d8=true;
end=" from now";
diff=-diff;
}
if(diff<60){
diff=Math.round(diff);
return diff+" second"+(diff==1?"":"s")+end;
}else{
if(diff<3600){
diff=Math.round(diff/60);
return diff+" minute"+(diff==1?"":"s")+end;
}else{
if(diff<3600*24&&date.getDay()==now.getDay()){
diff=Math.round(diff/3600);
return diff+" hour"+(diff==1?"":"s")+end;
}else{
if(diff<3600*24*7){
diff=Math.round(diff/(3600*24));
if(diff==1){
return _5d8?"Tomorrow":"Yesterday";
}else{
return diff+" days"+end;
}
}else{
return dojo.date.toShortDateString(date);
}
}
}
}
};
dojo.date.getDayOfWeekName=function(date){
dojo.deprecated("dojo.date.getDayOfWeekName","use dojo.date.getDayName instead","0.4");
return dojo.date.days[date.getDay()];
};
dojo.date.getShortDayOfWeekName=function(date){
dojo.deprecated("dojo.date.getShortDayOfWeekName","use dojo.date.getDayShortName instead","0.4");
return dojo.date.shortDays[date.getDay()];
};
dojo.date.getShortMonthName=function(date){
dojo.deprecated("dojo.date.getShortMonthName","use dojo.date.getMonthShortName instead","0.4");
return dojo.date.shortMonths[date.getMonth()];
};
dojo.date.toSql=function(date,_5dd){
return dojo.date.format(date,"%F"+!_5dd?" %T":"");
};
dojo.date.fromSql=function(_5de){
var _5df=_5de.split(/[\- :]/g);
while(_5df.length<6){
_5df.push(0);
}
return new Date(_5df[0],(parseInt(_5df[1],10)-1),_5df[2],_5df[3],_5df[4],_5df[5]);
};
dojo.provide("dojo.string.Builder");
dojo.require("dojo.string");
dojo.string.Builder=function(str){
this.arrConcat=(dojo.render.html.capable&&dojo.render.html["ie"]);
var a=[];
var b=str||"";
var _5e3=this.length=b.length;
if(this.arrConcat){
if(b.length>0){
a.push(b);
}
b="";
}
this.toString=this.valueOf=function(){
return (this.arrConcat)?a.join(""):b;
};
this.append=function(s){
if(this.arrConcat){
a.push(s);
}else{
b+=s;
}
_5e3+=s.length;
this.length=_5e3;
return this;
};
this.clear=function(){
a=[];
b="";
_5e3=this.length=0;
return this;
};
this.remove=function(f,l){
var s="";
if(this.arrConcat){
b=a.join("");
}
a=[];
if(f>0){
s=b.substring(0,(f-1));
}
b=s+b.substring(f+l);
_5e3=this.length=b.length;
if(this.arrConcat){
a.push(b);
b="";
}
return this;
};
this.replace=function(o,n){
if(this.arrConcat){
b=a.join("");
}
a=[];
b=b.replace(o,n);
_5e3=this.length=b.length;
if(this.arrConcat){
a.push(b);
b="";
}
return this;
};
this.insert=function(idx,s){
if(this.arrConcat){
b=a.join("");
}
a=[];
if(idx==0){
b=s+b;
}else{
var t=b.split("");
t.splice(idx,0,s);
b=t.join("");
}
_5e3=this.length=b.length;
if(this.arrConcat){
a.push(b);
b="";
}
return this;
};
};
dojo.kwCompoundRequire({common:["dojo.string","dojo.string.common","dojo.string.extras","dojo.string.Builder"]});
dojo.provide("dojo.string.*");
if(!this["dojo"]){
alert("\"dojo/__package__.js\" is now located at \"dojo/dojo.js\". Please update your includes accordingly");
}
dojo.provide("dojo.AdapterRegistry");
dojo.require("dojo.lang.func");
dojo.AdapterRegistry=function(){
this.pairs=[];
};
dojo.lang.extend(dojo.AdapterRegistry,{register:function(name,_5ee,wrap,_5f0){
if(_5f0){
this.pairs.unshift([name,_5ee,wrap]);
}else{
this.pairs.push([name,_5ee,wrap]);
}
},match:function(){
for(var i=0;i<this.pairs.length;i++){
var pair=this.pairs[i];
if(pair[1].apply(this,arguments)){
return pair[2].apply(this,arguments);
}
}
throw new Error("No match found");
},unregister:function(name){
for(var i=0;i<this.pairs.length;i++){
var pair=this.pairs[i];
if(pair[0]==name){
this.pairs.splice(i,1);
return true;
}
}
return false;
}});
dojo.provide("dojo.json");
dojo.require("dojo.lang.func");
dojo.require("dojo.string.extras");
dojo.require("dojo.AdapterRegistry");
dojo.json={jsonRegistry:new dojo.AdapterRegistry(),register:function(name,_5f7,wrap,_5f9){
dojo.json.jsonRegistry.register(name,_5f7,wrap,_5f9);
},evalJson:function(json){
try{
return eval("("+json+")");
}
catch(e){
dojo.debug(e);
return json;
}
},evalJSON:function(json){
dojo.deprecated("dojo.json.evalJSON","use dojo.json.evalJson","0.4");
return this.evalJson(json);
},serialize:function(o){
var _5fd=typeof (o);
if(_5fd=="undefined"){
return "undefined";
}else{
if((_5fd=="number")||(_5fd=="boolean")){
return o+"";
}else{
if(o===null){
return "null";
}
}
}
if(_5fd=="string"){
return dojo.string.escapeString(o);
}
var me=arguments.callee;
var _5ff;
if(typeof (o.__json__)=="function"){
_5ff=o.__json__();
if(o!==_5ff){
return me(_5ff);
}
}
if(typeof (o.json)=="function"){
_5ff=o.json();
if(o!==_5ff){
return me(_5ff);
}
}
if(_5fd!="function"&&typeof (o.length)=="number"){
var res=[];
for(var i=0;i<o.length;i++){
var val=me(o[i]);
if(typeof (val)!="string"){
val="undefined";
}
res.push(val);
}
return "["+res.join(",")+"]";
}
try{
window.o=o;
_5ff=dojo.json.jsonRegistry.match(o);
return me(_5ff);
}
catch(e){
}
if(_5fd=="function"){
return null;
}
res=[];
for(var k in o){
var _604;
if(typeof (k)=="number"){
_604="\""+k+"\"";
}else{
if(typeof (k)=="string"){
_604=dojo.string.escapeString(k);
}else{
continue;
}
}
val=me(o[k]);
if(typeof (val)!="string"){
continue;
}
res.push(_604+":"+val);
}
return "{"+res.join(",")+"}";
}};
dojo.provide("dojo.Deferred");
dojo.require("dojo.lang.func");
dojo.Deferred=function(_605){
this.chain=[];
this.id=this._nextId();
this.fired=-1;
this.paused=0;
this.results=[null,null];
this.canceller=_605;
this.silentlyCancelled=false;
};
dojo.lang.extend(dojo.Deferred,{getFunctionFromArgs:function(){
var a=arguments;
if((a[0])&&(!a[1])){
if(dojo.lang.isFunction(a[0])){
return a[0];
}else{
if(dojo.lang.isString(a[0])){
return dj_global[a[0]];
}
}
}else{
if((a[0])&&(a[1])){
return dojo.lang.hitch(a[0],a[1]);
}
}
return null;
},repr:function(){
var _607;
if(this.fired==-1){
_607="unfired";
}else{
if(this.fired==0){
_607="success";
}else{
_607="error";
}
}
return "Deferred("+this.id+", "+_607+")";
},toString:dojo.lang.forward("repr"),_nextId:(function(){
var n=1;
return function(){
return n++;
};
})(),cancel:function(){
if(this.fired==-1){
if(this.canceller){
this.canceller(this);
}else{
this.silentlyCancelled=true;
}
if(this.fired==-1){
this.errback(new Error(this.repr()));
}
}else{
if((this.fired==0)&&(this.results[0] instanceof dojo.Deferred)){
this.results[0].cancel();
}
}
},_pause:function(){
this.paused++;
},_unpause:function(){
this.paused--;
if((this.paused==0)&&(this.fired>=0)){
this._fire();
}
},_continue:function(res){
this._resback(res);
this._unpause();
},_resback:function(res){
this.fired=((res instanceof Error)?1:0);
this.results[this.fired]=res;
this._fire();
},_check:function(){
if(this.fired!=-1){
if(!this.silentlyCancelled){
dojo.raise("already called!");
}
this.silentlyCancelled=false;
return;
}
},callback:function(res){
this._check();
this._resback(res);
},errback:function(res){
this._check();
if(!(res instanceof Error)){
res=new Error(res);
}
this._resback(res);
},addBoth:function(cb,cbfn){
var _60f=this.getFunctionFromArgs(cb,cbfn);
if(arguments.length>2){
_60f=dojo.lang.curryArguments(null,_60f,arguments,2);
}
return this.addCallbacks(_60f,_60f);
},addCallback:function(cb,cbfn){
var _612=this.getFunctionFromArgs(cb,cbfn);
if(arguments.length>2){
_612=dojo.lang.curryArguments(null,_612,arguments,2);
}
return this.addCallbacks(_612,null);
},addErrback:function(cb,cbfn){
var _615=this.getFunctionFromArgs(cb,cbfn);
if(arguments.length>2){
_615=dojo.lang.curryArguments(null,_615,arguments,2);
}
return this.addCallbacks(null,_615);
return this.addCallbacks(null,cbfn);
},addCallbacks:function(cb,eb){
this.chain.push([cb,eb]);
if(this.fired>=0){
this._fire();
}
return this;
},_fire:function(){
var _618=this.chain;
var _619=this.fired;
var res=this.results[_619];
var self=this;
var cb=null;
while(_618.length>0&&this.paused==0){
var pair=_618.shift();
var f=pair[_619];
if(f==null){
continue;
}
try{
res=f(res);
_619=((res instanceof Error)?1:0);
if(res instanceof dojo.Deferred){
cb=function(res){
self._continue(res);
};
this._pause();
}
}
catch(err){
_619=1;
res=err;
}
}
this.fired=_619;
this.results[_619]=res;
if((cb)&&(this.paused)){
res.addBoth(cb);
}
}});
dojo.provide("dojo.rpc.Deferred");
dojo.require("dojo.Deferred");
dojo.rpc.Deferred=dojo.Deferred;
dojo.rpc.Deferred.prototype=dojo.Deferred.prototype;
dojo.provide("dojo.rpc.RpcService");
dojo.require("dojo.io.*");
dojo.require("dojo.json");
dojo.require("dojo.lang.func");
dojo.require("dojo.rpc.Deferred");
dojo.rpc.RpcService=function(url){
if(url){
this.connect(url);
}
};
dojo.lang.extend(dojo.rpc.RpcService,{strictArgChecks:true,serviceUrl:"",parseResults:function(obj){
return obj;
},errorCallback:function(_622){
return function(type,obj,e){
_622.errback(e);
};
},resultCallback:function(_626){
var tf=dojo.lang.hitch(this,function(type,obj,e){
var _62b=this.parseResults(obj||e);
_626.callback(_62b);
});
return tf;
},generateMethod:function(_62c,_62d,url){
return dojo.lang.hitch(this,function(){
var _62f=new dojo.rpc.Deferred();
if((this.strictArgChecks)&&(_62d!=null)&&(arguments.length!=_62d.length)){
dojo.raise("Invalid number of parameters for remote method.");
}else{
this.bind(_62c,arguments,_62f,url);
}
return _62f;
});
},processSmd:function(_630){
dojo.debug("RpcService: Processing returned SMD.");
if(_630.methods){
dojo.lang.forEach(_630.methods,function(m){
if(m&&m["name"]){
dojo.debug("RpcService: Creating Method: this.",m.name,"()");
this[m.name]=this.generateMethod(m.name,m.parameters,m["url"]||m["serviceUrl"]||m["serviceURL"]);
if(dojo.lang.isFunction(this[m.name])){
dojo.debug("RpcService: Successfully created",m.name,"()");
}else{
dojo.debug("RpcService: Failed to create",m.name,"()");
}
}
},this);
}
this.serviceUrl=_630.serviceUrl||_630.serviceURL;
dojo.debug("RpcService: Dojo RpcService is ready for use.");
},connect:function(_632){
dojo.debug("RpcService: Attempting to load SMD document from:",_632);
dojo.io.bind({url:_632,mimetype:"text/json",load:dojo.lang.hitch(this,function(type,_634,e){
return this.processSmd(_634);
}),sync:true});
}});
dojo.provide("dojo.rpc.JsonService");
dojo.require("dojo.rpc.RpcService");
dojo.require("dojo.io.*");
dojo.require("dojo.json");
dojo.require("dojo.lang");
dojo.rpc.JsonService=function(args){
if(args){
if(dojo.lang.isString(args)){
this.connect(args);
}else{
if(args["smdUrl"]){
this.connect(args.smdUrl);
}
if(args["smdStr"]){
this.processSmd(dj_eval("("+args.smdStr+")"));
}
if(args["smdObj"]){
this.processSmd(args.smdObj);
}
if(args["serviceUrl"]){
this.serviceUrl=args.serviceUrl;
}
if(typeof args["strictArgChecks"]!="undefined"){
this.strictArgChecks=args.strictArgChecks;
}
}
}
};
dojo.inherits(dojo.rpc.JsonService,dojo.rpc.RpcService);
dojo.lang.extend(dojo.rpc.JsonService,{bustCache:false,contentType:"application/json-rpc",lastSubmissionId:0,callRemote:function(_637,_638){
var _639=new dojo.rpc.Deferred();
this.bind(_637,_638,_639);
return _639;
},bind:function(_63a,_63b,_63c,url){
dojo.io.bind({url:url||this.serviceUrl,postContent:this.createRequest(_63a,_63b),method:"POST",contentType:this.contentType,mimetype:"text/json",load:this.resultCallback(_63c),preventCache:this.bustCache});
},createRequest:function(_63e,_63f){
var req={"params":_63f,"method":_63e,"id":++this.lastSubmissionId};
var data=dojo.json.serialize(req);
dojo.debug("JsonService: JSON-RPC Request: "+data);
return data;
},parseResults:function(obj){
if(!obj){
return;
}
if(obj["Result"]||obj["result"]){
return obj["result"]||obj["Result"];
}else{
if(obj["ResultSet"]){
return obj["ResultSet"];
}else{
return obj;
}
}
}});
dojo.kwCompoundRequire({common:["dojo.rpc.JsonService",false,false]});
dojo.provide("dojo.rpc.*");
dojo.provide("dojo.xml.Parse");
dojo.require("dojo.dom");
dojo.xml.Parse=function(){
function getDojoTagName(node){
var _644=node.tagName;
if(_644.substr(0,5).toLowerCase()!="dojo:"){
if(_644.substr(0,4).toLowerCase()=="dojo"){
return "dojo:"+_644.substring(4).toLowerCase();
}
var djt=node.getAttribute("dojoType")||node.getAttribute("dojotype");
if(djt){
return "dojo:"+djt.toLowerCase();
}
if(node.getAttributeNS&&node.getAttributeNS(dojo.dom.dojoml,"type")){
return "dojo:"+node.getAttributeNS(dojo.dom.dojoml,"type").toLowerCase();
}
try{
djt=node.getAttribute("dojo:type");
}
catch(e){
}
if(djt){
return "dojo:"+djt.toLowerCase();
}
if(!dj_global["djConfig"]||!djConfig["ignoreClassNames"]){
var _646=node.className||node.getAttribute("class");
if(_646&&_646.indexOf&&_646.indexOf("dojo-")!=-1){
var _647=_646.split(" ");
for(var x=0;x<_647.length;x++){
if(_647[x].length>5&&_647[x].indexOf("dojo-")>=0){
return "dojo:"+_647[x].substr(5).toLowerCase();
}
}
}
}
}
return _644.toLowerCase();
}
this.parseElement=function(node,_64a,_64b,_64c){
if(node.getAttribute("parseWidgets")=="false"){
return {};
}
var _64d={};
var _64e=getDojoTagName(node);
_64d[_64e]=[];
if((!_64b)||(_64e.substr(0,4).toLowerCase()=="dojo")){
var _64f=parseAttributes(node);
for(var attr in _64f){
if((!_64d[_64e][attr])||(typeof _64d[_64e][attr]!="array")){
_64d[_64e][attr]=[];
}
_64d[_64e][attr].push(_64f[attr]);
}
_64d[_64e].nodeRef=node;
_64d.tagName=_64e;
_64d.index=_64c||0;
}
var _651=0;
var tcn,i=0,nodes=node.childNodes;
while(tcn=nodes[i++]){
switch(tcn.nodeType){
case dojo.dom.ELEMENT_NODE:
_651++;
var ctn=getDojoTagName(tcn);
if(!_64d[ctn]){
_64d[ctn]=[];
}
_64d[ctn].push(this.parseElement(tcn,true,_64b,_651));
if((tcn.childNodes.length==1)&&(tcn.childNodes.item(0).nodeType==dojo.dom.TEXT_NODE)){
_64d[ctn][_64d[ctn].length-1].value=tcn.childNodes.item(0).nodeValue;
}
break;
case dojo.dom.TEXT_NODE:
if(node.childNodes.length==1){
_64d[_64e].push({value:node.childNodes.item(0).nodeValue});
}
break;
default:
break;
}
}
return _64d;
};
function parseAttributes(node){
var _655={};
var atts=node.attributes;
var _657,i=0;
while(_657=atts[i++]){
if((dojo.render.html.capable)&&(dojo.render.html.ie)){
if(!_657){
continue;
}
if((typeof _657=="object")&&(typeof _657.nodeValue=="undefined")||(_657.nodeValue==null)||(_657.nodeValue=="")){
continue;
}
}
var nn=(_657.nodeName.indexOf("dojo:")==-1)?_657.nodeName:_657.nodeName.split("dojo:")[1];
_655[nn]={value:_657.nodeValue};
}
return _655;
}
};
dojo.provide("dojo.xml.domUtil");
dojo.require("dojo.graphics.color");
dojo.require("dojo.dom");
dojo.require("dojo.style");
dojo.deprecated("dojo.xml.domUtil","use dojo.dom instead","0.4");
dojo.xml.domUtil=new function(){
this.nodeTypes={ELEMENT_NODE:1,ATTRIBUTE_NODE:2,TEXT_NODE:3,CDATA_SECTION_NODE:4,ENTITY_REFERENCE_NODE:5,ENTITY_NODE:6,PROCESSING_INSTRUCTION_NODE:7,COMMENT_NODE:8,DOCUMENT_NODE:9,DOCUMENT_TYPE_NODE:10,DOCUMENT_FRAGMENT_NODE:11,NOTATION_NODE:12};
this.dojoml="http://www.dojotoolkit.org/2004/dojoml";
this.idIncrement=0;
this.getTagName=function(){
return dojo.dom.getTagName.apply(dojo.dom,arguments);
};
this.getUniqueId=function(){
return dojo.dom.getUniqueId.apply(dojo.dom,arguments);
};
this.getFirstChildTag=function(){
return dojo.dom.getFirstChildElement.apply(dojo.dom,arguments);
};
this.getLastChildTag=function(){
return dojo.dom.getLastChildElement.apply(dojo.dom,arguments);
};
this.getNextSiblingTag=function(){
return dojo.dom.getNextSiblingElement.apply(dojo.dom,arguments);
};
this.getPreviousSiblingTag=function(){
return dojo.dom.getPreviousSiblingElement.apply(dojo.dom,arguments);
};
this.forEachChildTag=function(node,_65a){
var _65b=this.getFirstChildTag(node);
while(_65b){
if(_65a(_65b)=="break"){
break;
}
_65b=this.getNextSiblingTag(_65b);
}
};
this.moveChildren=function(){
return dojo.dom.moveChildren.apply(dojo.dom,arguments);
};
this.copyChildren=function(){
return dojo.dom.copyChildren.apply(dojo.dom,arguments);
};
this.clearChildren=function(){
return dojo.dom.removeChildren.apply(dojo.dom,arguments);
};
this.replaceChildren=function(){
return dojo.dom.replaceChildren.apply(dojo.dom,arguments);
};
this.getStyle=function(){
return dojo.style.getStyle.apply(dojo.style,arguments);
};
this.toCamelCase=function(){
return dojo.style.toCamelCase.apply(dojo.style,arguments);
};
this.toSelectorCase=function(){
return dojo.style.toSelectorCase.apply(dojo.style,arguments);
};
this.getAncestors=function(){
return dojo.dom.getAncestors.apply(dojo.dom,arguments);
};
this.isChildOf=function(){
return dojo.dom.isDescendantOf.apply(dojo.dom,arguments);
};
this.createDocumentFromText=function(){
return dojo.dom.createDocumentFromText.apply(dojo.dom,arguments);
};
if(dojo.render.html.capable||dojo.render.svg.capable){
this.createNodesFromText=function(txt,wrap){
return dojo.dom.createNodesFromText.apply(dojo.dom,arguments);
};
}
this.extractRGB=function(_65e){
return dojo.graphics.color.extractRGB(_65e);
};
this.hex2rgb=function(hex){
return dojo.graphics.color.hex2rgb(hex);
};
this.rgb2hex=function(r,g,b){
return dojo.graphics.color.rgb2hex(r,g,b);
};
this.insertBefore=function(){
return dojo.dom.insertBefore.apply(dojo.dom,arguments);
};
this.before=this.insertBefore;
this.insertAfter=function(){
return dojo.dom.insertAfter.apply(dojo.dom,arguments);
};
this.after=this.insertAfter;
this.insert=function(){
return dojo.dom.insertAtPosition.apply(dojo.dom,arguments);
};
this.insertAtIndex=function(){
return dojo.dom.insertAtIndex.apply(dojo.dom,arguments);
};
this.textContent=function(){
return dojo.dom.textContent.apply(dojo.dom,arguments);
};
this.renderedTextContent=function(){
return dojo.dom.renderedTextContent.apply(dojo.dom,arguments);
};
this.remove=function(node){
return dojo.dom.removeNode.apply(dojo.dom,arguments);
};
};
dojo.provide("dojo.xml.htmlUtil");
dojo.require("dojo.html");
dojo.require("dojo.style");
dojo.require("dojo.dom");
dojo.deprecated("dojo.xml.htmlUtil","use dojo.html instead","0.4");
dojo.xml.htmlUtil=new function(){
this.styleSheet=dojo.style.styleSheet;
this._clobberSelection=function(){
return dojo.html.clearSelection.apply(dojo.html,arguments);
};
this.disableSelect=function(){
return dojo.html.disableSelection.apply(dojo.html,arguments);
};
this.enableSelect=function(){
return dojo.html.enableSelection.apply(dojo.html,arguments);
};
this.getInnerWidth=function(){
return dojo.style.getInnerWidth.apply(dojo.style,arguments);
};
this.getOuterWidth=function(node){
dojo.unimplemented("dojo.xml.htmlUtil.getOuterWidth");
};
this.getInnerHeight=function(){
return dojo.style.getInnerHeight.apply(dojo.style,arguments);
};
this.getOuterHeight=function(node){
dojo.unimplemented("dojo.xml.htmlUtil.getOuterHeight");
};
this.getTotalOffset=function(){
return dojo.style.getTotalOffset.apply(dojo.style,arguments);
};
this.totalOffsetLeft=function(){
return dojo.style.totalOffsetLeft.apply(dojo.style,arguments);
};
this.getAbsoluteX=this.totalOffsetLeft;
this.totalOffsetTop=function(){
return dojo.style.totalOffsetTop.apply(dojo.style,arguments);
};
this.getAbsoluteY=this.totalOffsetTop;
this.getEventTarget=function(){
return dojo.html.getEventTarget.apply(dojo.html,arguments);
};
this.getScrollTop=function(){
return dojo.html.getScrollTop.apply(dojo.html,arguments);
};
this.getScrollLeft=function(){
return dojo.html.getScrollLeft.apply(dojo.html,arguments);
};
this.evtTgt=this.getEventTarget;
this.getParentOfType=function(){
return dojo.html.getParentOfType.apply(dojo.html,arguments);
};
this.getAttribute=function(){
return dojo.html.getAttribute.apply(dojo.html,arguments);
};
this.getAttr=function(node,attr){
dojo.deprecated("dojo.xml.htmlUtil.getAttr","use dojo.xml.htmlUtil.getAttribute instead","0.4");
return dojo.xml.htmlUtil.getAttribute(node,attr);
};
this.hasAttribute=function(){
return dojo.html.hasAttribute.apply(dojo.html,arguments);
};
this.hasAttr=function(node,attr){
dojo.deprecated("dojo.xml.htmlUtil.hasAttr","use dojo.xml.htmlUtil.hasAttribute instead","0.4");
return dojo.xml.htmlUtil.hasAttribute(node,attr);
};
this.getClass=function(){
return dojo.html.getClass.apply(dojo.html,arguments);
};
this.hasClass=function(){
return dojo.html.hasClass.apply(dojo.html,arguments);
};
this.prependClass=function(){
return dojo.html.prependClass.apply(dojo.html,arguments);
};
this.addClass=function(){
return dojo.html.addClass.apply(dojo.html,arguments);
};
this.setClass=function(){
return dojo.html.setClass.apply(dojo.html,arguments);
};
this.removeClass=function(){
return dojo.html.removeClass.apply(dojo.html,arguments);
};
this.classMatchType={ContainsAll:0,ContainsAny:1,IsOnly:2};
this.getElementsByClass=function(){
return dojo.html.getElementsByClass.apply(dojo.html,arguments);
};
this.getElementsByClassName=this.getElementsByClass;
this.setOpacity=function(){
return dojo.style.setOpacity.apply(dojo.style,arguments);
};
this.getOpacity=function(){
return dojo.style.getOpacity.apply(dojo.style,arguments);
};
this.clearOpacity=function(){
return dojo.style.clearOpacity.apply(dojo.style,arguments);
};
this.gravity=function(){
return dojo.html.gravity.apply(dojo.html,arguments);
};
this.gravity.NORTH=1;
this.gravity.SOUTH=1<<1;
this.gravity.EAST=1<<2;
this.gravity.WEST=1<<3;
this.overElement=function(){
return dojo.html.overElement.apply(dojo.html,arguments);
};
this.insertCssRule=function(){
return dojo.style.insertCssRule.apply(dojo.style,arguments);
};
this.insertCSSRule=function(_66a,_66b,_66c){
dojo.deprecated("dojo.xml.htmlUtil.insertCSSRule","use dojo.style.insertCssRule instead","0.4");
return dojo.xml.htmlUtil.insertCssRule(_66a,_66b,_66c);
};
this.removeCssRule=function(){
return dojo.style.removeCssRule.apply(dojo.style,arguments);
};
this.removeCSSRule=function(_66d){
dojo.deprecated("dojo.xml.htmlUtil.removeCSSRule","use dojo.xml.htmlUtil.removeCssRule instead","0.4");
return dojo.xml.htmlUtil.removeCssRule(_66d);
};
this.insertCssFile=function(){
return dojo.style.insertCssFile.apply(dojo.style,arguments);
};
this.insertCSSFile=function(URI,doc,_670){
dojo.deprecated("dojo.xml.htmlUtil.insertCSSFile","use dojo.xml.htmlUtil.insertCssFile instead","0.4");
return dojo.xml.htmlUtil.insertCssFile(URI,doc,_670);
};
this.getBackgroundColor=function(){
return dojo.style.getBackgroundColor.apply(dojo.style,arguments);
};
this.getUniqueId=function(){
return dojo.dom.getUniqueId();
};
this.getStyle=function(){
return dojo.style.getStyle.apply(dojo.style,arguments);
};
};
dojo.require("dojo.xml.Parse");
dojo.kwCompoundRequire({common:["dojo.xml.domUtil"],browser:["dojo.xml.htmlUtil"],dashboard:["dojo.xml.htmlUtil"],svg:["dojo.xml.svgUtil"]});
dojo.provide("dojo.xml.*");
dojo.provide("dojo.lang.type");
dojo.require("dojo.lang.common");
dojo.lang.whatAmI=function(wh){
try{
if(dojo.lang.isArray(wh)){
return "array";
}
if(dojo.lang.isFunction(wh)){
return "function";
}
if(dojo.lang.isString(wh)){
return "string";
}
if(dojo.lang.isNumber(wh)){
return "number";
}
if(dojo.lang.isBoolean(wh)){
return "boolean";
}
if(dojo.lang.isAlien(wh)){
return "alien";
}
if(dojo.lang.isUndefined(wh)){
return "undefined";
}
for(var name in dojo.lang.whatAmI.custom){
if(dojo.lang.whatAmI.custom[name](wh)){
return name;
}
}
if(dojo.lang.isObject(wh)){
return "object";
}
}
catch(E){
}
return "unknown";
};
dojo.lang.whatAmI.custom={};
dojo.lang.isNumeric=function(wh){
return (!isNaN(wh)&&isFinite(wh)&&(wh!=null)&&!dojo.lang.isBoolean(wh)&&!dojo.lang.isArray(wh));
};
dojo.lang.isBuiltIn=function(wh){
return (dojo.lang.isArray(wh)||dojo.lang.isFunction(wh)||dojo.lang.isString(wh)||dojo.lang.isNumber(wh)||dojo.lang.isBoolean(wh)||(wh==null)||(wh instanceof Error)||(typeof wh=="error"));
};
dojo.lang.isPureObject=function(wh){
return ((wh!=null)&&dojo.lang.isObject(wh)&&wh.constructor==Object);
};
dojo.lang.isOfType=function(_676,type){
if(dojo.lang.isArray(type)){
var _678=type;
for(var i in _678){
var _67a=_678[i];
if(dojo.lang.isOfType(_676,_67a)){
return true;
}
}
return false;
}else{
if(dojo.lang.isString(type)){
type=type.toLowerCase();
}
switch(type){
case Array:
case "array":
return dojo.lang.isArray(_676);
break;
case Function:
case "function":
return dojo.lang.isFunction(_676);
break;
case String:
case "string":
return dojo.lang.isString(_676);
break;
case Number:
case "number":
return dojo.lang.isNumber(_676);
break;
case "numeric":
return dojo.lang.isNumeric(_676);
break;
case Boolean:
case "boolean":
return dojo.lang.isBoolean(_676);
break;
case Object:
case "object":
return dojo.lang.isObject(_676);
break;
case "pureobject":
return dojo.lang.isPureObject(_676);
break;
case "builtin":
return dojo.lang.isBuiltIn(_676);
break;
case "alien":
return dojo.lang.isAlien(_676);
break;
case "undefined":
return dojo.lang.isUndefined(_676);
break;
case null:
case "null":
return (_676===null);
break;
case "optional":
return ((_676===null)||dojo.lang.isUndefined(_676));
break;
default:
if(dojo.lang.isFunction(type)){
return (_676 instanceof type);
}else{
dojo.raise("dojo.lang.isOfType() was passed an invalid type");
}
break;
}
}
dojo.raise("If we get here, it means a bug was introduced above.");
};
dojo.lang.getObject=function(str){
var _67c=str.split("."),i=0,obj=dj_global;
do{
obj=obj[_67c[i++]];
}while(i<_67c.length&&obj);
return (obj!=dj_global)?obj:null;
};
dojo.lang.doesObjectExist=function(str){
var _67e=str.split("."),i=0,obj=dj_global;
do{
obj=obj[_67e[i++]];
}while(i<_67e.length&&obj);
return (obj&&obj!=dj_global);
};
dojo.provide("dojo.lang.assert");
dojo.require("dojo.lang.common");
dojo.require("dojo.lang.array");
dojo.require("dojo.lang.type");
dojo.lang.assert=function(_67f,_680){
if(!_67f){
var _681="An assert statement failed.\n"+"The method dojo.lang.assert() was called with a 'false' value.\n";
if(_680){
_681+="Here's the assert message:\n"+_680+"\n";
}
throw new Error(_681);
}
};
dojo.lang.assertType=function(_682,type,_684){
if(!dojo.lang.isOfType(_682,type)){
if(!_684){
if(!dojo.lang.assertType._errorMessage){
dojo.lang.assertType._errorMessage="Type mismatch: dojo.lang.assertType() failed.";
}
_684=dojo.lang.assertType._errorMessage;
}
dojo.lang.assert(false,_684);
}
};
dojo.lang.assertValidKeywords=function(_685,_686,_687){
var key;
if(!_687){
if(!dojo.lang.assertValidKeywords._errorMessage){
dojo.lang.assertValidKeywords._errorMessage="In dojo.lang.assertValidKeywords(), found invalid keyword:";
}
_687=dojo.lang.assertValidKeywords._errorMessage;
}
if(dojo.lang.isArray(_686)){
for(key in _685){
if(!dojo.lang.inArray(_686,key)){
dojo.lang.assert(false,_687+" "+key);
}
}
}else{
for(key in _685){
if(!(key in _686)){
dojo.lang.assert(false,_687+" "+key);
}
}
}
};
dojo.provide("dojo.lang.repr");
dojo.require("dojo.lang.common");
dojo.require("dojo.AdapterRegistry");
dojo.require("dojo.string.extras");
dojo.lang.reprRegistry=new dojo.AdapterRegistry();
dojo.lang.registerRepr=function(name,_68a,wrap,_68c){
dojo.lang.reprRegistry.register(name,_68a,wrap,_68c);
};
dojo.lang.repr=function(obj){
if(typeof (obj)=="undefined"){
return "undefined";
}else{
if(obj===null){
return "null";
}
}
try{
if(typeof (obj["__repr__"])=="function"){
return obj["__repr__"]();
}else{
if((typeof (obj["repr"])=="function")&&(obj.repr!=arguments.callee)){
return obj["repr"]();
}
}
return dojo.lang.reprRegistry.match(obj);
}
catch(e){
if(typeof (obj.NAME)=="string"&&(obj.toString==Function.prototype.toString||obj.toString==Object.prototype.toString)){
return o.NAME;
}
}
if(typeof (obj)=="function"){
obj=(obj+"").replace(/^\s+/,"");
var idx=obj.indexOf("{");
if(idx!=-1){
obj=obj.substr(0,idx)+"{...}";
}
}
return obj+"";
};
dojo.lang.reprArrayLike=function(arr){
try{
var na=dojo.lang.map(arr,dojo.lang.repr);
return "["+na.join(", ")+"]";
}
catch(e){
}
};
dojo.lang.reprString=function(str){
dojo.deprecated("dojo.lang.reprNumber","use `String(num)` instead","0.4");
return dojo.string.escapeString(str);
};
dojo.lang.reprNumber=function(num){
dojo.deprecated("dojo.lang.reprNumber","use `String(num)` instead","0.4");
return num+"";
};
(function(){
var m=dojo.lang;
m.registerRepr("arrayLike",m.isArrayLike,m.reprArrayLike);
m.registerRepr("string",m.isString,m.reprString);
m.registerRepr("numbers",m.isNumber,m.reprNumber);
m.registerRepr("boolean",m.isBoolean,m.reprNumber);
})();
dojo.provide("dojo.lang.declare");
dojo.require("dojo.lang.common");
dojo.require("dojo.lang.extras");
dojo.lang.declare=function(_694,_695,init,_697){
if((dojo.lang.isFunction(_697))||((!_697)&&(!dojo.lang.isFunction(init)))){
var temp=_697;
_697=init;
init=temp;
}
var _699=[];
if(dojo.lang.isArray(_695)){
_699=_695;
_695=_699.shift();
}
if(!init){
init=dojo.evalObjPath(_694,false);
if((init)&&(!dojo.lang.isFunction(init))){
init=null;
}
}
var ctor=dojo.lang.declare._makeConstructor();
var scp=(_695?_695.prototype:null);
if(scp){
scp.prototyping=true;
ctor.prototype=new _695();
scp.prototyping=false;
}
ctor.superclass=scp;
ctor.mixins=_699;
for(var i=0,l=_699.length;i<l;i++){
dojo.lang.extend(ctor,_699[i].prototype);
}
ctor.prototype.initializer=null;
ctor.prototype.declaredClass=_694;
if(dojo.lang.isArray(_697)){
dojo.lang.extend.apply(dojo.lang,[ctor].concat(_697));
}else{
dojo.lang.extend(ctor,(_697)||{});
}
dojo.lang.extend(ctor,dojo.lang.declare.base);
ctor.prototype.constructor=ctor;
ctor.prototype.initializer=(ctor.prototype.initializer)||(init)||(function(){
});
dojo.lang.setObjPathValue(_694,ctor,null,true);
};
dojo.lang.declare._makeConstructor=function(){
return function(){
var self=this._getPropContext();
var s=self.constructor.superclass;
if((s)&&(s.constructor)){
if(s.constructor==arguments.callee){
this.inherited("constructor",arguments);
}else{
this._inherited(s,"constructor",arguments);
}
}
var m=(self.constructor.mixins)||([]);
for(var i=0,l=m.length;i<l;i++){
(((m[i].prototype)&&(m[i].prototype.initializer))||(m[i])).apply(this,arguments);
}
if((!this.prototyping)&&(self.initializer)){
self.initializer.apply(this,arguments);
}
};
};
dojo.lang.declare.base={_getPropContext:function(){
return (this.___proto||this);
},_inherited:function(_6a1,_6a2,args){
var _6a4=this.___proto;
this.___proto=_6a1;
var _6a5=_6a1[_6a2].apply(this,(args||[]));
this.___proto=_6a4;
return _6a5;
},inheritedFrom:function(ctor,prop,args){
var p=((ctor)&&(ctor.prototype)&&(ctor.prototype[prop]));
return (dojo.lang.isFunction(p)?p.apply(this,(args||[])):p);
},inherited:function(prop,args){
var p=this._getPropContext();
do{
if((!p.constructor)||(!p.constructor.superclass)){
return;
}
p=p.constructor.superclass;
}while(!(prop in p));
return (dojo.lang.isFunction(p[prop])?this._inherited(p,prop,args):p[prop]);
}};
dojo.declare=dojo.lang.declare;
dojo.kwCompoundRequire({common:["dojo.lang","dojo.lang.common","dojo.lang.assert","dojo.lang.array","dojo.lang.type","dojo.lang.func","dojo.lang.extras","dojo.lang.repr","dojo.lang.declare"]});
dojo.provide("dojo.lang.*");
dojo.provide("dojo.storage");
dojo.provide("dojo.storage.StorageProvider");
dojo.require("dojo.lang.*");
dojo.require("dojo.event.*");
dojo.storage=function(){
};
dojo.lang.extend(dojo.storage,{SUCCESS:"success",FAILED:"failed",PENDING:"pending",SIZE_NOT_AVAILABLE:"Size not available",SIZE_NO_LIMIT:"No size limit",namespace:"dojoStorage",onHideSettingsUI:null,initialize:function(){
dojo.unimplemented("dojo.storage.initialize");
},isAvailable:function(){
dojo.unimplemented("dojo.storage.isAvailable");
},put:function(key,_6ae,_6af){
dojo.unimplemented("dojo.storage.put");
},get:function(key){
dojo.unimplemented("dojo.storage.get");
},hasKey:function(key){
if(this.get(key)!=null){
return true;
}else{
return false;
}
},getKeys:function(){
dojo.unimplemented("dojo.storage.getKeys");
},clear:function(){
dojo.unimplemented("dojo.storage.clear");
},remove:function(key){
dojo.unimplemented("dojo.storage.remove");
},isPermanent:function(){
dojo.unimplemented("dojo.storage.isPermanent");
},getMaximumSize:function(){
dojo.unimplemented("dojo.storage.getMaximumSize");
},hasSettingsUI:function(){
return false;
},showSettingsUI:function(){
dojo.unimplemented("dojo.storage.showSettingsUI");
},hideSettingsUI:function(){
dojo.unimplemented("dojo.storage.hideSettingsUI");
},getType:function(){
dojo.unimplemented("dojo.storage.getType");
},isValidKey:function(_6b3){
if(_6b3==null||typeof _6b3=="undefined"){
return false;
}
return /^[0-9A-Za-z_]*$/.test(_6b3);
}});
dojo.storage.manager=new function(){
this.currentProvider=null;
this.available=false;
this.initialized=false;
this.providers=new Array();
this.namespace="dojo.storage";
this.initialize=function(){
this.autodetect();
};
this.register=function(name,_6b5){
this.providers[this.providers.length]=_6b5;
this.providers[name]=_6b5;
};
this.setProvider=function(_6b6){
};
this.autodetect=function(){
if(this.initialized==true){
return;
}
var _6b7=null;
for(var i=0;i<this.providers.length;i++){
_6b7=this.providers[i];
if(_6b7.isAvailable()){
break;
}
}
if(_6b7==null){
this.initialized=true;
this.available=false;
this.currentProvider=null;
dojo.raise("No storage provider found for this platform");
}
this.currentProvider=_6b7;
for(var i in _6b7){
dojo.storage[i]=_6b7[i];
}
dojo.storage.manager=this;
dojo.storage.initialize();
this.initialized=true;
this.available=true;
};
this.isAvailable=function(){
return this.available;
};
this.isInitialized=function(){
if(dojo.flash.ready==false){
return false;
}else{
return this.initialized;
}
};
this.supportsProvider=function(_6b9){
try{
var _6ba=eval("new "+_6b9+"()");
var _6bb=_6ba.isAvailable();
if(_6bb==null||typeof _6bb=="undefined"){
return false;
}
return _6bb;
}
catch(exception){
dojo.debug("exception="+exception);
return false;
}
};
this.getProvider=function(){
return this.currentProvider;
};
this.loaded=function(){
};
};
dojo.provide("dojo.flash");
dojo.require("dojo.string.*");
dojo.require("dojo.uri.*");
dojo.flash={flash6_version:null,flash8_version:null,ready:false,_visible:true,_loadedListeners:new Array(),_installingListeners:new Array(),setSwf:function(_6bc){
if(_6bc==null||dojo.lang.isUndefined(_6bc)){
return;
}
if(_6bc.flash6!=null&&!dojo.lang.isUndefined(_6bc.flash6)){
this.flash6_version=_6bc.flash6;
}
if(_6bc.flash8!=null&&!dojo.lang.isUndefined(_6bc.flash8)){
this.flash8_version=_6bc.flash8;
}
if(!dojo.lang.isUndefined(_6bc.visible)){
this._visible=_6bc.visible;
}
this._initialize();
},useFlash6:function(){
if(this.flash6_version==null){
return false;
}else{
if(this.flash6_version!=null&&dojo.flash.info.commVersion==6){
return true;
}else{
return false;
}
}
},useFlash8:function(){
if(this.flash8_version==null){
return false;
}else{
if(this.flash8_version!=null&&dojo.flash.info.commVersion==8){
return true;
}else{
return false;
}
}
},addLoadedListener:function(_6bd){
this._loadedListeners.push(_6bd);
},addInstallingListener:function(_6be){
this._installingListeners.push(_6be);
},loaded:function(){
dojo.flash.ready=true;
if(dojo.flash._loadedListeners.length>0){
for(var i=0;i<dojo.flash._loadedListeners.length;i++){
dojo.flash._loadedListeners[i].call(null);
}
}
},installing:function(){
if(dojo.flash._installingListeners.length>0){
for(var i=0;i<dojo.flash._installingListeners.length;i++){
dojo.flash._installingListeners[i].call(null);
}
}
},_initialize:function(){
var _6c1=new dojo.flash.Install();
dojo.flash.installer=_6c1;
if(_6c1.needed()==true){
_6c1.install();
}else{
dojo.flash.obj=new dojo.flash.Embed(this._visible);
dojo.flash.obj.write(dojo.flash.info.commVersion);
dojo.flash.comm=new dojo.flash.Communicator();
}
}};
dojo.flash.Info=function(){
if(dojo.render.html.ie){
document.writeln("<script language=\"VBScript\" type=\"text/vbscript\">");
document.writeln("Function VBGetSwfVer(i)");
document.writeln("  on error resume next");
document.writeln("  Dim swControl, swVersion");
document.writeln("  swVersion = 0");
document.writeln("  set swControl = CreateObject(\"ShockwaveFlash.ShockwaveFlash.\" + CStr(i))");
document.writeln("  if (IsObject(swControl)) then");
document.writeln("    swVersion = swControl.GetVariable(\"$version\")");
document.writeln("  end if");
document.writeln("  VBGetSwfVer = swVersion");
document.writeln("End Function");
document.writeln("</script>");
}
this._detectVersion();
this._detectCommunicationVersion();
};
dojo.flash.Info.prototype={version:-1,versionMajor:-1,versionMinor:-1,versionRevision:-1,capable:false,commVersion:6,installing:false,isVersionOrAbove:function(_6c2,_6c3,_6c4){
_6c4=parseFloat("."+_6c4);
if(this.versionMajor>=_6c2&&this.versionMinor>=_6c3&&this.versionRevision>=_6c4){
return true;
}else{
return false;
}
},_detectVersion:function(){
var _6c5;
for(var _6c6=25;_6c6>0;_6c6--){
if(dojo.render.html.ie){
_6c5=VBGetSwfVer(_6c6);
}else{
_6c5=this._JSFlashInfo(_6c6);
}
if(_6c5==-1){
this.capable=false;
return;
}else{
if(_6c5!=0){
var _6c7;
if(dojo.render.html.ie){
var _6c8=_6c5.split(" ");
var _6c9=_6c8[1];
_6c7=_6c9.split(",");
}else{
_6c7=_6c5.split(".");
}
this.versionMajor=_6c7[0];
this.versionMinor=_6c7[1];
this.versionRevision=_6c7[2];
var _6ca=this.versionMajor+"."+this.versionRevision;
this.version=parseFloat(_6ca);
this.capable=true;
break;
}
}
}
},_JSFlashInfo:function(_6cb){
if(navigator.plugins!=null&&navigator.plugins.length>0){
if(navigator.plugins["Shockwave Flash 2.0"]||navigator.plugins["Shockwave Flash"]){
var _6cc=navigator.plugins["Shockwave Flash 2.0"]?" 2.0":"";
var _6cd=navigator.plugins["Shockwave Flash"+_6cc].description;
var _6ce=_6cd.split(" ");
var _6cf=_6ce[2].split(".");
var _6d0=_6cf[0];
var _6d1=_6cf[1];
if(_6ce[3]!=""){
var _6d2=_6ce[3].split("r");
}else{
var _6d2=_6ce[4].split("r");
}
var _6d3=_6d2[1]>0?_6d2[1]:0;
var _6d4=_6d0+"."+_6d1+"."+_6d3;
return _6d4;
}
}
return -1;
},_detectCommunicationVersion:function(){
if(this.capable==false){
this.commVersion=null;
return;
}
if(typeof djConfig["forceFlashComm"]!="undefined"&&typeof djConfig["forceFlashComm"]!=null){
this.commVersion=djConfig["forceFlashComm"];
return;
}
if(dojo.render.html.safari==true||dojo.render.html.opera==true){
this.commVersion=8;
}else{
this.commVersion=6;
}
}};
dojo.flash.Embed=function(_6d5){
this._visible=_6d5;
};
dojo.flash.Embed.prototype={width:215,height:138,id:"flashObject",_visible:true,write:function(_6d6,_6d7){
if(dojo.lang.isUndefined(_6d7)){
_6d7=false;
}
var _6d8=new dojo.string.Builder();
_6d8.append("width: "+this.width+"px; ");
_6d8.append("height: "+this.height+"px; ");
if(this._visible==false){
_6d8.append("position: absolute; ");
_6d8.append("z-index: 10000; ");
_6d8.append("top: -1000px; ");
_6d8.append("left: -1000px; ");
}
_6d8=_6d8.toString();
var _6d9;
var _6da;
if(_6d6==6){
_6da=dojo.flash.flash6_version;
var _6db=djConfig.baseRelativePath;
_6da=_6da+"?baseRelativePath="+escape(_6db);
_6d9="<embed id=\""+this.id+"\" src=\""+_6da+"\" "+"    quality=\"high\" bgcolor=\"#ffffff\" "+"    width=\""+this.width+"\" height=\""+this.height+"\" "+"    name=\""+this.id+"\" "+"    align=\"middle\" allowScriptAccess=\"sameDomain\" "+"    type=\"application/x-shockwave-flash\" swLiveConnect=\"true\" "+"    pluginspage=\"http://www.macromedia.com/go/getflashplayer\">";
}else{
_6da=dojo.flash.flash8_version;
var _6dc=_6da;
var _6dd=_6da;
var _6db=djConfig.baseRelativePath;
if(_6d7){
var _6de=escape(window.location);
document.title=document.title.slice(0,47)+" - Flash Player Installation";
var _6df=escape(document.title);
_6dc+="?MMredirectURL="+_6de+"&MMplayerType=ActiveX"+"&MMdoctitle="+_6df+"&baseRelativePath="+escape(_6db);
_6dd+="?MMredirectURL="+_6de+"&MMplayerType=PlugIn"+"&baseRelativePath="+escape(_6db);
}
_6d9="<object classid=\"clsid:d27cdb6e-ae6d-11cf-96b8-444553540000\" "+"codebase=\"http://fpdownload.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=8,0,0,0\" "+"width=\""+this.width+"\" "+"height=\""+this.height+"\" "+"id=\""+this.id+"\" "+"align=\"middle\"> "+"<param name=\"allowScriptAccess\" value=\"sameDomain\" /> "+"<param name=\"movie\" value=\""+_6dc+"\" /> "+"<param name=\"quality\" value=\"high\" /> "+"<param name=\"bgcolor\" value=\"#ffffff\" /> "+"<embed src=\""+_6dd+"\" "+"quality=\"high\" "+"bgcolor=\"#ffffff\" "+"width=\""+this.width+"\" "+"height=\""+this.height+"\" "+"id=\""+this.id+"\" "+"name=\""+this.id+"\" "+"swLiveConnect=\"true\" "+"align=\"middle\" "+"allowScriptAccess=\"sameDomain\" "+"type=\"application/x-shockwave-flash\" "+"&baseRelativePath="+escape(_6db);
+"pluginspage=\"http://www.macromedia.com/go/getflashplayer\" />"+"</object>";
}
_6d9="<div id=\""+this.id+"Container\" style=\""+_6d8+"\"> "+_6d9+"</div>";
document.writeln(_6d9);
},get:function(){
return document.getElementById(this.id);
},setVisible:function(_6e0){
var _6e1=dojo.byId(this.id+"Container");
if(_6e0==true){
_6e1.style.visibility="visible";
}else{
_6e1.style.position="absolute";
_6e1.style.x="-1000px";
_6e1.style.y="-1000px";
_6e1.style.visibility="hidden";
}
},center:function(){
var _6e2=this.width;
var _6e3=this.height;
var _6e4=document.body.clientWidth;
var _6e5=document.body.clientHeight;
if(!dojo.render.html.ie&&document.compatMode=="CSS1Compat"){
_6e4=document.body.parentNode.clientWidth;
_6e5=document.body.parentNode.clientHeight;
}else{
if(dojo.render.html.ie&&document.compatMode=="CSS1Compat"){
_6e4=document.documentElement.clientWidth;
_6e5=document.documentElement.clientHeight;
}else{
if(dojo.render.html.safari){
_6e5=self.innerHeight;
}
}
}
var _6e6=window.scrollX;
var _6e7=window.scrollY;
if(typeof _6e6=="undefined"){
if(document.compatMode=="CSS1Compat"){
_6e6=document.documentElement.scrollLeft;
_6e7=document.documentElement.scrollTop;
}else{
_6e6=document.body.scrollLeft;
_6e7=document.body.scrollTop;
}
}
var x=_6e6+(_6e4-_6e2)/2;
var y=_6e7+(_6e5-_6e3)/2;
var _6ea=dojo.byId(this.id+"Container");
_6ea.style.top=y+"px";
_6ea.style.left=x+"px";
}};
dojo.flash.Communicator=function(){
if(dojo.flash.useFlash6()){
this._writeFlash6();
}else{
if(dojo.flash.useFlash8()){
this._writeFlash8();
}
}
};
dojo.flash.Communicator.prototype={_writeFlash6:function(){
var id=dojo.flash.obj.id;
document.writeln("<script language=\"JavaScript\">");
document.writeln("  function "+id+"_DoFSCommand(command, args){ ");
document.writeln("    dojo.flash.comm._handleFSCommand(command, args); ");
document.writeln("}");
document.writeln("</script>");
if(dojo.render.html.ie){
document.writeln("<SCRIPT LANGUAGE=VBScript> ");
document.writeln("on error resume next ");
document.writeln("Sub "+id+"_FSCommand(ByVal command, ByVal args)");
document.writeln(" call "+id+"_DoFSCommand(command, args)");
document.writeln("end sub");
document.writeln("</SCRIPT> ");
}
},_writeFlash8:function(){
},_handleFSCommand:function(_6ec,args){
if(_6ec!=null&&!dojo.lang.isUndefined(_6ec)&&/^FSCommand:(.*)/.test(_6ec)==true){
_6ec=_6ec.match(/^FSCommand:(.*)/)[1];
}
if(_6ec=="addCallback"){
this._fscommandAddCallback(_6ec,args);
}else{
if(_6ec=="call"){
this._fscommandCall(_6ec,args);
}else{
if(_6ec=="fscommandReady"){
this._fscommandReady();
}
}
}
},_fscommandAddCallback:function(_6ee,args){
var _6f0=args;
var _6f1=function(){
return dojo.flash.comm._call(_6f0,arguments);
};
dojo.flash.comm[_6f0]=_6f1;
dojo.flash.obj.get().SetVariable("_succeeded",true);
},_fscommandCall:function(_6f2,args){
var _6f4=dojo.flash.obj.get();
var _6f5=args;
var _6f6=parseInt(_6f4.GetVariable("_numArgs"));
var _6f7=new Array();
for(var i=0;i<_6f6;i++){
var _6f9=_6f4.GetVariable("_"+i);
_6f7.push(_6f9);
}
var _6fa;
if(_6f5.indexOf(".")==-1){
_6fa=window[_6f5];
}else{
_6fa=eval(_6f5);
}
var _6fb=null;
if(!dojo.lang.isUndefined(_6fa)&&_6fa!=null){
_6fb=_6fa.apply(null,_6f7);
}
_6f4.SetVariable("_returnResult",_6fb);
},_fscommandReady:function(){
var _6fc=dojo.flash.obj.get();
_6fc.SetVariable("fscommandReady","true");
},_call:function(_6fd,args){
var _6ff=dojo.flash.obj.get();
_6ff.SetVariable("_functionName",_6fd);
_6ff.SetVariable("_numArgs",args.length);
for(var i=0;i<args.length;i++){
var _701=args[i];
_701=_701.replace(/\0/g,"\\0");
_6ff.SetVariable("_"+i,_701);
}
_6ff.TCallLabel("/_flashRunner","execute");
var _702=_6ff.GetVariable("_returnResult");
_702=_702.replace(/\\0/g,"\x00");
return _702;
},_addExternalInterfaceCallback:function(_703){
var _704=function(){
var _705=new Array(arguments.length);
for(var i=0;i<arguments.length;i++){
_705[i]=arguments[i];
}
return dojo.flash.comm._execFlash(_703,_705);
};
dojo.flash.comm[_703]=_704;
},_encodeData:function(data){
var _708=/\&([^;]*)\;/g;
data=data.replace(_708,"&amp;$1;");
data=data.replace(/</g,"&lt;");
data=data.replace(/>/g,"&gt;");
data=data.replace("\\","&custom_backslash;&custom_backslash;");
data=data.replace(/\n/g,"\\n");
data=data.replace(/\r/g,"\\r");
data=data.replace(/\f/g,"\\f");
data=data.replace(/\0/g,"\\0");
data=data.replace(/\'/g,"\\'");
data=data.replace(/\"/g,"\\\"");
return data;
},_decodeData:function(data){
if(data==null||typeof data=="undefined"){
return data;
}
data=data.replace(/\&custom_lt\;/g,"<");
data=data.replace(/\&custom_gt\;/g,">");
data=eval("\""+data+"\"");
return data;
},_chunkArgumentData:function(_70a,_70b){
var _70c=dojo.flash.obj.get();
var _70d=Math.ceil(_70a.length/1024);
for(var i=0;i<_70d;i++){
var _70f=i*1024;
var _710=i*1024+1024;
if(i==(_70d-1)){
_710=i*1024+_70a.length;
}
var _711=_70a.substring(_70f,_710);
_711=this._encodeData(_711);
_70c.CallFunction("<invoke name=\"chunkArgumentData\" "+"returntype=\"javascript\">"+"<arguments>"+"<string>"+_711+"</string>"+"<number>"+_70b+"</number>"+"</arguments>"+"</invoke>");
}
},_chunkReturnData:function(){
var _712=dojo.flash.obj.get();
var _713=_712.getReturnLength();
var _714=new Array();
for(var i=0;i<_713;i++){
var _716=_712.CallFunction("<invoke name=\"chunkReturnData\" "+"returntype=\"javascript\">"+"<arguments>"+"<number>"+i+"</number>"+"</arguments>"+"</invoke>");
if(_716=="\"\""||_716=="''"){
_716="";
}else{
_716=_716.substring(1,_716.length-1);
}
_714.push(_716);
}
var _717=_714.join("");
return _717;
},_execFlash:function(_718,_719){
var _71a=dojo.flash.obj.get();
_71a.startExec();
_71a.setNumberArguments(_719.length);
for(var i=0;i<_719.length;i++){
this._chunkArgumentData(_719[i],i);
}
_71a.exec(_718);
var _71c=this._chunkReturnData();
_71c=this._decodeData(_71c);
_71a.endExec();
return _71c;
}};
dojo.flash.Install=function(){
};
dojo.flash.Install.prototype={needed:function(){
if(dojo.flash.info.capable==false){
return true;
}
if(dojo.render.os.mac==true&&!dojo.flash.info.isVersionOrAbove(8,0,0)){
return true;
}
if(!dojo.flash.info.isVersionOrAbove(6,0,0)){
return true;
}
return false;
},install:function(){
dojo.flash.info.installing=true;
dojo.flash.installing();
if(dojo.flash.info.capable==false){
var _71d=new dojo.flash.Embed(false);
_71d.write(8);
}else{
if(dojo.flash.info.isVersionOrAbove(6,0,65)){
var _71d=new dojo.flash.Embed(false);
_71d.write(8,true);
_71d.setVisible(true);
_71d.center();
}else{
alert("This content requires a more recent version of the Macromedia "+" Flash Player.");
window.location.href="http://www.macromedia.com/go/getflashplayer";
}
}
},_onInstallStatus:function(msg){
if(msg=="Download.Complete"){
dojo.flash._initialize();
}else{
if(msg=="Download.Cancelled"){
alert("This content requires a more recent version of the Macromedia "+" Flash Player.");
window.location.href="http://www.macromedia.com/go/getflashplayer";
}else{
if(msg=="Download.Failed"){
alert("There was an error downloading the Flash Player update. "+"Please try again later, or visit macromedia.com to download "+"the latest version of the Flash plugin.");
}
}
}
}};
dojo.flash.info=new dojo.flash.Info();
dojo.provide("dojo.storage.browser");
dojo.provide("dojo.storage.browser.FlashStorageProvider");
dojo.require("dojo.storage");
dojo.require("dojo.flash");
dojo.require("dojo.json");
dojo.require("dojo.uri.*");
dojo.storage.browser.FlashStorageProvider=function(){
};
dojo.inherits(dojo.storage.browser.FlashStorageProvider,dojo.storage);
dojo.lang.extend(dojo.storage.browser.FlashStorageProvider,{namespace:"default",initialized:false,_available:null,_statusHandler:null,initialize:function(){
if(djConfig["disableFlashStorage"]==true){
return;
}
var _71f=function(){
dojo.storage._flashLoaded();
};
dojo.flash.addLoadedListener(_71f);
var _720=dojo.uri.dojoUri("Storage_version6.swf").toString();
var _721=dojo.uri.dojoUri("Storage_version8.swf").toString();
dojo.flash.setSwf({flash6:_720,flash8:_721,visible:false});
},isAvailable:function(){
if(djConfig["disableFlashStorage"]==true){
this._available=false;
}
return this._available;
},setNamespace:function(_722){
this.namespace=_722;
},put:function(key,_724,_725){
if(this.isValidKey(key)==false){
dojo.raise("Invalid key given: "+key);
}
this._statusHandler=_725;
if(dojo.lang.isString(_724)){
_724="string:"+_724;
}else{
_724=dojo.json.serialize(_724);
}
dojo.flash.comm.put(key,_724,this.namespace);
},get:function(key){
if(this.isValidKey(key)==false){
dojo.raise("Invalid key given: "+key);
}
var _727=dojo.flash.comm.get(key,this.namespace);
if(_727==""){
return null;
}
if(!dojo.lang.isUndefined(_727)&&_727!=null&&/^string:/.test(_727)){
_727=_727.substring("string:".length);
}else{
_727=dojo.json.evalJson(_727);
}
return _727;
},getKeys:function(){
var _728=dojo.flash.comm.getKeys(this.namespace);
if(_728==""){
return new Array();
}
_728=_728.split(",");
return _728;
},clear:function(){
dojo.flash.comm.clear(this.namespace);
},remove:function(key){
},isPermanent:function(){
return true;
},getMaximumSize:function(){
return dojo.storage.SIZE_NO_LIMIT;
},hasSettingsUI:function(){
return true;
},showSettingsUI:function(){
dojo.flash.comm.showSettings();
dojo.flash.obj.setVisible(true);
dojo.flash.obj.center();
},hideSettingsUI:function(){
dojo.flash.obj.setVisible(false);
if(dojo.storage.onHideSettingsUI!=null&&!dojo.lang.isUndefined(dojo.storage.onHideSettingsUI)){
dojo.storage.onHideSettingsUI.call(null);
}
},getType:function(){
return "dojo.storage.FlashStorageProvider";
},_flashLoaded:function(){
this.initialized=true;
dojo.storage.manager.loaded();
},_onStatus:function(_72a,key){
if(_72a==dojo.storage.PENDING){
dojo.flash.obj.center();
dojo.flash.obj.setVisible(true);
}else{
dojo.flash.obj.setVisible(false);
}
if(!dojo.lang.isUndefined(dojo.storage._statusHandler)&&dojo.storage._statusHandler!=null){
dojo.storage._statusHandler.call(null,_72a,key);
}
}});
dojo.storage.manager.register("dojo.storage.browser.FlashStorageProvider",new dojo.storage.browser.FlashStorageProvider());
dojo.storage.manager.initialize();
dojo.kwCompoundRequire({common:["dojo.storage"],browser:["dojo.storage.browser"],dashboard:["dojo.storage.dashboard"]});
dojo.provide("dojo.storage.*");
dojo.provide("dojo.undo.Manager");
dojo.require("dojo.lang");
dojo.undo.Manager=function(_72c){
this.clear();
this._parent=_72c;
};
dojo.lang.extend(dojo.undo.Manager,{_parent:null,_undoStack:null,_redoStack:null,_currentManager:null,canUndo:false,canRedo:false,isUndoing:false,isRedoing:false,onUndo:function(_72d,item){
},onRedo:function(_72f,item){
},onUndoAny:function(_731,item){
},onRedoAny:function(_733,item){
},_updateStatus:function(){
this.canUndo=this._undoStack.length>0;
this.canRedo=this._redoStack.length>0;
},clear:function(){
this._undoStack=[];
this._redoStack=[];
this._currentManager=this;
this.isUndoing=false;
this.isRedoing=false;
this._updateStatus();
},undo:function(){
if(!this.canUndo){
return false;
}
this.endAllTransactions();
this.isUndoing=true;
var top=this._undoStack.pop();
if(top instanceof dojo.undo.Manager){
top.undoAll();
}else{
top.undo();
}
if(top.redo){
this._redoStack.push(top);
}
this.isUndoing=false;
this._updateStatus();
this.onUndo(this,top);
if(!(top instanceof dojo.undo.Manager)){
this.getTop().onUndoAny(this,top);
}
return true;
},redo:function(){
if(!this.canRedo){
return false;
}
this.isRedoing=true;
var top=this._redoStack.pop();
if(top instanceof dojo.undo.Manager){
top.redoAll();
}else{
top.redo();
}
this._undoStack.push(top);
this.isRedoing=false;
this._updateStatus();
this.onRedo(this,top);
if(!(top instanceof dojo.undo.Manager)){
this.getTop().onRedoAny(this,top);
}
return true;
},undoAll:function(){
while(this._undoStack.length>0){
this.undo();
}
},redoAll:function(){
while(this._redoStack.length>0){
this.redo();
}
},push:function(undo,redo,_739){
if(!undo){
return;
}
if(this._currentManager==this){
this._undoStack.push({undo:undo,redo:redo,description:_739});
}else{
this._currentManager.push.apply(this._currentManager,arguments);
}
this._redoStack=[];
this._updateStatus();
},concat:function(_73a){
if(!_73a){
return;
}
if(this._currentManager==this){
for(var x=0;x<_73a._undoStack.length;x++){
this._undoStack.push(_73a._undoStack[x]);
}
this._redoStack=[];
this._updateStatus();
}else{
this._currentManager.concat.apply(this._currentManager,arguments);
}
},beginTransaction:function(_73c){
if(this._currentManager==this){
var mgr=new dojo.undo.Manager(this);
mgr.description=_73c?_73c:"";
this._undoStack.push(mgr);
this._currentManager=mgr;
return mgr;
}else{
this._currentManager=this._currentManager.beginTransaction.apply(this._currentManager,arguments);
}
},endTransaction:function(_73e){
if(this._currentManager==this){
if(this._parent){
this._parent._currentManager=this._parent;
if(this._undoStack.length==0||_73e){
var idx=dojo.lang.find(this._parent._undoStack,this);
if(idx>=0){
this._parent._undoStack.splice(idx,1);
if(_73e){
for(var x=0;x<this._undoStack.length;x++){
this._parent._undoStack.splice(idx++,0,this._undoStack[x]);
}
this._updateStatus();
}
}
}
return this._parent;
}
}else{
this._currentManager=this._currentManager.endTransaction.apply(this._currentManager,arguments);
}
},endAllTransactions:function(){
while(this._currentManager!=this){
this.endTransaction();
}
},getTop:function(){
if(this._parent){
return this._parent.getTop();
}else{
return this;
}
}});
dojo.require("dojo.undo.Manager");
dojo.provide("dojo.undo.*");
dojo.provide("dojo.crypto");
dojo.crypto.cipherModes={ECB:0,CBC:1,PCBC:2,CFB:3,OFB:4,CTR:5};
dojo.crypto.outputTypes={Base64:0,Hex:1,String:2,Raw:3};
dojo.require("dojo.crypto");
dojo.provide("dojo.crypto.MD5");
dojo.crypto.MD5=new function(){
var _741=8;
var mask=(1<<_741)-1;
function toWord(s){
var wa=[];
for(var i=0;i<s.length*_741;i+=_741){
wa[i>>5]|=(s.charCodeAt(i/_741)&mask)<<(i%32);
}
return wa;
}
function toString(wa){
var s=[];
for(var i=0;i<wa.length*32;i+=_741){
s.push(String.fromCharCode((wa[i>>5]>>>(i%32))&mask));
}
return s.join("");
}
function toHex(wa){
var h="0123456789abcdef";
var s=[];
for(var i=0;i<wa.length*4;i++){
s.push(h.charAt((wa[i>>2]>>((i%4)*8+4))&15)+h.charAt((wa[i>>2]>>((i%4)*8))&15));
}
return s.join("");
}
function toBase64(wa){
var p="=";
var tab="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
var s=[];
for(var i=0;i<wa.length*4;i+=3){
var t=(((wa[i>>2]>>8*(i%4))&255)<<16)|(((wa[i+1>>2]>>8*((i+1)%4))&255)<<8)|((wa[i+2>>2]>>8*((i+2)%4))&255);
for(var j=0;j<4;j++){
if(i*8+j*6>wa.length*32){
s.push(p);
}else{
s.push(tab.charAt((t>>6*(3-j))&63));
}
}
}
return s.join("");
}
function add(x,y){
var l=(x&65535)+(y&65535);
var m=(x>>16)+(y>>16)+(l>>16);
return (m<<16)|(l&65535);
}
function R(n,c){
return (n<<c)|(n>>>(32-c));
}
function C(q,a,b,x,s,t){
return add(R(add(add(a,q),add(x,t)),s),b);
}
function FF(a,b,c,d,x,s,t){
return C((b&c)|((~b)&d),a,b,x,s,t);
}
function GG(a,b,c,d,x,s,t){
return C((b&d)|(c&(~d)),a,b,x,s,t);
}
function HH(a,b,c,d,x,s,t){
return C(b^c^d,a,b,x,s,t);
}
function II(a,b,c,d,x,s,t){
return C(c^(b|(~d)),a,b,x,s,t);
}
function core(x,len){
x[len>>5]|=128<<((len)%32);
x[(((len+64)>>>9)<<4)+14]=len;
var a=1732584193;
var b=-271733879;
var c=-1732584194;
var d=271733878;
for(var i=0;i<x.length;i+=16){
var olda=a;
var oldb=b;
var oldc=c;
var oldd=d;
a=FF(a,b,c,d,x[i+0],7,-680876936);
d=FF(d,a,b,c,x[i+1],12,-389564586);
c=FF(c,d,a,b,x[i+2],17,606105819);
b=FF(b,c,d,a,x[i+3],22,-1044525330);
a=FF(a,b,c,d,x[i+4],7,-176418897);
d=FF(d,a,b,c,x[i+5],12,1200080426);
c=FF(c,d,a,b,x[i+6],17,-1473231341);
b=FF(b,c,d,a,x[i+7],22,-45705983);
a=FF(a,b,c,d,x[i+8],7,1770035416);
d=FF(d,a,b,c,x[i+9],12,-1958414417);
c=FF(c,d,a,b,x[i+10],17,-42063);
b=FF(b,c,d,a,x[i+11],22,-1990404162);
a=FF(a,b,c,d,x[i+12],7,1804603682);
d=FF(d,a,b,c,x[i+13],12,-40341101);
c=FF(c,d,a,b,x[i+14],17,-1502002290);
b=FF(b,c,d,a,x[i+15],22,1236535329);
a=GG(a,b,c,d,x[i+1],5,-165796510);
d=GG(d,a,b,c,x[i+6],9,-1069501632);
c=GG(c,d,a,b,x[i+11],14,643717713);
b=GG(b,c,d,a,x[i+0],20,-373897302);
a=GG(a,b,c,d,x[i+5],5,-701558691);
d=GG(d,a,b,c,x[i+10],9,38016083);
c=GG(c,d,a,b,x[i+15],14,-660478335);
b=GG(b,c,d,a,x[i+4],20,-405537848);
a=GG(a,b,c,d,x[i+9],5,568446438);
d=GG(d,a,b,c,x[i+14],9,-1019803690);
c=GG(c,d,a,b,x[i+3],14,-187363961);
b=GG(b,c,d,a,x[i+8],20,1163531501);
a=GG(a,b,c,d,x[i+13],5,-1444681467);
d=GG(d,a,b,c,x[i+2],9,-51403784);
c=GG(c,d,a,b,x[i+7],14,1735328473);
b=GG(b,c,d,a,x[i+12],20,-1926607734);
a=HH(a,b,c,d,x[i+5],4,-378558);
d=HH(d,a,b,c,x[i+8],11,-2022574463);
c=HH(c,d,a,b,x[i+11],16,1839030562);
b=HH(b,c,d,a,x[i+14],23,-35309556);
a=HH(a,b,c,d,x[i+1],4,-1530992060);
d=HH(d,a,b,c,x[i+4],11,1272893353);
c=HH(c,d,a,b,x[i+7],16,-155497632);
b=HH(b,c,d,a,x[i+10],23,-1094730640);
a=HH(a,b,c,d,x[i+13],4,681279174);
d=HH(d,a,b,c,x[i+0],11,-358537222);
c=HH(c,d,a,b,x[i+3],16,-722521979);
b=HH(b,c,d,a,x[i+6],23,76029189);
a=HH(a,b,c,d,x[i+9],4,-640364487);
d=HH(d,a,b,c,x[i+12],11,-421815835);
c=HH(c,d,a,b,x[i+15],16,530742520);
b=HH(b,c,d,a,x[i+2],23,-995338651);
a=II(a,b,c,d,x[i+0],6,-198630844);
d=II(d,a,b,c,x[i+7],10,1126891415);
c=II(c,d,a,b,x[i+14],15,-1416354905);
b=II(b,c,d,a,x[i+5],21,-57434055);
a=II(a,b,c,d,x[i+12],6,1700485571);
d=II(d,a,b,c,x[i+3],10,-1894986606);
c=II(c,d,a,b,x[i+10],15,-1051523);
b=II(b,c,d,a,x[i+1],21,-2054922799);
a=II(a,b,c,d,x[i+8],6,1873313359);
d=II(d,a,b,c,x[i+15],10,-30611744);
c=II(c,d,a,b,x[i+6],15,-1560198380);
b=II(b,c,d,a,x[i+13],21,1309151649);
a=II(a,b,c,d,x[i+4],6,-145523070);
d=II(d,a,b,c,x[i+11],10,-1120210379);
c=II(c,d,a,b,x[i+2],15,718787259);
b=II(b,c,d,a,x[i+9],21,-343485551);
a=add(a,olda);
b=add(b,oldb);
c=add(c,oldc);
d=add(d,oldd);
}
return [a,b,c,d];
}
function hmac(data,key){
var wa=toWord(key);
if(wa.length>16){
wa=core(wa,key.length*_741);
}
var l=[],r=[];
for(var i=0;i<16;i++){
l[i]=wa[i]^909522486;
r[i]=wa[i]^1549556828;
}
var h=core(l.concat(toWord(data)),512+data.length*_741);
return core(r.concat(h),640);
}
this.compute=function(data,_78e){
var out=_78e||dojo.crypto.outputTypes.Base64;
switch(out){
case dojo.crypto.outputTypes.Hex:
return toHex(core(toWord(data),data.length*_741));
case dojo.crypto.outputTypes.String:
return toString(core(toWord(data),data.length*_741));
default:
return toBase64(core(toWord(data),data.length*_741));
}
};
this.getHMAC=function(data,key,_792){
var out=_792||dojo.crypto.outputTypes.Base64;
switch(out){
case dojo.crypto.outputTypes.Hex:
return toHex(hmac(data,key));
case dojo.crypto.outputTypes.String:
return toString(hmac(data,key));
default:
return toBase64(hmac(data,key));
}
};
}();
dojo.kwCompoundRequire({common:["dojo.crypto","dojo.crypto.MD5"]});
dojo.provide("dojo.crypto.*");
dojo.provide("dojo.collections.Collections");
dojo.collections={Collections:true};
dojo.collections.DictionaryEntry=function(k,v){
this.key=k;
this.value=v;
this.valueOf=function(){
return this.value;
};
this.toString=function(){
return String(this.value);
};
};
dojo.collections.Iterator=function(arr){
var a=arr;
var _798=0;
this.element=a[_798]||null;
this.atEnd=function(){
return (_798>=a.length);
};
this.get=function(){
if(this.atEnd()){
return null;
}
this.element=a[_798++];
return this.element;
};
this.map=function(fn,_79a){
var s=_79a||dj_global;
if(Array.map){
return Array.map(a,fn,s);
}else{
var arr=[];
for(var i=0;i<a.length;i++){
arr.push(fn.call(s,a[i]));
}
return arr;
}
};
this.reset=function(){
_798=0;
this.element=a[_798];
};
};
dojo.collections.DictionaryIterator=function(obj){
var a=[];
var _7a0={};
for(var p in obj){
if(!_7a0[p]){
a.push(obj[p]);
}
}
var _7a2=0;
this.element=a[_7a2]||null;
this.atEnd=function(){
return (_7a2>=a.length);
};
this.get=function(){
if(this.atEnd()){
return null;
}
this.element=a[_7a2++];
return this.element;
};
this.map=function(fn,_7a4){
var s=_7a4||dj_global;
if(Array.map){
return Array.map(a,fn,s);
}else{
var arr=[];
for(var i=0;i<a.length;i++){
arr.push(fn.call(s,a[i]));
}
return arr;
}
};
this.reset=function(){
_7a2=0;
this.element=a[_7a2];
};
};
dojo.provide("dojo.collections.ArrayList");
dojo.require("dojo.collections.Collections");
dojo.collections.ArrayList=function(arr){
var _7a9=[];
if(arr){
_7a9=_7a9.concat(arr);
}
this.count=_7a9.length;
this.add=function(obj){
_7a9.push(obj);
this.count=_7a9.length;
};
this.addRange=function(a){
if(a.getIterator){
var e=a.getIterator();
while(!e.atEnd()){
this.add(e.get());
}
this.count=_7a9.length;
}else{
for(var i=0;i<a.length;i++){
_7a9.push(a[i]);
}
this.count=_7a9.length;
}
};
this.clear=function(){
_7a9.splice(0,_7a9.length);
this.count=0;
};
this.clone=function(){
return new dojo.collections.ArrayList(_7a9);
};
this.contains=function(obj){
for(var i=0;i<_7a9.length;i++){
if(_7a9[i]==obj){
return true;
}
}
return false;
};
this.forEach=function(fn,_7b1){
var s=_7b1||dj_global;
if(Array.forEach){
Array.forEach(_7a9,fn,s);
}else{
for(var i=0;i<_7a9.length;i++){
fn.call(s,_7a9[i],i,_7a9);
}
}
};
this.getIterator=function(){
return new dojo.collections.Iterator(_7a9);
};
this.indexOf=function(obj){
for(var i=0;i<_7a9.length;i++){
if(_7a9[i]==obj){
return i;
}
}
return -1;
};
this.insert=function(i,obj){
_7a9.splice(i,0,obj);
this.count=_7a9.length;
};
this.item=function(i){
return _7a9[i];
};
this.remove=function(obj){
var i=this.indexOf(obj);
if(i>=0){
_7a9.splice(i,1);
}
this.count=_7a9.length;
};
this.removeAt=function(i){
_7a9.splice(i,1);
this.count=_7a9.length;
};
this.reverse=function(){
_7a9.reverse();
};
this.sort=function(fn){
if(fn){
_7a9.sort(fn);
}else{
_7a9.sort();
}
};
this.setByIndex=function(i,obj){
_7a9[i]=obj;
this.count=_7a9.length;
};
this.toArray=function(){
return [].concat(_7a9);
};
this.toString=function(_7bf){
return _7a9.join((_7bf||","));
};
};
dojo.provide("dojo.collections.Queue");
dojo.require("dojo.collections.Collections");
dojo.collections.Queue=function(arr){
var q=[];
if(arr){
q=q.concat(arr);
}
this.count=q.length;
this.clear=function(){
q=[];
this.count=q.length;
};
this.clone=function(){
return new dojo.collections.Queue(q);
};
this.contains=function(o){
for(var i=0;i<q.length;i++){
if(q[i]==o){
return true;
}
}
return false;
};
this.copyTo=function(arr,i){
arr.splice(i,0,q);
};
this.dequeue=function(){
var r=q.shift();
this.count=q.length;
return r;
};
this.enqueue=function(o){
this.count=q.push(o);
};
this.forEach=function(fn,_7c9){
var s=_7c9||dj_global;
if(Array.forEach){
Array.forEach(q,fn,s);
}else{
for(var i=0;i<q.length;i++){
fn.call(s,q[i],i,q);
}
}
};
this.getIterator=function(){
return new dojo.collections.Iterator(q);
};
this.peek=function(){
return q[0];
};
this.toArray=function(){
return [].concat(q);
};
};
dojo.provide("dojo.collections.Stack");
dojo.require("dojo.collections.Collections");
dojo.collections.Stack=function(arr){
var q=[];
if(arr){
q=q.concat(arr);
}
this.count=q.length;
this.clear=function(){
q=[];
this.count=q.length;
};
this.clone=function(){
return new dojo.collections.Stack(q);
};
this.contains=function(o){
for(var i=0;i<q.length;i++){
if(q[i]==o){
return true;
}
}
return false;
};
this.copyTo=function(arr,i){
arr.splice(i,0,q);
};
this.forEach=function(fn,_7d3){
var s=_7d3||dj_global;
if(Array.forEach){
Array.forEach(q,fn,s);
}else{
for(var i=0;i<q.length;i++){
fn.call(s,q[i],i,q);
}
}
};
this.getIterator=function(){
return new dojo.collections.Iterator(q);
};
this.peek=function(){
return q[(q.length-1)];
};
this.pop=function(){
var r=q.pop();
this.count=q.length;
return r;
};
this.push=function(o){
this.count=q.push(o);
};
this.toArray=function(){
return [].concat(q);
};
};
dojo.require("dojo.lang");
dojo.provide("dojo.dnd.DragSource");
dojo.provide("dojo.dnd.DropTarget");
dojo.provide("dojo.dnd.DragObject");
dojo.provide("dojo.dnd.DragAndDrop");
dojo.dnd.DragSource=function(){
var dm=dojo.dnd.dragManager;
if(dm["registerDragSource"]){
dm.registerDragSource(this);
}
};
dojo.lang.extend(dojo.dnd.DragSource,{type:"",onDragEnd:function(){
},onDragStart:function(){
},onSelected:function(){
},unregister:function(){
dojo.dnd.dragManager.unregisterDragSource(this);
},reregister:function(){
dojo.dnd.dragManager.registerDragSource(this);
}});
dojo.dnd.DragObject=function(){
var dm=dojo.dnd.dragManager;
if(dm["registerDragObject"]){
dm.registerDragObject(this);
}
};
dojo.lang.extend(dojo.dnd.DragObject,{type:"",onDragStart:function(){
},onDragMove:function(){
},onDragOver:function(){
},onDragOut:function(){
},onDragEnd:function(){
},onDragLeave:this.onDragOut,onDragEnter:this.onDragOver,ondragout:this.onDragOut,ondragover:this.onDragOver});
dojo.dnd.DropTarget=function(){
if(this.constructor==dojo.dnd.DropTarget){
return;
}
this.acceptedTypes=[];
dojo.dnd.dragManager.registerDropTarget(this);
};
dojo.lang.extend(dojo.dnd.DropTarget,{acceptsType:function(type){
if(!dojo.lang.inArray(this.acceptedTypes,"*")){
if(!dojo.lang.inArray(this.acceptedTypes,type)){
return false;
}
}
return true;
},accepts:function(_7db){
if(!dojo.lang.inArray(this.acceptedTypes,"*")){
for(var i=0;i<_7db.length;i++){
if(!dojo.lang.inArray(this.acceptedTypes,_7db[i].type)){
return false;
}
}
}
return true;
},onDragOver:function(){
},onDragOut:function(){
},onDragMove:function(){
},onDropStart:function(){
},onDrop:function(){
},onDropEnd:function(){
}});
dojo.dnd.DragEvent=function(){
this.dragSource=null;
this.dragObject=null;
this.target=null;
this.eventStatus="success";
};
dojo.dnd.DragManager=function(){
};
dojo.lang.extend(dojo.dnd.DragManager,{selectedSources:[],dragObjects:[],dragSources:[],registerDragSource:function(){
},dropTargets:[],registerDropTarget:function(){
},lastDragTarget:null,currentDragTarget:null,onKeyDown:function(){
},onMouseOut:function(){
},onMouseMove:function(){
},onMouseUp:function(){
}});
dojo.provide("dojo.dnd.HtmlDragManager");
dojo.require("dojo.dnd.DragAndDrop");
dojo.require("dojo.event.*");
dojo.require("dojo.lang.array");
dojo.require("dojo.html");
dojo.require("dojo.style");
dojo.dnd.HtmlDragManager=function(){
};
dojo.inherits(dojo.dnd.HtmlDragManager,dojo.dnd.DragManager);
dojo.lang.extend(dojo.dnd.HtmlDragManager,{disabled:false,nestedTargets:false,mouseDownTimer:null,dsCounter:0,dsPrefix:"dojoDragSource",dropTargetDimensions:[],currentDropTarget:null,previousDropTarget:null,_dragTriggered:false,selectedSources:[],dragObjects:[],currentX:null,currentY:null,lastX:null,lastY:null,mouseDownX:null,mouseDownY:null,threshold:7,dropAcceptable:false,cancelEvent:function(e){
e.stopPropagation();
e.preventDefault();
},registerDragSource:function(ds){
if(ds["domNode"]){
var dp=this.dsPrefix;
var _7e0=dp+"Idx_"+(this.dsCounter++);
ds.dragSourceId=_7e0;
this.dragSources[_7e0]=ds;
ds.domNode.setAttribute(dp,_7e0);
if(dojo.render.html.ie){
dojo.event.connect(ds.domNode,"ondragstart",this.cancelEvent);
}
}
},unregisterDragSource:function(ds){
if(ds["domNode"]){
var dp=this.dsPrefix;
var _7e3=ds.dragSourceId;
delete ds.dragSourceId;
delete this.dragSources[_7e3];
ds.domNode.setAttribute(dp,null);
}
if(dojo.render.html.ie){
dojo.event.disconnect(ds.domNode,"ondragstart",this.cancelEvent);
}
},registerDropTarget:function(dt){
this.dropTargets.push(dt);
},unregisterDropTarget:function(dt){
var _7e6=dojo.lang.find(this.dropTargets,dt,true);
if(_7e6>=0){
this.dropTargets.splice(_7e6,1);
}
},getDragSource:function(e){
var tn=e.target;
if(tn===document.body){
return;
}
var ta=dojo.html.getAttribute(tn,this.dsPrefix);
while((!ta)&&(tn)){
tn=tn.parentNode;
if((!tn)||(tn===document.body)){
return;
}
ta=dojo.html.getAttribute(tn,this.dsPrefix);
}
return this.dragSources[ta];
},onKeyDown:function(e){
},onMouseDown:function(e){
if(this.disabled){
return;
}
if(dojo.render.html.ie){
if(e.button!=1){
return;
}
}else{
if(e.which!=1){
return;
}
}
var _7ec=e.target.nodeType==dojo.dom.TEXT_NODE?e.target.parentNode:e.target;
if(dojo.html.isTag(_7ec,"button","textarea","input","select","option")){
return;
}
var ds=this.getDragSource(e);
if(!ds){
return;
}
if(!dojo.lang.inArray(this.selectedSources,ds)){
this.selectedSources.push(ds);
ds.onSelected();
}
this.mouseDownX=e.pageX;
this.mouseDownY=e.pageY;
e.preventDefault();
dojo.event.connect(document,"onmousemove",this,"onMouseMove");
},onMouseUp:function(e,_7ef){
if(this.selectedSources.length==0){
return;
}
this.mouseDownX=null;
this.mouseDownY=null;
this._dragTriggered=false;
e.dragSource=this.dragSource;
if((!e.shiftKey)&&(!e.ctrlKey)){
if(this.currentDropTarget){
this.currentDropTarget.onDropStart();
}
dojo.lang.forEach(this.dragObjects,function(_7f0){
var ret=null;
if(!_7f0){
return;
}
if(this.currentDropTarget){
e.dragObject=_7f0;
var ce=this.currentDropTarget.domNode.childNodes;
if(ce.length>0){
e.dropTarget=ce[0];
while(e.dropTarget==_7f0.domNode){
e.dropTarget=e.dropTarget.nextSibling;
}
}else{
e.dropTarget=this.currentDropTarget.domNode;
}
if(this.dropAcceptable){
ret=this.currentDropTarget.onDrop(e);
}else{
this.currentDropTarget.onDragOut(e);
}
}
e.dragStatus=this.dropAcceptable&&ret?"dropSuccess":"dropFailure";
dojo.lang.delayThese([function(){
try{
_7f0.dragSource.onDragEnd(e);
}
catch(err){
var _7f3={};
for(var i in e){
if(i=="type"){
_7f3.type="mouseup";
continue;
}
_7f3[i]=e[i];
}
_7f0.dragSource.onDragEnd(_7f3);
}
},function(){
_7f0.onDragEnd(e);
}]);
},this);
this.selectedSources=[];
this.dragObjects=[];
this.dragSource=null;
if(this.currentDropTarget){
this.currentDropTarget.onDropEnd();
}
}
dojo.event.disconnect(document,"onmousemove",this,"onMouseMove");
this.currentDropTarget=null;
},onScroll:function(){
for(var i=0;i<this.dragObjects.length;i++){
if(this.dragObjects[i].updateDragOffset){
this.dragObjects[i].updateDragOffset();
}
}
this.cacheTargetLocations();
},_dragStartDistance:function(x,y){
if((!this.mouseDownX)||(!this.mouseDownX)){
return;
}
var dx=Math.abs(x-this.mouseDownX);
var dx2=dx*dx;
var dy=Math.abs(y-this.mouseDownY);
var dy2=dy*dy;
return parseInt(Math.sqrt(dx2+dy2),10);
},cacheTargetLocations:function(){
this.dropTargetDimensions=[];
dojo.lang.forEach(this.dropTargets,function(_7fc){
var tn=_7fc.domNode;
if(!tn){
return;
}
var ttx=dojo.style.getAbsoluteX(tn,true);
var tty=dojo.style.getAbsoluteY(tn,true);
this.dropTargetDimensions.push([[ttx,tty],[ttx+dojo.style.getInnerWidth(tn),tty+dojo.style.getInnerHeight(tn)],_7fc]);
},this);
},onMouseMove:function(e){
if((dojo.render.html.ie)&&(e.button!=1)){
this.currentDropTarget=null;
this.onMouseUp(e,true);
return;
}
if((this.selectedSources.length)&&(!this.dragObjects.length)){
var dx;
var dy;
if(!this._dragTriggered){
this._dragTriggered=(this._dragStartDistance(e.pageX,e.pageY)>this.threshold);
if(!this._dragTriggered){
return;
}
dx=e.pageX-this.mouseDownX;
dy=e.pageY-this.mouseDownY;
}
this.dragSource=this.selectedSources[0];
dojo.lang.forEach(this.selectedSources,function(_803){
if(!_803){
return;
}
var tdo=_803.onDragStart(e);
if(tdo){
tdo.onDragStart(e);
tdo.dragOffset.top+=dy;
tdo.dragOffset.left+=dx;
tdo.dragSource=_803;
this.dragObjects.push(tdo);
}
},this);
this.previousDropTarget=null;
this.cacheTargetLocations();
}
dojo.lang.forEach(this.dragObjects,function(_805){
if(_805){
_805.onDragMove(e);
}
});
if(this.currentDropTarget){
var c=dojo.style.toCoordinateArray(this.currentDropTarget.domNode,true);
var dtp=[[c[0],c[1]],[c[0]+c[2],c[1]+c[3]]];
}
if((!this.nestedTargets)&&(dtp)&&(this.isInsideBox(e,dtp))){
if(this.dropAcceptable){
this.currentDropTarget.onDragMove(e,this.dragObjects);
}
}else{
var _808=this.findBestTarget(e);
if(_808.target===null){
if(this.currentDropTarget){
this.currentDropTarget.onDragOut(e);
this.previousDropTarget=this.currentDropTarget;
this.currentDropTarget=null;
}
this.dropAcceptable=false;
return;
}
if(this.currentDropTarget!==_808.target){
if(this.currentDropTarget){
this.previousDropTarget=this.currentDropTarget;
this.currentDropTarget.onDragOut(e);
}
this.currentDropTarget=_808.target;
e.dragObjects=this.dragObjects;
this.dropAcceptable=this.currentDropTarget.onDragOver(e);
}else{
if(this.dropAcceptable){
this.currentDropTarget.onDragMove(e,this.dragObjects);
}
}
}
},findBestTarget:function(e){
var _80a=this;
var _80b=new Object();
_80b.target=null;
_80b.points=null;
dojo.lang.every(this.dropTargetDimensions,function(_80c){
if(!_80a.isInsideBox(e,_80c)){
return true;
}
_80b.target=_80c[2];
_80b.points=_80c;
return Boolean(_80a.nestedTargets);
});
return _80b;
},isInsideBox:function(e,_80e){
if((e.pageX>_80e[0][0])&&(e.pageX<_80e[1][0])&&(e.pageY>_80e[0][1])&&(e.pageY<_80e[1][1])){
return true;
}
return false;
},onMouseOver:function(e){
},onMouseOut:function(e){
}});
dojo.dnd.dragManager=new dojo.dnd.HtmlDragManager();
(function(){
var d=document;
var dm=dojo.dnd.dragManager;
dojo.event.connect(d,"onkeydown",dm,"onKeyDown");
dojo.event.connect(d,"onmouseover",dm,"onMouseOver");
dojo.event.connect(d,"onmouseout",dm,"onMouseOut");
dojo.event.connect(d,"onmousedown",dm,"onMouseDown");
dojo.event.connect(d,"onmouseup",dm,"onMouseUp");
dojo.event.connect(window,"onscroll",dm,"onScroll");
})();
dojo.require("dojo.html");
dojo.provide("dojo.html.extras");
dojo.require("dojo.string.extras");
dojo.html.gravity=function(node,e){
node=dojo.byId(node);
var _815=dojo.html.getCursorPosition(e);
with(dojo.html){
var _816=getAbsoluteX(node,true)+(getInnerWidth(node)/2);
var _817=getAbsoluteY(node,true)+(getInnerHeight(node)/2);
}
with(dojo.html.gravity){
return ((_815.x<_816?WEST:EAST)|(_815.y<_817?NORTH:SOUTH));
}
};
dojo.html.gravity.NORTH=1;
dojo.html.gravity.SOUTH=1<<1;
dojo.html.gravity.EAST=1<<2;
dojo.html.gravity.WEST=1<<3;
dojo.html.renderedTextContent=function(node){
node=dojo.byId(node);
var _819="";
if(node==null){
return _819;
}
for(var i=0;i<node.childNodes.length;i++){
switch(node.childNodes[i].nodeType){
case 1:
case 5:
var _81b="unknown";
try{
_81b=dojo.style.getStyle(node.childNodes[i],"display");
}
catch(E){
}
switch(_81b){
case "block":
case "list-item":
case "run-in":
case "table":
case "table-row-group":
case "table-header-group":
case "table-footer-group":
case "table-row":
case "table-column-group":
case "table-column":
case "table-cell":
case "table-caption":
_819+="\n";
_819+=dojo.html.renderedTextContent(node.childNodes[i]);
_819+="\n";
break;
case "none":
break;
default:
if(node.childNodes[i].tagName&&node.childNodes[i].tagName.toLowerCase()=="br"){
_819+="\n";
}else{
_819+=dojo.html.renderedTextContent(node.childNodes[i]);
}
break;
}
break;
case 3:
case 2:
case 4:
var text=node.childNodes[i].nodeValue;
var _81d="unknown";
try{
_81d=dojo.style.getStyle(node,"text-transform");
}
catch(E){
}
switch(_81d){
case "capitalize":
text=dojo.string.capitalize(text);
break;
case "uppercase":
text=text.toUpperCase();
break;
case "lowercase":
text=text.toLowerCase();
break;
default:
break;
}
switch(_81d){
case "nowrap":
break;
case "pre-wrap":
break;
case "pre-line":
break;
case "pre":
break;
default:
text=text.replace(/\s+/," ");
if(/\s$/.test(_819)){
text.replace(/^\s/,"");
}
break;
}
_819+=text;
break;
default:
break;
}
}
return _819;
};
dojo.html.createNodesFromText=function(txt,trim){
if(trim){
txt=dojo.string.trim(txt);
}
var tn=document.createElement("div");
tn.style.visibility="hidden";
document.body.appendChild(tn);
var _821="none";
if((/^<t[dh][\s\r\n>]/i).test(dojo.string.trimStart(txt))){
txt="<table><tbody><tr>"+txt+"</tr></tbody></table>";
_821="cell";
}else{
if((/^<tr[\s\r\n>]/i).test(dojo.string.trimStart(txt))){
txt="<table><tbody>"+txt+"</tbody></table>";
_821="row";
}else{
if((/^<(thead|tbody|tfoot)[\s\r\n>]/i).test(dojo.string.trimStart(txt))){
txt="<table>"+txt+"</table>";
_821="section";
}
}
}
tn.innerHTML=txt;
if(tn["normalize"]){
tn.normalize();
}
var _822=null;
switch(_821){
case "cell":
_822=tn.getElementsByTagName("tr")[0];
break;
case "row":
_822=tn.getElementsByTagName("tbody")[0];
break;
case "section":
_822=tn.getElementsByTagName("table")[0];
break;
default:
_822=tn;
break;
}
var _823=[];
for(var x=0;x<_822.childNodes.length;x++){
_823.push(_822.childNodes[x].cloneNode(true));
}
tn.style.display="none";
document.body.removeChild(tn);
return _823;
};
dojo.html.placeOnScreen=function(node,_826,_827,_828,_829){
if(dojo.lang.isArray(_826)){
_829=_828;
_828=_827;
_827=_826[1];
_826=_826[0];
}
if(!isNaN(_828)){
_828=[Number(_828),Number(_828)];
}else{
if(!dojo.lang.isArray(_828)){
_828=[0,0];
}
}
var _82a=dojo.html.getScrollOffset();
var view=dojo.html.getViewportSize();
node=dojo.byId(node);
var w=node.offsetWidth+_828[0];
var h=node.offsetHeight+_828[1];
if(_829){
_826-=_82a.x;
_827-=_82a.y;
}
var x=_826+w;
if(x>view.w){
x=view.w-w;
}else{
x=_826;
}
x=Math.max(_828[0],x)+_82a.x;
var y=_827+h;
if(y>view.h){
y=view.h-h;
}else{
y=_827;
}
y=Math.max(_828[1],y)+_82a.y;
node.style.left=x+"px";
node.style.top=y+"px";
var ret=[x,y];
ret.x=x;
ret.y=y;
return ret;
};
dojo.html.placeOnScreenPoint=function(node,_832,_833,_834,_835){
if(dojo.lang.isArray(_832)){
_835=_834;
_834=_833;
_833=_832[1];
_832=_832[0];
}
if(!isNaN(_834)){
_834=[Number(_834),Number(_834)];
}else{
if(!dojo.lang.isArray(_834)){
_834=[0,0];
}
}
var _836=dojo.html.getScrollOffset();
var view=dojo.html.getViewportSize();
node=dojo.byId(node);
var _838=node.style.display;
node.style.display="";
var w=dojo.style.getInnerWidth(node);
var h=dojo.style.getInnerHeight(node);
node.style.display=_838;
if(_835){
_832-=_836.x;
_833-=_836.y;
}
var x=-1,y=-1;
if((_832+_834[0])+w<=view.w&&(_833+_834[1])+h<=view.h){
x=(_832+_834[0]);
y=(_833+_834[1]);
}
if((x<0||y<0)&&(_832-_834[0])<=view.w&&(_833+_834[1])+h<=view.h){
x=(_832-_834[0])-w;
y=(_833+_834[1]);
}
if((x<0||y<0)&&(_832+_834[0])+w<=view.w&&(_833-_834[1])<=view.h){
x=(_832+_834[0]);
y=(_833-_834[1])-h;
}
if((x<0||y<0)&&(_832-_834[0])<=view.w&&(_833-_834[1])<=view.h){
x=(_832-_834[0])-w;
y=(_833-_834[1])-h;
}
if(x<0||y<0||(x+w>view.w)||(y+h>view.h)){
return dojo.html.placeOnScreen(node,_832,_833,_834,_835);
}
x+=_836.x;
y+=_836.y;
node.style.left=x+"px";
node.style.top=y+"px";
var ret=[x,y];
ret.x=x;
ret.y=y;
return ret;
};
dojo.html.BackgroundIframe=function(node){
if(dojo.render.html.ie55||dojo.render.html.ie60){
var html="<iframe "+"style='position: absolute; left: 0px; top: 0px; width: 100%; height: 100%;"+"z-index: -1; filter:Alpha(Opacity=\"0\");' "+">";
this.iframe=document.createElement(html);
if(node){
node.appendChild(this.iframe);
this.domNode=node;
}else{
document.body.appendChild(this.iframe);
this.iframe.style.display="none";
}
}
};
dojo.lang.extend(dojo.html.BackgroundIframe,{iframe:null,onResized:function(){
if(this.iframe&&this.domNode&&this.domNode.parentElement){
var w=dojo.style.getOuterWidth(this.domNode);
var h=dojo.style.getOuterHeight(this.domNode);
if(w==0||h==0){
dojo.lang.setTimeout(this,this.onResized,50);
return;
}
var s=this.iframe.style;
s.width=w+"px";
s.height=h+"px";
}
},size:function(node){
if(!this.iframe){
return;
}
var _843=dojo.style.toCoordinateArray(node,true);
var s=this.iframe.style;
s.width=_843.w+"px";
s.height=_843.h+"px";
s.left=_843.x+"px";
s.top=_843.y+"px";
},setZIndex:function(node){
if(!this.iframe){
return;
}
if(dojo.dom.isNode(node)){
this.iframe.style.zIndex=dojo.html.getStyle(node,"z-index")-1;
}else{
if(!isNaN(node)){
this.iframe.style.zIndex=node;
}
}
},show:function(){
if(!this.iframe){
return;
}
this.iframe.style.display="block";
},hide:function(){
if(!this.ie){
return;
}
var s=this.iframe.style;
s.display="none";
},remove:function(){
dojo.dom.removeNode(this.iframe);
}});
dojo.provide("dojo.dnd.HtmlDragAndDrop");
dojo.provide("dojo.dnd.HtmlDragSource");
dojo.provide("dojo.dnd.HtmlDropTarget");
dojo.provide("dojo.dnd.HtmlDragObject");
dojo.require("dojo.dnd.HtmlDragManager");
dojo.require("dojo.dnd.DragAndDrop");
dojo.require("dojo.dom");
dojo.require("dojo.style");
dojo.require("dojo.html");
dojo.require("dojo.html.extras");
dojo.require("dojo.lang.extras");
dojo.require("dojo.lfx.*");
dojo.require("dojo.event");
dojo.dnd.HtmlDragSource=function(node,type){
node=dojo.byId(node);
this.dragObjects=[];
this.constrainToContainer=false;
if(node){
this.domNode=node;
this.dragObject=node;
dojo.dnd.DragSource.call(this);
this.type=(type)||(this.domNode.nodeName.toLowerCase());
}
};
dojo.inherits(dojo.dnd.HtmlDragSource,dojo.dnd.DragSource);
dojo.lang.extend(dojo.dnd.HtmlDragSource,{dragClass:"",onDragStart:function(){
var _849=new dojo.dnd.HtmlDragObject(this.dragObject,this.type);
if(this.dragClass){
_849.dragClass=this.dragClass;
}
if(this.constrainToContainer){
_849.constrainTo(this.constrainingContainer||this.domNode.parentNode);
}
return _849;
},setDragHandle:function(node){
node=dojo.byId(node);
dojo.dnd.dragManager.unregisterDragSource(this);
this.domNode=node;
dojo.dnd.dragManager.registerDragSource(this);
},setDragTarget:function(node){
this.dragObject=node;
},constrainTo:function(_84c){
this.constrainToContainer=true;
if(_84c){
this.constrainingContainer=_84c;
}
},onSelected:function(){
for(var i=0;i<this.dragObjects.length;i++){
dojo.dnd.dragManager.selectedSources.push(new dojo.dnd.HtmlDragSource(this.dragObjects[i]));
}
},addDragObjects:function(el){
for(var i=0;i<arguments.length;i++){
this.dragObjects.push(arguments[i]);
}
}});
dojo.dnd.HtmlDragObject=function(node,type){
this.domNode=dojo.byId(node);
this.type=type;
this.constrainToContainer=false;
this.dragSource=null;
};
dojo.inherits(dojo.dnd.HtmlDragObject,dojo.dnd.DragObject);
dojo.lang.extend(dojo.dnd.HtmlDragObject,{dragClass:"",opacity:0.5,createIframe:true,disableX:false,disableY:false,createDragNode:function(){
var node=this.domNode.cloneNode(true);
if(this.dragClass){
dojo.html.addClass(node,this.dragClass);
}
if(this.opacity<1){
dojo.style.setOpacity(node,this.opacity);
}
if(node.tagName.toLowerCase()=="tr"){
var doc=this.domNode.ownerDocument;
var _854=doc.createElement("table");
var _855=doc.createElement("tbody");
_855.appendChild(node);
_854.appendChild(_855);
var _856=this.domNode.childNodes;
var _857=node.childNodes;
for(var i=0;i<_856.length;i++){
if((_857[i])&&(_857[i].style)){
_857[i].style.width=dojo.style.getContentWidth(_856[i])+"px";
}
}
node=_854;
}
if((dojo.render.html.ie55||dojo.render.html.ie60)&&this.createIframe){
with(node.style){
top="0px";
left="0px";
}
var _859=document.createElement("div");
_859.appendChild(node);
this.bgIframe=new dojo.html.BackgroundIframe(_859);
_859.appendChild(this.bgIframe.iframe);
node=_859;
}
node.style.zIndex=999;
return node;
},onDragStart:function(e){
dojo.html.clearSelection();
this.scrollOffset=dojo.html.getScrollOffset();
this.dragStartPosition=dojo.style.getAbsolutePosition(this.domNode,true);
this.dragOffset={y:this.dragStartPosition.y-e.pageY,x:this.dragStartPosition.x-e.pageX};
this.dragClone=this.createDragNode();
this.containingBlockPosition=this.domNode.offsetParent?dojo.style.getAbsolutePosition(this.domNode.offsetParent):{x:0,y:0};
if(this.constrainToContainer){
this.constraints=this.getConstraints();
}
with(this.dragClone.style){
position="absolute";
top=this.dragOffset.y+e.pageY+"px";
left=this.dragOffset.x+e.pageX+"px";
}
document.body.appendChild(this.dragClone);
dojo.event.topic.publish("dragStart",{source:this});
},getConstraints:function(){
if(this.constrainingContainer.nodeName.toLowerCase()=="body"){
var _85b=dojo.html.getViewportWidth();
var _85c=dojo.html.getViewportHeight();
var x=0;
var y=0;
}else{
_85b=dojo.style.getContentWidth(this.constrainingContainer);
_85c=dojo.style.getContentHeight(this.constrainingContainer);
x=this.containingBlockPosition.x+dojo.style.getPixelValue(this.constrainingContainer,"padding-left",true)+dojo.style.getBorderExtent(this.constrainingContainer,"left");
y=this.containingBlockPosition.y+dojo.style.getPixelValue(this.constrainingContainer,"padding-top",true)+dojo.style.getBorderExtent(this.constrainingContainer,"top");
}
return {minX:x,minY:y,maxX:x+_85b-dojo.style.getOuterWidth(this.domNode),maxY:y+_85c-dojo.style.getOuterHeight(this.domNode)};
},updateDragOffset:function(){
var _85f=dojo.html.getScrollOffset();
if(_85f.y!=this.scrollOffset.y){
var diff=_85f.y-this.scrollOffset.y;
this.dragOffset.y+=diff;
this.scrollOffset.y=_85f.y;
}
if(_85f.x!=this.scrollOffset.x){
var diff=_85f.x-this.scrollOffset.x;
this.dragOffset.x+=diff;
this.scrollOffset.x=_85f.x;
}
},onDragMove:function(e){
this.updateDragOffset();
var x=this.dragOffset.x+e.pageX;
var y=this.dragOffset.y+e.pageY;
if(this.constrainToContainer){
if(x<this.constraints.minX){
x=this.constraints.minX;
}
if(y<this.constraints.minY){
y=this.constraints.minY;
}
if(x>this.constraints.maxX){
x=this.constraints.maxX;
}
if(y>this.constraints.maxY){
y=this.constraints.maxY;
}
}
this.setAbsolutePosition(x,y);
dojo.event.topic.publish("dragMove",{source:this});
},setAbsolutePosition:function(x,y){
if(!this.disableY){
this.dragClone.style.top=y+"px";
}
if(!this.disableX){
this.dragClone.style.left=x+"px";
}
},onDragEnd:function(e){
switch(e.dragStatus){
case "dropSuccess":
dojo.dom.removeNode(this.dragClone);
this.dragClone=null;
break;
case "dropFailure":
var _867=dojo.style.getAbsolutePosition(this.dragClone,true);
var _868=[this.dragStartPosition.x+1,this.dragStartPosition.y+1];
var line=new dojo.lfx.Line(_867,_868);
var anim=new dojo.lfx.Animation(500,line,dojo.lfx.easeOut);
var _86b=this;
dojo.event.connect(anim,"onAnimate",function(e){
_86b.dragClone.style.left=e[0]+"px";
_86b.dragClone.style.top=e[1]+"px";
});
dojo.event.connect(anim,"onEnd",function(e){
dojo.lang.setTimeout(function(){
dojo.dom.removeNode(_86b.dragClone);
_86b.dragClone=null;
},200);
});
anim.play();
break;
}
dojo.event.connect(this.domNode,"onclick",this,"squelchOnClick");
dojo.event.topic.publish("dragEnd",{source:this});
},squelchOnClick:function(e){
e.preventDefault();
dojo.event.disconnect(this.domNode,"onclick",this,"squelchOnClick");
},constrainTo:function(_86f){
this.constrainToContainer=true;
if(_86f){
this.constrainingContainer=_86f;
}else{
this.constrainingContainer=this.domNode.parentNode;
}
}});
dojo.dnd.HtmlDropTarget=function(node,_871){
if(arguments.length==0){
return;
}
this.domNode=dojo.byId(node);
dojo.dnd.DropTarget.call(this);
if(_871&&dojo.lang.isString(_871)){
_871=[_871];
}
this.acceptedTypes=_871||[];
};
dojo.inherits(dojo.dnd.HtmlDropTarget,dojo.dnd.DropTarget);
dojo.lang.extend(dojo.dnd.HtmlDropTarget,{onDragOver:function(e){
if(!this.accepts(e.dragObjects)){
return false;
}
this.childBoxes=[];
for(var i=0,child;i<this.domNode.childNodes.length;i++){
child=this.domNode.childNodes[i];
if(child.nodeType!=dojo.dom.ELEMENT_NODE){
continue;
}
var pos=dojo.style.getAbsolutePosition(child,true);
var _875=dojo.style.getInnerHeight(child);
var _876=dojo.style.getInnerWidth(child);
this.childBoxes.push({top:pos.y,bottom:pos.y+_875,left:pos.x,right:pos.x+_876,node:child});
}
return true;
},_getNodeUnderMouse:function(e){
for(var i=0,child;i<this.childBoxes.length;i++){
with(this.childBoxes[i]){
if(e.pageX>=left&&e.pageX<=right&&e.pageY>=top&&e.pageY<=bottom){
return i;
}
}
}
return -1;
},createDropIndicator:function(){
this.dropIndicator=document.createElement("div");
with(this.dropIndicator.style){
position="absolute";
zIndex=999;
borderTopWidth="1px";
borderTopColor="black";
borderTopStyle="solid";
width=dojo.style.getInnerWidth(this.domNode)+"px";
left=dojo.style.getAbsoluteX(this.domNode,true)+"px";
}
},onDragMove:function(e,_87a){
var i=this._getNodeUnderMouse(e);
if(!this.dropIndicator){
this.createDropIndicator();
}
if(i<0){
if(this.childBoxes.length){
var _87c=(dojo.html.gravity(this.childBoxes[0].node,e)&dojo.html.gravity.NORTH);
}else{
var _87c=true;
}
}else{
var _87d=this.childBoxes[i];
var _87c=(dojo.html.gravity(_87d.node,e)&dojo.html.gravity.NORTH);
}
this.placeIndicator(e,_87a,i,_87c);
if(!dojo.html.hasParent(this.dropIndicator)){
document.body.appendChild(this.dropIndicator);
}
},placeIndicator:function(e,_87f,_880,_881){
with(this.dropIndicator.style){
if(_880<0){
if(this.childBoxes.length){
top=(_881?this.childBoxes[0].top:this.childBoxes[this.childBoxes.length-1].bottom)+"px";
}else{
top=dojo.style.getAbsoluteY(this.domNode,true)+"px";
}
}else{
var _882=this.childBoxes[_880];
top=(_881?_882.top:_882.bottom)+"px";
}
}
},onDragOut:function(e){
if(this.dropIndicator){
dojo.dom.removeNode(this.dropIndicator);
delete this.dropIndicator;
}
},onDrop:function(e){
this.onDragOut(e);
var i=this._getNodeUnderMouse(e);
if(i<0){
if(this.childBoxes.length){
if(dojo.html.gravity(this.childBoxes[0].node,e)&dojo.html.gravity.NORTH){
return this.insert(e,this.childBoxes[0].node,"before");
}else{
return this.insert(e,this.childBoxes[this.childBoxes.length-1].node,"after");
}
}
return this.insert(e,this.domNode,"append");
}
var _886=this.childBoxes[i];
if(dojo.html.gravity(_886.node,e)&dojo.html.gravity.NORTH){
return this.insert(e,_886.node,"before");
}else{
return this.insert(e,_886.node,"after");
}
},insert:function(e,_888,_889){
var node=e.dragObject.domNode;
if(_889=="before"){
return dojo.html.insertBefore(node,_888);
}else{
if(_889=="after"){
return dojo.html.insertAfter(node,_888);
}else{
if(_889=="append"){
_888.appendChild(node);
return true;
}
}
}
return false;
}});
dojo.kwCompoundRequire({common:["dojo.dnd.DragAndDrop"],browser:["dojo.dnd.HtmlDragAndDrop"],dashboard:["dojo.dnd.HtmlDragAndDrop"]});
dojo.provide("dojo.dnd.*");
dojo.provide("dojo.widget.Manager");
dojo.require("dojo.lang.array");
dojo.require("dojo.lang.func");
dojo.require("dojo.event.*");
dojo.widget.manager=new function(){
this.widgets=[];
this.widgetIds=[];
this.topWidgets={};
var _88b={};
var _88c=[];
this.getUniqueId=function(_88d){
return _88d+"_"+(_88b[_88d]!=undefined?++_88b[_88d]:_88b[_88d]=0);
};
this.add=function(_88e){
dojo.profile.start("dojo.widget.manager.add");
this.widgets.push(_88e);
if(!_88e.extraArgs["id"]){
_88e.extraArgs["id"]=_88e.extraArgs["ID"];
}
if(_88e.widgetId==""){
if(_88e["id"]){
_88e.widgetId=_88e["id"];
}else{
if(_88e.extraArgs["id"]){
_88e.widgetId=_88e.extraArgs["id"];
}else{
_88e.widgetId=this.getUniqueId(_88e.widgetType);
}
}
}
if(this.widgetIds[_88e.widgetId]){
dojo.debug("widget ID collision on ID: "+_88e.widgetId);
}
this.widgetIds[_88e.widgetId]=_88e;
dojo.profile.end("dojo.widget.manager.add");
};
this.destroyAll=function(){
for(var x=this.widgets.length-1;x>=0;x--){
try{
this.widgets[x].destroy(true);
delete this.widgets[x];
}
catch(e){
}
}
};
this.remove=function(_890){
var tw=this.widgets[_890].widgetId;
delete this.widgetIds[tw];
this.widgets.splice(_890,1);
};
this.removeById=function(id){
for(var i=0;i<this.widgets.length;i++){
if(this.widgets[i].widgetId==id){
this.remove(i);
break;
}
}
};
this.getWidgetById=function(id){
return this.widgetIds[id];
};
this.getWidgetsByType=function(type){
var lt=type.toLowerCase();
var ret=[];
dojo.lang.forEach(this.widgets,function(x){
if(x.widgetType.toLowerCase()==lt){
ret.push(x);
}
});
return ret;
};
this.getWidgetsOfType=function(id){
dojo.deprecated("getWidgetsOfType","use getWidgetsByType","0.4");
return dojo.widget.manager.getWidgetsByType(id);
};
this.getWidgetsByFilter=function(_89a,_89b){
var ret=[];
dojo.lang.every(this.widgets,function(x){
if(_89a(x)){
ret.push(x);
if(_89b){
return false;
}
}
return true;
});
return (_89b?ret[0]:ret);
};
this.getAllWidgets=function(){
return this.widgets.concat();
};
this.getWidgetByNode=function(node){
var w=this.getAllWidgets();
for(var i=0;i<w.length;i++){
if(w[i].domNode==node){
return w[i];
}
}
return null;
};
this.byId=this.getWidgetById;
this.byType=this.getWidgetsByType;
this.byFilter=this.getWidgetsByFilter;
this.byNode=this.getWidgetByNode;
var _8a1={};
var _8a2=["dojo.widget"];
for(var i=0;i<_8a2.length;i++){
_8a2[_8a2[i]]=true;
}
this.registerWidgetPackage=function(_8a4){
if(!_8a2[_8a4]){
_8a2[_8a4]=true;
_8a2.push(_8a4);
}
};
this.getWidgetPackageList=function(){
return dojo.lang.map(_8a2,function(elt){
return (elt!==true?elt:undefined);
});
};
this.getImplementation=function(_8a6,_8a7,_8a8){
var impl=this.getImplementationName(_8a6);
if(impl){
var ret=new impl(_8a7);
return ret;
}
};
this.getImplementationName=function(_8ab){
var _8ac=_8ab.toLowerCase();
var impl=_8a1[_8ac];
if(impl){
return impl;
}
if(!_88c.length){
for(var _8ae in dojo.render){
if(dojo.render[_8ae]["capable"]===true){
var _8af=dojo.render[_8ae].prefixes;
for(var i=0;i<_8af.length;i++){
_88c.push(_8af[i].toLowerCase());
}
}
}
_88c.push("");
}
for(var i=0;i<_8a2.length;i++){
var _8b1=dojo.evalObjPath(_8a2[i]);
if(!_8b1){
continue;
}
for(var j=0;j<_88c.length;j++){
if(!_8b1[_88c[j]]){
continue;
}
for(var _8b3 in _8b1[_88c[j]]){
if(_8b3.toLowerCase()!=_8ac){
continue;
}
_8a1[_8ac]=_8b1[_88c[j]][_8b3];
return _8a1[_8ac];
}
}
for(var j=0;j<_88c.length;j++){
for(var _8b3 in _8b1){
if(_8b3.toLowerCase()!=(_88c[j]+_8ac)){
continue;
}
_8a1[_8ac]=_8b1[_8b3];
return _8a1[_8ac];
}
}
}
throw new Error("Could not locate \""+_8ab+"\" class");
};
this.resizing=false;
this.onWindowResized=function(){
if(this.resizing){
return;
}
try{
this.resizing=true;
for(var id in this.topWidgets){
var _8b5=this.topWidgets[id];
if(_8b5.checkSize){
_8b5.checkSize();
}
}
}
catch(e){
}
finally{
this.resizing=false;
}
};
if(typeof window!="undefined"){
dojo.addOnLoad(this,"onWindowResized");
dojo.event.connect(window,"onresize",this,"onWindowResized");
}
};
(function(){
var dw=dojo.widget;
var dwm=dw.manager;
var h=dojo.lang.curry(dojo.lang,"hitch",dwm);
var g=function(_8ba,_8bb){
dw[(_8bb||_8ba)]=h(_8ba);
};
g("add","addWidget");
g("destroyAll","destroyAllWidgets");
g("remove","removeWidget");
g("removeById","removeWidgetById");
g("getWidgetById");
g("getWidgetById","byId");
g("getWidgetsByType");
g("getWidgetsByFilter");
g("getWidgetsByType","byType");
g("getWidgetsByFilter","byFilter");
g("getWidgetByNode","byNode");
dw.all=function(n){
var _8bd=dwm.getAllWidgets.apply(dwm,arguments);
if(arguments.length>0){
return _8bd[n];
}
return _8bd;
};
g("registerWidgetPackage");
g("getImplementation","getWidgetImplementation");
g("getImplementationName","getWidgetImplementationName");
dw.widgets=dwm.widgets;
dw.widgetIds=dwm.widgetIds;
dw.root=dwm.root;
})();
dojo.provide("dojo.widget.Widget");
dojo.provide("dojo.widget.tags");
dojo.require("dojo.lang.func");
dojo.require("dojo.lang.array");
dojo.require("dojo.lang.extras");
dojo.require("dojo.lang.declare");
dojo.require("dojo.widget.Manager");
dojo.require("dojo.event.*");
dojo.declare("dojo.widget.Widget",null,{initializer:function(){
this.children=[];
this.extraArgs={};
},parent:null,isTopLevel:false,isModal:false,isEnabled:true,isHidden:false,isContainer:false,widgetId:"",widgetType:"Widget",toString:function(){
return "[Widget "+this.widgetType+", "+(this.widgetId||"NO ID")+"]";
},repr:function(){
return this.toString();
},enable:function(){
this.isEnabled=true;
},disable:function(){
this.isEnabled=false;
},hide:function(){
this.isHidden=true;
},show:function(){
this.isHidden=false;
},onResized:function(){
this.notifyChildrenOfResize();
},notifyChildrenOfResize:function(){
for(var i=0;i<this.children.length;i++){
var _8bf=this.children[i];
if(_8bf.onResized){
_8bf.onResized();
}
}
},create:function(args,_8c1,_8c2){
this.satisfyPropertySets(args,_8c1,_8c2);
this.mixInProperties(args,_8c1,_8c2);
this.postMixInProperties(args,_8c1,_8c2);
dojo.widget.manager.add(this);
this.buildRendering(args,_8c1,_8c2);
this.initialize(args,_8c1,_8c2);
this.postInitialize(args,_8c1,_8c2);
this.postCreate(args,_8c1,_8c2);
return this;
},destroy:function(_8c3){
this.destroyChildren();
this.uninitialize();
this.destroyRendering(_8c3);
dojo.widget.manager.removeById(this.widgetId);
},destroyChildren:function(){
while(this.children.length>0){
var tc=this.children[0];
this.removeChild(tc);
tc.destroy();
}
},getChildrenOfType:function(type,_8c6){
var ret=[];
var _8c8=dojo.lang.isFunction(type);
if(!_8c8){
type=type.toLowerCase();
}
for(var x=0;x<this.children.length;x++){
if(_8c8){
if(this.children[x] instanceof type){
ret.push(this.children[x]);
}
}else{
if(this.children[x].widgetType.toLowerCase()==type){
ret.push(this.children[x]);
}
}
if(_8c6){
ret=ret.concat(this.children[x].getChildrenOfType(type,_8c6));
}
}
return ret;
},getDescendants:function(){
var _8ca=[];
var _8cb=[this];
var elem;
while(elem=_8cb.pop()){
_8ca.push(elem);
dojo.lang.forEach(elem.children,function(elem){
_8cb.push(elem);
});
}
return _8ca;
},satisfyPropertySets:function(args){
return args;
},mixInProperties:function(args,frag){
if((args["fastMixIn"])||(frag["fastMixIn"])){
for(var x in args){
this[x]=args[x];
}
return;
}
var _8d2;
var _8d3=dojo.widget.lcArgsCache[this.widgetType];
if(_8d3==null){
_8d3={};
for(var y in this){
_8d3[((new String(y)).toLowerCase())]=y;
}
dojo.widget.lcArgsCache[this.widgetType]=_8d3;
}
var _8d5={};
for(var x in args){
if(!this[x]){
var y=_8d3[(new String(x)).toLowerCase()];
if(y){
args[y]=args[x];
x=y;
}
}
if(_8d5[x]){
continue;
}
_8d5[x]=true;
if((typeof this[x])!=(typeof _8d2)){
if(typeof args[x]!="string"){
this[x]=args[x];
}else{
if(dojo.lang.isString(this[x])){
this[x]=args[x];
}else{
if(dojo.lang.isNumber(this[x])){
this[x]=new Number(args[x]);
}else{
if(dojo.lang.isBoolean(this[x])){
this[x]=(args[x].toLowerCase()=="false")?false:true;
}else{
if(dojo.lang.isFunction(this[x])){
if(args[x].search(/[^\w\.]+/i)==-1){
this[x]=dojo.evalObjPath(args[x],false);
}else{
var tn=dojo.lang.nameAnonFunc(new Function(args[x]),this);
dojo.event.connect(this,x,this,tn);
}
}else{
if(dojo.lang.isArray(this[x])){
this[x]=args[x].split(";");
}else{
if(this[x] instanceof Date){
this[x]=new Date(Number(args[x]));
}else{
if(typeof this[x]=="object"){
if(this[x] instanceof dojo.uri.Uri){
this[x]=args[x];
}else{
var _8d7=args[x].split(";");
for(var y=0;y<_8d7.length;y++){
var si=_8d7[y].indexOf(":");
if((si!=-1)&&(_8d7[y].length>si)){
this[x][_8d7[y].substr(0,si).replace(/^\s+|\s+$/g,"")]=_8d7[y].substr(si+1);
}
}
}
}else{
this[x]=args[x];
}
}
}
}
}
}
}
}
}else{
this.extraArgs[x.toLowerCase()]=args[x];
}
}
},postMixInProperties:function(){
},initialize:function(args,frag){
return false;
},postInitialize:function(args,frag){
return false;
},postCreate:function(args,frag){
return false;
},uninitialize:function(){
return false;
},buildRendering:function(){
dojo.unimplemented("dojo.widget.Widget.buildRendering, on "+this.toString()+", ");
return false;
},destroyRendering:function(){
dojo.unimplemented("dojo.widget.Widget.destroyRendering");
return false;
},cleanUp:function(){
dojo.unimplemented("dojo.widget.Widget.cleanUp");
return false;
},addedTo:function(_8df){
},addChild:function(_8e0){
dojo.unimplemented("dojo.widget.Widget.addChild");
return false;
},removeChild:function(_8e1){
for(var x=0;x<this.children.length;x++){
if(this.children[x]===_8e1){
this.children.splice(x,1);
break;
}
}
return _8e1;
},resize:function(_8e3,_8e4){
this.setWidth(_8e3);
this.setHeight(_8e4);
},setWidth:function(_8e5){
if((typeof _8e5=="string")&&(_8e5.substr(-1)=="%")){
this.setPercentageWidth(_8e5);
}else{
this.setNativeWidth(_8e5);
}
},setHeight:function(_8e6){
if((typeof _8e6=="string")&&(_8e6.substr(-1)=="%")){
this.setPercentageHeight(_8e6);
}else{
this.setNativeHeight(_8e6);
}
},setPercentageHeight:function(_8e7){
return false;
},setNativeHeight:function(_8e8){
return false;
},setPercentageWidth:function(_8e9){
return false;
},setNativeWidth:function(_8ea){
return false;
},getPreviousSibling:function(){
var idx=this.getParentIndex();
if(idx<=0){
return null;
}
return this.getSiblings()[idx-1];
},getSiblings:function(){
return this.parent.children;
},getParentIndex:function(){
return dojo.lang.indexOf(this.getSiblings(),this,true);
},getNextSibling:function(){
var idx=this.getParentIndex();
if(idx==this.getSiblings().length-1){
return null;
}
if(idx<0){
return null;
}
return this.getSiblings()[idx+1];
}});
dojo.widget.lcArgsCache={};
dojo.widget.tags={};
dojo.widget.tags.addParseTreeHandler=function(type){
var _8ee=type.toLowerCase();
this[_8ee]=function(_8ef,_8f0,_8f1,_8f2,_8f3){
return dojo.widget.buildWidgetFromParseTree(_8ee,_8ef,_8f0,_8f1,_8f2,_8f3);
};
};
dojo.widget.tags.addParseTreeHandler("dojo:widget");
dojo.widget.tags["dojo:propertyset"]=function(_8f4,_8f5,_8f6){
var _8f7=_8f5.parseProperties(_8f4["dojo:propertyset"]);
};
dojo.widget.tags["dojo:connect"]=function(_8f8,_8f9,_8fa){
var _8fb=_8f9.parseProperties(_8f8["dojo:connect"]);
};
dojo.widget.buildWidgetFromParseTree=function(type,frag,_8fe,_8ff,_900,_901){
var _902=type.split(":");
_902=(_902.length==2)?_902[1]:type;
var _903=_901||_8fe.parseProperties(frag["dojo:"+_902]);
var _904=dojo.widget.manager.getImplementation(_902);
if(!_904){
throw new Error("cannot find \""+_902+"\" widget");
}else{
if(!_904.create){
throw new Error("\""+_902+"\" widget object does not appear to implement *Widget");
}
}
_903["dojoinsertionindex"]=_900;
var ret=_904.create(_903,frag,_8ff);
return ret;
};
dojo.widget.defineWidget=function(_906,_907,_908,init,_90a){
if(dojo.lang.isString(arguments[3])){
dojo.widget._defineWidget(arguments[0],arguments[3],arguments[1],arguments[4],arguments[2]);
}else{
var args=[arguments[0]],p=3;
if(dojo.lang.isString(arguments[1])){
args.push(arguments[1],arguments[2]);
}else{
args.push("",arguments[1]);
p=2;
}
if(dojo.lang.isFunction(arguments[p])){
args.push(arguments[p],arguments[p+1]);
}else{
args.push(null,arguments[p]);
}
dojo.widget._defineWidget.apply(this,args);
}
};
dojo.widget.defineWidget.renderers="html|svg|vml";
dojo.widget._defineWidget=function(_90c,_90d,_90e,init,_910){
var _911=_90c.split(".");
var type=_911.pop();
var regx="\\.("+(_90d?_90d+"|":"")+dojo.widget.defineWidget.renderers+")\\.";
var r=_90c.search(new RegExp(regx));
_911=(r<0?_911.join("."):_90c.substr(0,r));
dojo.widget.manager.registerWidgetPackage(_911);
dojo.widget.tags.addParseTreeHandler("dojo:"+type.toLowerCase());
_910=(_910)||{};
_910.widgetType=type;
if((!init)&&(_910["classConstructor"])){
init=_910.classConstructor;
delete _910.classConstructor;
}
dojo.declare(_90c,_90e,init,_910);
};
dojo.provide("dojo.widget.Parse");
dojo.require("dojo.widget.Manager");
dojo.require("dojo.dom");
dojo.widget.Parse=function(_915){
this.propertySetsList=[];
this.fragment=_915;
this.createComponents=function(frag,_917){
var _918=[];
var _919=false;
try{
if((frag)&&(frag["tagName"])&&(frag!=frag["nodeRef"])){
var _91a=dojo.widget.tags;
var tna=String(frag["tagName"]).split(";");
for(var x=0;x<tna.length;x++){
var ltn=(tna[x].replace(/^\s+|\s+$/g,"")).toLowerCase();
if(_91a[ltn]){
_919=true;
frag.tagName=ltn;
var ret=_91a[ltn](frag,this,_917,frag["index"]);
_918.push(ret);
}else{
if((dojo.lang.isString(ltn))&&(ltn.substr(0,5)=="dojo:")){
dojo.debug("no tag handler registed for type: ",ltn);
}
}
}
}
}
catch(e){
dojo.debug("dojo.widget.Parse: error:",e);
}
if(!_919){
_918=_918.concat(this.createSubComponents(frag,_917));
}
return _918;
};
this.createSubComponents=function(_91f,_920){
var frag,comps=[];
for(var item in _91f){
frag=_91f[item];
if((frag)&&(typeof frag=="object")&&(frag!=_91f.nodeRef)&&(frag!=_91f["tagName"])){
comps=comps.concat(this.createComponents(frag,_920));
}
}
return comps;
};
this.parsePropertySets=function(_923){
return [];
var _924=[];
for(var item in _923){
if((_923[item]["tagName"]=="dojo:propertyset")){
_924.push(_923[item]);
}
}
this.propertySetsList.push(_924);
return _924;
};
this.parseProperties=function(_926){
var _927={};
for(var item in _926){
if((_926[item]==_926["tagName"])||(_926[item]==_926.nodeRef)){
}else{
if((_926[item]["tagName"])&&(dojo.widget.tags[_926[item].tagName.toLowerCase()])){
}else{
if((_926[item][0])&&(_926[item][0].value!="")&&(_926[item][0].value!=null)){
try{
if(item.toLowerCase()=="dataprovider"){
var _929=this;
this.getDataProvider(_929,_926[item][0].value);
_927.dataProvider=this.dataProvider;
}
_927[item]=_926[item][0].value;
var _92a=this.parseProperties(_926[item]);
for(var _92b in _92a){
_927[_92b]=_92a[_92b];
}
}
catch(e){
dojo.debug(e);
}
}
}
}
}
return _927;
};
this.getDataProvider=function(_92c,_92d){
dojo.io.bind({url:_92d,load:function(type,_92f){
if(type=="load"){
_92c.dataProvider=_92f;
}
},mimetype:"text/javascript",sync:true});
};
this.getPropertySetById=function(_930){
for(var x=0;x<this.propertySetsList.length;x++){
if(_930==this.propertySetsList[x]["id"][0].value){
return this.propertySetsList[x];
}
}
return "";
};
this.getPropertySetsByType=function(_932){
var _933=[];
for(var x=0;x<this.propertySetsList.length;x++){
var cpl=this.propertySetsList[x];
var cpcc=cpl["componentClass"]||cpl["componentType"]||null;
if((cpcc)&&(propertySetId==cpcc[0].value)){
_933.push(cpl);
}
}
return _933;
};
this.getPropertySets=function(_937){
var ppl="dojo:propertyproviderlist";
var _939=[];
var _93a=_937["tagName"];
if(_937[ppl]){
var _93b=_937[ppl].value.split(" ");
for(var _93c in _93b){
if((_93c.indexOf("..")==-1)&&(_93c.indexOf("://")==-1)){
var _93d=this.getPropertySetById(_93c);
if(_93d!=""){
_939.push(_93d);
}
}else{
}
}
}
return (this.getPropertySetsByType(_93a)).concat(_939);
};
this.createComponentFromScript=function(_93e,_93f,_940){
var ltn="dojo:"+_93f.toLowerCase();
if(dojo.widget.tags[ltn]){
_940.fastMixIn=true;
return [dojo.widget.tags[ltn](_940,this,null,null,_940)];
}else{
if(ltn.substr(0,5)=="dojo:"){
dojo.debug("no tag handler registed for type: ",ltn);
}
}
};
};
dojo.widget._parser_collection={"dojo":new dojo.widget.Parse()};
dojo.widget.getParser=function(name){
if(!name){
name="dojo";
}
if(!this._parser_collection[name]){
this._parser_collection[name]=new dojo.widget.Parse();
}
return this._parser_collection[name];
};
dojo.widget.createWidget=function(name,_944,_945,_946){
var _947=name.toLowerCase();
var _948="dojo:"+_947;
var _949=(dojo.byId(name)&&(!dojo.widget.tags[_948]));
if((arguments.length==1)&&((typeof name!="string")||(_949))){
var xp=new dojo.xml.Parse();
var tn=(_949)?dojo.byId(name):name;
return dojo.widget.getParser().createComponents(xp.parseElement(tn,null,true))[0];
}
function fromScript(_94c,name,_94e){
_94e[_948]={dojotype:[{value:_947}],nodeRef:_94c,fastMixIn:true};
return dojo.widget.getParser().createComponentFromScript(_94c,name,_94e,true);
}
if(typeof name!="string"&&typeof _944=="string"){
dojo.deprecated("dojo.widget.createWidget","argument order is now of the form "+"dojo.widget.createWidget(NAME, [PROPERTIES, [REFERENCENODE, [POSITION]]])","0.4");
return fromScript(name,_944,_945);
}
_944=_944||{};
var _94f=false;
var tn=null;
var h=dojo.render.html.capable;
if(h){
tn=document.createElement("span");
}
if(!_945){
_94f=true;
_945=tn;
if(h){
document.body.appendChild(_945);
}
}else{
if(_946){
dojo.dom.insertAtPosition(tn,_945,_946);
}else{
tn=_945;
}
}
var _951=fromScript(tn,name,_944);
if(!_951||!_951[0]||typeof _951[0].widgetType=="undefined"){
throw new Error("createWidget: Creation of \""+name+"\" widget failed.");
}
if(_94f){
if(_951[0].domNode.parentNode){
_951[0].domNode.parentNode.removeChild(_951[0].domNode);
}
}
return _951[0];
};
dojo.widget.fromScript=function(name,_953,_954,_955){
dojo.deprecated("dojo.widget.fromScript"," use "+"dojo.widget.createWidget instead","0.4");
return dojo.widget.createWidget(name,_953,_954,_955);
};
dojo.provide("dojo.widget.DomWidget");
dojo.require("dojo.event.*");
dojo.require("dojo.widget.Widget");
dojo.require("dojo.dom");
dojo.require("dojo.xml.Parse");
dojo.require("dojo.uri.*");
dojo.require("dojo.lang.func");
dojo.require("dojo.lang.extras");
dojo.widget._cssFiles={};
dojo.widget._cssStrings={};
dojo.widget._templateCache={};
dojo.widget.defaultStrings={dojoRoot:dojo.hostenv.getBaseScriptUri(),baseScriptUri:dojo.hostenv.getBaseScriptUri()};
dojo.widget.buildFromTemplate=function(){
dojo.lang.forward("fillFromTemplateCache");
};
dojo.widget.fillFromTemplateCache=function(obj,_957,_958,_959,_95a){
var _95b=_957||obj.templatePath;
var _95c=_958||obj.templateCssPath;
if(_95b&&!(_95b instanceof dojo.uri.Uri)){
_95b=dojo.uri.dojoUri(_95b);
dojo.deprecated("templatePath should be of type dojo.uri.Uri",null,"0.4");
}
if(_95c&&!(_95c instanceof dojo.uri.Uri)){
_95c=dojo.uri.dojoUri(_95c);
dojo.deprecated("templateCssPath should be of type dojo.uri.Uri",null,"0.4");
}
var _95d=dojo.widget._templateCache;
if(!obj["widgetType"]){
do{
var _95e="__dummyTemplate__"+dojo.widget._templateCache.dummyCount++;
}while(_95d[_95e]);
obj.widgetType=_95e;
}
var wt=obj.widgetType;
if(_95c&&!dojo.widget._cssFiles[_95c.toString()]){
if((!obj.templateCssString)&&(_95c)){
obj.templateCssString=dojo.hostenv.getText(_95c);
obj.templateCssPath=null;
}
if((obj["templateCssString"])&&(!obj.templateCssString["loaded"])){
dojo.style.insertCssText(obj.templateCssString,null,_95c);
if(!obj.templateCssString){
obj.templateCssString="";
}
obj.templateCssString.loaded=true;
}
dojo.widget._cssFiles[_95c.toString()]=true;
}
var ts=_95d[wt];
if(!ts){
_95d[wt]={"string":null,"node":null};
if(_95a){
ts={};
}else{
ts=_95d[wt];
}
}
if((!obj.templateString)&&(!_95a)){
obj.templateString=_959||ts["string"];
}
if((!obj.templateNode)&&(!_95a)){
obj.templateNode=ts["node"];
}
if((!obj.templateNode)&&(!obj.templateString)&&(_95b)){
var _961=dojo.hostenv.getText(_95b);
if(_961){
_961=_961.replace(/^\s*<\?xml(\s)+version=[\'\"](\d)*.(\d)*[\'\"](\s)*\?>/im,"");
var _962=_961.match(/<body[^>]*>\s*([\s\S]+)\s*<\/body>/im);
if(_962){
_961=_962[1];
}
}else{
_961="";
}
obj.templateString=_961;
if(!_95a){
_95d[wt]["string"]=_961;
}
}
if((!ts["string"])&&(!_95a)){
ts.string=obj.templateString;
}
};
dojo.widget._templateCache.dummyCount=0;
dojo.widget.attachProperties=["dojoAttachPoint","id"];
dojo.widget.eventAttachProperty="dojoAttachEvent";
dojo.widget.onBuildProperty="dojoOnBuild";
dojo.widget.waiNames=["waiRole","waiState"];
dojo.widget.wai={waiRole:{name:"waiRole",namespace:"http://www.w3.org/TR/xhtml2",alias:"x2",prefix:"wairole:",nsName:"role"},waiState:{name:"waiState",namespace:"http://www.w3.org/2005/07/aaa",alias:"aaa",prefix:"",nsName:"state"},setAttr:function(node,attr,_965){
if(dojo.render.html.ie){
node.setAttribute(this[attr].alias+":"+this[attr].nsName,this[attr].prefix+_965);
}else{
node.setAttributeNS(this[attr].namespace,this[attr].nsName,this[attr].prefix+_965);
}
}};
dojo.widget.attachTemplateNodes=function(_966,_967,_968){
var _969=dojo.dom.ELEMENT_NODE;
function trim(str){
return str.replace(/^\s+|\s+$/g,"");
}
if(!_966){
_966=_967.domNode;
}
if(_966.nodeType!=_969){
return;
}
var _96b=_966.all||_966.getElementsByTagName("*");
var _96c=_967;
for(var x=-1;x<_96b.length;x++){
var _96e=(x==-1)?_966:_96b[x];
var _96f=[];
for(var y=0;y<this.attachProperties.length;y++){
var _971=_96e.getAttribute(this.attachProperties[y]);
if(_971){
_96f=_971.split(";");
for(var z=0;z<_96f.length;z++){
if(dojo.lang.isArray(_967[_96f[z]])){
_967[_96f[z]].push(_96e);
}else{
_967[_96f[z]]=_96e;
}
}
break;
}
}
var _973=_96e.getAttribute(this.templateProperty);
if(_973){
_967[_973]=_96e;
}
dojo.lang.forEach(dojo.widget.waiNames,function(name){
var wai=dojo.widget.wai[name];
var val=_96e.getAttribute(wai.name);
if(val){
dojo.widget.wai.setAttr(_96e,wai.name,val);
}
},this);
var _977=_96e.getAttribute(this.eventAttachProperty);
if(_977){
var evts=_977.split(";");
for(var y=0;y<evts.length;y++){
if((!evts[y])||(!evts[y].length)){
continue;
}
var _979=null;
var tevt=trim(evts[y]);
if(evts[y].indexOf(":")>=0){
var _97b=tevt.split(":");
tevt=trim(_97b[0]);
_979=trim(_97b[1]);
}
if(!_979){
_979=tevt;
}
var tf=function(){
var ntf=new String(_979);
return function(evt){
if(_96c[ntf]){
_96c[ntf](dojo.event.browser.fixEvent(evt,this));
}
};
}();
dojo.event.browser.addListener(_96e,tevt,tf,false,true);
}
}
for(var y=0;y<_968.length;y++){
var _97f=_96e.getAttribute(_968[y]);
if((_97f)&&(_97f.length)){
var _979=null;
var _980=_968[y].substr(4);
_979=trim(_97f);
var _981=[_979];
if(_979.indexOf(";")>=0){
_981=dojo.lang.map(_979.split(";"),trim);
}
for(var z=0;z<_981.length;z++){
if(!_981[z].length){
continue;
}
var tf=function(){
var ntf=new String(_981[z]);
return function(evt){
if(_96c[ntf]){
_96c[ntf](dojo.event.browser.fixEvent(evt,this));
}
};
}();
dojo.event.browser.addListener(_96e,_980,tf,false,true);
}
}
}
var _984=_96e.getAttribute(this.onBuildProperty);
if(_984){
eval("var node = baseNode; var widget = targetObj; "+_984);
}
}
};
dojo.widget.getDojoEventsFromStr=function(str){
var re=/(dojoOn([a-z]+)(\s?))=/gi;
var evts=str?str.match(re)||[]:[];
var ret=[];
var lem={};
for(var x=0;x<evts.length;x++){
if(evts[x].legth<1){
continue;
}
var cm=evts[x].replace(/\s/,"");
cm=(cm.slice(0,cm.length-1));
if(!lem[cm]){
lem[cm]=true;
ret.push(cm);
}
}
return ret;
};
dojo.declare("dojo.widget.DomWidget",dojo.widget.Widget,{initializer:function(){
if((arguments.length>0)&&(typeof arguments[0]=="object")){
this.create(arguments[0]);
}
},templateNode:null,templateString:null,templateCssString:null,preventClobber:false,domNode:null,containerNode:null,addChild:function(_98c,_98d,pos,ref,_990){
if(!this.isContainer){
dojo.debug("dojo.widget.DomWidget.addChild() attempted on non-container widget");
return null;
}else{
this.addWidgetAsDirectChild(_98c,_98d,pos,ref,_990);
this.registerChild(_98c,_990);
}
return _98c;
},addWidgetAsDirectChild:function(_991,_992,pos,ref,_995){
if((!this.containerNode)&&(!_992)){
this.containerNode=this.domNode;
}
var cn=(_992)?_992:this.containerNode;
if(!pos){
pos="after";
}
if(!ref){
if(!cn){
cn=document.body;
}
ref=cn.lastChild;
}
if(!_995){
_995=0;
}
_991.domNode.setAttribute("dojoinsertionindex",_995);
if(!ref){
cn.appendChild(_991.domNode);
}else{
if(pos=="insertAtIndex"){
dojo.dom.insertAtIndex(_991.domNode,ref.parentNode,_995);
}else{
if((pos=="after")&&(ref===cn.lastChild)){
cn.appendChild(_991.domNode);
}else{
dojo.dom.insertAtPosition(_991.domNode,cn,pos);
}
}
}
},registerChild:function(_997,_998){
_997.dojoInsertionIndex=_998;
var idx=-1;
for(var i=0;i<this.children.length;i++){
if(this.children[i].dojoInsertionIndex<_998){
idx=i;
}
}
this.children.splice(idx+1,0,_997);
_997.parent=this;
_997.addedTo(this);
delete dojo.widget.manager.topWidgets[_997.widgetId];
},removeChild:function(_99b){
dojo.dom.removeNode(_99b.domNode);
return dojo.widget.DomWidget.superclass.removeChild.call(this,_99b);
},getFragNodeRef:function(frag){
if(!frag||!frag["dojo:"+this.widgetType.toLowerCase()]){
dojo.raise("Error: no frag for widget type "+this.widgetType+", id "+this.widgetId+" (maybe a widget has set it's type incorrectly)");
}
return (frag?frag["dojo:"+this.widgetType.toLowerCase()]["nodeRef"]:null);
},postInitialize:function(args,frag,_99f){
var _9a0=this.getFragNodeRef(frag);
if(_99f&&(_99f.snarfChildDomOutput||!_9a0)){
_99f.addWidgetAsDirectChild(this,"","insertAtIndex","",args["dojoinsertionindex"],_9a0);
}else{
if(_9a0){
if(this.domNode&&(this.domNode!==_9a0)){
var _9a1=_9a0.parentNode.replaceChild(this.domNode,_9a0);
}
}
}
if(_99f){
_99f.registerChild(this,args.dojoinsertionindex);
}else{
dojo.widget.manager.topWidgets[this.widgetId]=this;
}
if(this.isContainer){
var _9a2=dojo.widget.getParser();
_9a2.createSubComponents(frag,this);
}
},buildRendering:function(args,frag){
var ts=dojo.widget._templateCache[this.widgetType];
if((!this.preventClobber)&&((this.templatePath)||(this.templateNode)||((this["templateString"])&&(this.templateString.length))||((typeof ts!="undefined")&&((ts["string"])||(ts["node"]))))){
this.buildFromTemplate(args,frag);
}else{
this.domNode=this.getFragNodeRef(frag);
}
this.fillInTemplate(args,frag);
},buildFromTemplate:function(args,frag){
var _9a8=false;
if(args["templatecsspath"]){
args["templateCssPath"]=args["templatecsspath"];
}
if(args["templatepath"]){
_9a8=true;
args["templatePath"]=args["templatepath"];
}
dojo.widget.fillFromTemplateCache(this,args["templatePath"],args["templateCssPath"],null,_9a8);
var ts=dojo.widget._templateCache[this.widgetType];
if((ts)&&(!_9a8)){
if(!this.templateString.length){
this.templateString=ts["string"];
}
if(!this.templateNode){
this.templateNode=ts["node"];
}
}
var _9aa=false;
var node=null;
var tstr=this.templateString;
if((!this.templateNode)&&(this.templateString)){
_9aa=this.templateString.match(/\$\{([^\}]+)\}/g);
if(_9aa){
var hash=this.strings||{};
for(var key in dojo.widget.defaultStrings){
if(dojo.lang.isUndefined(hash[key])){
hash[key]=dojo.widget.defaultStrings[key];
}
}
for(var i=0;i<_9aa.length;i++){
var key=_9aa[i];
key=key.substring(2,key.length-1);
var kval=(key.substring(0,5)=="this.")?dojo.lang.getObjPathValue(key.substring(5),this):hash[key];
var _9b1;
if((kval)||(dojo.lang.isString(kval))){
_9b1=(dojo.lang.isFunction(kval))?kval.call(this,key,this.templateString):kval;
tstr=tstr.replace(_9aa[i],_9b1);
}
}
}else{
this.templateNode=this.createNodesFromText(this.templateString,true)[0];
if(!_9a8){
ts.node=this.templateNode;
}
}
}
if((!this.templateNode)&&(!_9aa)){
dojo.debug("weren't able to create template!");
return false;
}else{
if(!_9aa){
node=this.templateNode.cloneNode(true);
if(!node){
return false;
}
}else{
node=this.createNodesFromText(tstr,true)[0];
}
}
this.domNode=node;
this.attachTemplateNodes(this.domNode,this);
if(this.isContainer&&this.containerNode){
var src=this.getFragNodeRef(frag);
if(src){
dojo.dom.moveChildren(src,this.containerNode);
}
}
},attachTemplateNodes:function(_9b3,_9b4){
if(!_9b4){
_9b4=this;
}
return dojo.widget.attachTemplateNodes(_9b3,_9b4,dojo.widget.getDojoEventsFromStr(this.templateString));
},fillInTemplate:function(){
},destroyRendering:function(){
try{
delete this.domNode;
}
catch(e){
}
},cleanUp:function(){
},getContainerHeight:function(){
dojo.unimplemented("dojo.widget.DomWidget.getContainerHeight");
},getContainerWidth:function(){
dojo.unimplemented("dojo.widget.DomWidget.getContainerWidth");
},createNodesFromText:function(){
dojo.unimplemented("dojo.widget.DomWidget.createNodesFromText");
}});
dojo.provide("dojo.lfx.toggle");
dojo.require("dojo.lfx.*");
dojo.lfx.toggle.plain={show:function(node,_9b6,_9b7,_9b8){
dojo.style.show(node);
if(dojo.lang.isFunction(_9b8)){
_9b8();
}
},hide:function(node,_9ba,_9bb,_9bc){
dojo.style.hide(node);
if(dojo.lang.isFunction(_9bc)){
_9bc();
}
}};
dojo.lfx.toggle.fade={show:function(node,_9be,_9bf,_9c0){
dojo.lfx.fadeShow(node,_9be,_9bf,_9c0).play();
},hide:function(node,_9c2,_9c3,_9c4){
dojo.lfx.fadeHide(node,_9c2,_9c3,_9c4).play();
}};
dojo.lfx.toggle.wipe={show:function(node,_9c6,_9c7,_9c8){
dojo.lfx.wipeIn(node,_9c6,_9c7,_9c8).play();
},hide:function(node,_9ca,_9cb,_9cc){
dojo.lfx.wipeOut(node,_9ca,_9cb,_9cc).play();
}};
dojo.lfx.toggle.explode={show:function(node,_9ce,_9cf,_9d0,_9d1){
dojo.lfx.explode(_9d1||[0,0,0,0],node,_9ce,_9cf,_9d0).play();
},hide:function(node,_9d3,_9d4,_9d5,_9d6){
dojo.lfx.implode(node,_9d6||[0,0,0,0],_9d3,_9d4,_9d5).play();
}};
dojo.provide("dojo.widget.HtmlWidget");
dojo.require("dojo.widget.DomWidget");
dojo.require("dojo.html");
dojo.require("dojo.html.extras");
dojo.require("dojo.lang.extras");
dojo.require("dojo.lang.func");
dojo.require("dojo.lfx.toggle");
dojo.declare("dojo.widget.HtmlWidget",dojo.widget.DomWidget,{widgetType:"HtmlWidget",templateCssPath:null,templatePath:null,toggle:"plain",toggleDuration:150,animationInProgress:false,initialize:function(args,frag){
},postMixInProperties:function(args,frag){
this.toggleObj=dojo.lfx.toggle[this.toggle.toLowerCase()]||dojo.lfx.toggle.plain;
},getContainerHeight:function(){
dojo.unimplemented("dojo.widget.HtmlWidget.getContainerHeight");
},getContainerWidth:function(){
return this.parent.domNode.offsetWidth;
},setNativeHeight:function(_9db){
var ch=this.getContainerHeight();
},createNodesFromText:function(txt,wrap){
return dojo.html.createNodesFromText(txt,wrap);
},destroyRendering:function(_9df){
try{
if(!_9df){
dojo.event.browser.clean(this.domNode);
}
this.domNode.parentNode.removeChild(this.domNode);
delete this.domNode;
}
catch(e){
}
},isShowing:function(){
return dojo.style.isShowing(this.domNode);
},toggleShowing:function(){
if(this.isHidden){
this.show();
}else{
this.hide();
}
},show:function(){
this.animationInProgress=true;
this.isHidden=false;
this.toggleObj.show(this.domNode,this.toggleDuration,null,dojo.lang.hitch(this,this.onShow),this.explodeSrc);
},onShow:function(){
this.animationInProgress=false;
this.checkSize();
},hide:function(){
this.animationInProgress=true;
this.isHidden=true;
this.toggleObj.hide(this.domNode,this.toggleDuration,null,dojo.lang.hitch(this,this.onHide),this.explodeSrc);
},onHide:function(){
this.animationInProgress=false;
},_isResized:function(w,h){
if(!this.isShowing()){
return false;
}
w=w||dojo.style.getOuterWidth(this.domNode);
h=h||dojo.style.getOuterHeight(this.domNode);
if(this.width==w&&this.height==h){
return false;
}
this.width=w;
this.height=h;
return true;
},checkSize:function(){
if(!this._isResized()){
return;
}
this.onResized();
},resizeTo:function(w,h){
if(!this._isResized(w,h)){
return;
}
dojo.style.setOuterWidth(this.domNode,w);
dojo.style.setOuterHeight(this.domNode,h);
this.onResized();
},resizeSoon:function(){
if(this.isShowing()){
dojo.lang.setTimeout(this,this.onResized,0);
}
},onResized:function(){
dojo.lang.forEach(this.children,function(_9e4){
_9e4.checkSize();
});
}});
dojo.kwCompoundRequire({common:["dojo.xml.Parse","dojo.widget.Widget","dojo.widget.Parse","dojo.widget.Manager"],browser:["dojo.widget.DomWidget","dojo.widget.HtmlWidget"],dashboard:["dojo.widget.DomWidget","dojo.widget.HtmlWidget"],svg:["dojo.widget.SvgWidget"],rhino:["dojo.widget.SwtWidget"]});
dojo.provide("dojo.widget.*");
dojo.provide("dojo.math");
dojo.math.degToRad=function(x){
return (x*Math.PI)/180;
};
dojo.math.radToDeg=function(x){
return (x*180)/Math.PI;
};
dojo.math.factorial=function(n){
if(n<1){
return 0;
}
var _9e8=1;
for(var i=1;i<=n;i++){
_9e8*=i;
}
return _9e8;
};
dojo.math.permutations=function(n,k){
if(n==0||k==0){
return 1;
}
return (dojo.math.factorial(n)/dojo.math.factorial(n-k));
};
dojo.math.combinations=function(n,r){
if(n==0||r==0){
return 1;
}
return (dojo.math.factorial(n)/(dojo.math.factorial(n-r)*dojo.math.factorial(r)));
};
dojo.math.bernstein=function(t,n,i){
return (dojo.math.combinations(n,i)*Math.pow(t,i)*Math.pow(1-t,n-i));
};
dojo.math.gaussianRandom=function(){
var k=2;
do{
var i=2*Math.random()-1;
var j=2*Math.random()-1;
k=i*i+j*j;
}while(k>=1);
k=Math.sqrt((-2*Math.log(k))/k);
return i*k;
};
dojo.math.mean=function(){
var _9f4=dojo.lang.isArray(arguments[0])?arguments[0]:arguments;
var mean=0;
for(var i=0;i<_9f4.length;i++){
mean+=_9f4[i];
}
return mean/_9f4.length;
};
dojo.math.round=function(_9f7,_9f8){
if(!_9f8){
var _9f9=1;
}else{
var _9f9=Math.pow(10,_9f8);
}
return Math.round(_9f7*_9f9)/_9f9;
};
dojo.math.sd=function(){
var _9fa=dojo.lang.isArray(arguments[0])?arguments[0]:arguments;
return Math.sqrt(dojo.math.variance(_9fa));
};
dojo.math.variance=function(){
var _9fb=dojo.lang.isArray(arguments[0])?arguments[0]:arguments;
var mean=0,squares=0;
for(var i=0;i<_9fb.length;i++){
mean+=_9fb[i];
squares+=Math.pow(_9fb[i],2);
}
return (squares/_9fb.length)-Math.pow(mean/_9fb.length,2);
};
dojo.math.range=function(a,b,step){
if(arguments.length<2){
b=a;
a=0;
}
if(arguments.length<3){
step=1;
}
var _a01=[];
if(step>0){
for(var i=a;i<b;i+=step){
_a01.push(i);
}
}else{
if(step<0){
for(var i=a;i>b;i+=step){
_a01.push(i);
}
}else{
throw new Error("dojo.math.range: step must be non-zero");
}
}
return _a01;
};
dojo.provide("dojo.math.curves");
dojo.require("dojo.math");
dojo.math.curves={Line:function(_a03,end){
this.start=_a03;
this.end=end;
this.dimensions=_a03.length;
for(var i=0;i<_a03.length;i++){
_a03[i]=Number(_a03[i]);
}
for(var i=0;i<end.length;i++){
end[i]=Number(end[i]);
}
this.getValue=function(n){
var _a07=new Array(this.dimensions);
for(var i=0;i<this.dimensions;i++){
_a07[i]=((this.end[i]-this.start[i])*n)+this.start[i];
}
return _a07;
};
return this;
},Bezier:function(pnts){
this.getValue=function(step){
if(step>=1){
return this.p[this.p.length-1];
}
if(step<=0){
return this.p[0];
}
var _a0b=new Array(this.p[0].length);
for(var k=0;j<this.p[0].length;k++){
_a0b[k]=0;
}
for(var j=0;j<this.p[0].length;j++){
var C=0;
var D=0;
for(var i=0;i<this.p.length;i++){
C+=this.p[i][j]*this.p[this.p.length-1][0]*dojo.math.bernstein(step,this.p.length,i);
}
for(var l=0;l<this.p.length;l++){
D+=this.p[this.p.length-1][0]*dojo.math.bernstein(step,this.p.length,l);
}
_a0b[j]=C/D;
}
return _a0b;
};
this.p=pnts;
return this;
},CatmullRom:function(pnts,c){
this.getValue=function(step){
var _a15=step*(this.p.length-1);
var node=Math.floor(_a15);
var _a17=_a15-node;
var i0=node-1;
if(i0<0){
i0=0;
}
var i=node;
var i1=node+1;
if(i1>=this.p.length){
i1=this.p.length-1;
}
var i2=node+2;
if(i2>=this.p.length){
i2=this.p.length-1;
}
var u=_a17;
var u2=_a17*_a17;
var u3=_a17*_a17*_a17;
var _a1f=new Array(this.p[0].length);
for(var k=0;k<this.p[0].length;k++){
var x1=(-this.c*this.p[i0][k])+((2-this.c)*this.p[i][k])+((this.c-2)*this.p[i1][k])+(this.c*this.p[i2][k]);
var x2=(2*this.c*this.p[i0][k])+((this.c-3)*this.p[i][k])+((3-2*this.c)*this.p[i1][k])+(-this.c*this.p[i2][k]);
var x3=(-this.c*this.p[i0][k])+(this.c*this.p[i1][k]);
var x4=this.p[i][k];
_a1f[k]=x1*u3+x2*u2+x3*u+x4;
}
return _a1f;
};
if(!c){
this.c=0.7;
}else{
this.c=c;
}
this.p=pnts;
return this;
},Arc:function(_a25,end,ccw){
var _a28=dojo.math.points.midpoint(_a25,end);
var _a29=dojo.math.points.translate(dojo.math.points.invert(_a28),_a25);
var rad=Math.sqrt(Math.pow(_a29[0],2)+Math.pow(_a29[1],2));
var _a2b=dojo.math.radToDeg(Math.atan(_a29[1]/_a29[0]));
if(_a29[0]<0){
_a2b-=90;
}else{
_a2b+=90;
}
dojo.math.curves.CenteredArc.call(this,_a28,rad,_a2b,_a2b+(ccw?-180:180));
},CenteredArc:function(_a2c,_a2d,_a2e,end){
this.center=_a2c;
this.radius=_a2d;
this.start=_a2e||0;
this.end=end;
this.getValue=function(n){
var _a31=new Array(2);
var _a32=dojo.math.degToRad(this.start+((this.end-this.start)*n));
_a31[0]=this.center[0]+this.radius*Math.sin(_a32);
_a31[1]=this.center[1]-this.radius*Math.cos(_a32);
return _a31;
};
return this;
},Circle:function(_a33,_a34){
dojo.math.curves.CenteredArc.call(this,_a33,_a34,0,360);
return this;
},Path:function(){
var _a35=[];
var _a36=[];
var _a37=[];
var _a38=0;
this.add=function(_a39,_a3a){
if(_a3a<0){
dojo.raise("dojo.math.curves.Path.add: weight cannot be less than 0");
}
_a35.push(_a39);
_a36.push(_a3a);
_a38+=_a3a;
computeRanges();
};
this.remove=function(_a3b){
for(var i=0;i<_a35.length;i++){
if(_a35[i]==_a3b){
_a35.splice(i,1);
_a38-=_a36.splice(i,1)[0];
break;
}
}
computeRanges();
};
this.removeAll=function(){
_a35=[];
_a36=[];
_a38=0;
};
this.getValue=function(n){
var _a3e=false,value=0;
for(var i=0;i<_a37.length;i++){
var r=_a37[i];
if(n>=r[0]&&n<r[1]){
var subN=(n-r[0])/r[2];
value=_a35[i].getValue(subN);
_a3e=true;
break;
}
}
if(!_a3e){
value=_a35[_a35.length-1].getValue(1);
}
for(var j=0;j<i;j++){
value=dojo.math.points.translate(value,_a35[j].getValue(1));
}
return value;
};
function computeRanges(){
var _a43=0;
for(var i=0;i<_a36.length;i++){
var end=_a43+_a36[i]/_a38;
var len=end-_a43;
_a37[i]=[_a43,end,len];
_a43=end;
}
}
return this;
}};
dojo.provide("dojo.math.points");
dojo.require("dojo.math");
dojo.math.points={translate:function(a,b){
if(a.length!=b.length){
dojo.raise("dojo.math.translate: points not same size (a:["+a+"], b:["+b+"])");
}
var c=new Array(a.length);
for(var i=0;i<a.length;i++){
c[i]=a[i]+b[i];
}
return c;
},midpoint:function(a,b){
if(a.length!=b.length){
dojo.raise("dojo.math.midpoint: points not same size (a:["+a+"], b:["+b+"])");
}
var c=new Array(a.length);
for(var i=0;i<a.length;i++){
c[i]=(a[i]+b[i])/2;
}
return c;
},invert:function(a){
var b=new Array(a.length);
for(var i=0;i<a.length;i++){
b[i]=-a[i];
}
return b;
},distance:function(a,b){
return Math.sqrt(Math.pow(b[0]-a[0],2)+Math.pow(b[1]-a[1],2));
}};
dojo.kwCompoundRequire({common:[["dojo.math",false,false],["dojo.math.curves",false,false],["dojo.math.points",false,false]]});
dojo.provide("dojo.math.*");

