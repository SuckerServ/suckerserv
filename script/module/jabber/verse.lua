package.preload['util.encodings']=(function(...)
local function e()
error("Function not implemented");
end
local t=require"mime";
module"encodings"
stringprep={};
base64={encode=t.b64,decode=e};
return _M;
end)
package.preload['util.hashes']=(function(...)
local e=require"util.sha1";
return{sha1=e.sha1};
end)
package.preload['util.logger']=(function(...)
local s,t=select,tostring;
local h=io.write;
module"logger"
local function e(o,...)
local e,a=0,#arg;
return(o:gsub("%%(.)",function(o)if o~="%"and e<=a then e=e+1;return t(arg[e]);end end));
end
local function n(i,...)
local e,o=0,s('#',...);
local a={...};
return(i:gsub("%%(.)",function(i)if e<=o then e=e+1;return t(a[e]);end end));
end
function init(e)
return function(e,t,...)
h(e,"\t",n(t,...),"\n");
end
end
return _M;
end)
package.preload['util.sha1']=(function(...)
local m=string.len
local a=string.char
local b=string.byte
local g=string.sub
local c=math.floor
local t=require"bit"
local k=t.bnot
local e=t.band
local y=t.bor
local n=t.bxor
local i=t.lshift
local o=t.rshift
local l,u,d,h,s
local function p(e,t)
return i(e,t)+o(e,32-t)
end
local function r(i)
local t,o
local t=""
for n=1,8 do
o=e(i,15)
if(o<10)then
t=a(o+48)..t
else
t=a(o+87)..t
end
i=c(i/16)
end
return t
end
local function q(t)
local i,o
local n=""
i=m(t)*8
t=t..a(128)
o=56-e(m(t),63)
if(o<0)then
o=o+64
end
for e=1,o do
t=t..a(0)
end
for t=1,8 do
n=a(e(i,255))..n
i=c(i/256)
end
return t..n
end
local function j(f)
local m,t,a,o,w,r,c,v
local i,i
local i={}
while(f~="")do
for e=0,15 do
i[e]=0
for t=1,4 do
i[e]=i[e]*256+b(f,e*4+t)
end
end
for e=16,79 do
i[e]=p(n(n(i[e-3],i[e-8]),n(i[e-14],i[e-16])),1)
end
m=l
t=u
a=d
o=h
w=s
for s=0,79 do
if(s<20)then
r=y(e(t,a),e(k(t),o))
c=1518500249
elseif(s<40)then
r=n(n(t,a),o)
c=1859775393
elseif(s<60)then
r=y(y(e(t,a),e(t,o)),e(a,o))
c=2400959708
else
r=n(n(t,a),o)
c=3395469782
end
v=p(m,5)+r+w+c+i[s]
w=o
o=a
a=p(t,30)
t=m
m=v
end
l=e(l+m,4294967295)
u=e(u+t,4294967295)
d=e(d+a,4294967295)
h=e(h+o,4294967295)
s=e(s+w,4294967295)
f=g(f,65)
end
end
local function a(e,t)
e=q(e)
l=1732584193
u=4023233417
d=2562383102
h=271733878
s=3285377520
j(e)
local e=r(l)..r(u)..r(d)
..r(h)..r(s);
if t then
return e;
else
return(e:gsub("..",function(e)
return string.char(tonumber(e,16));
end));
end
end
_G.sha1={sha1=a};
return _G.sha1;
end)
package.preload['lib.adhoc']=(function(...)
local n,r=require"util.stanza",require"util.uuid";
local h="http://jabber.org/protocol/commands";
local i={}
local s={};
function _cmdtag(o,e,t,a)
local e=n.stanza("command",{xmlns=h,node=o.node,status=e});
if t then e.attr.sessionid=t;end
if a then e.attr.action=a;end
return e;
end
function s.new(t,o,a,e)
return{name=t,node=o,handler=a,cmdtag=_cmdtag,permission=(e or"user")};
end
function s.handle_cmd(a,s,o)
local e=o.tags[1].attr.sessionid or r.generate();
local t={};
t.to=o.attr.to;
t.from=o.attr.from;
t.action=o.tags[1].attr.action or"execute";
t.form=o.tags[1]:child_with_ns("jabber:x:data");
local t,h=a:handler(t,i[e]);
i[e]=h;
local o=n.reply(o);
if t.status=="completed"then
i[e]=nil;
cmdtag=a:cmdtag("completed",e);
elseif t.status=="canceled"then
i[e]=nil;
cmdtag=a:cmdtag("canceled",e);
elseif t.status=="error"then
i[e]=nil;
o=n.error_reply(o,t.error.type,t.error.condition,t.error.message);
s.send(o);
return true;
else
cmdtag=a:cmdtag("executing",e);
end
for t,e in pairs(t)do
if t=="info"then
cmdtag:tag("note",{type="info"}):text(e):up();
elseif t=="warn"then
cmdtag:tag("note",{type="warn"}):text(e):up();
elseif t=="error"then
cmdtag:tag("note",{type="error"}):text(e.message):up();
elseif t=="actions"then
local t=n.stanza("actions");
for o,e in ipairs(e)do
if(e=="prev")or(e=="next")or(e=="complete")then
t:tag(e):up();
else
module:log("error",'Command "'..a.name..
'" at node "'..a.node..'" provided an invalid action "'..e..'"');
end
end
cmdtag:add_child(t);
elseif t=="form"then
cmdtag:add_child((e.layout or e):form(e.values));
elseif t=="result"then
cmdtag:add_child((e.layout or e):form(e.values,"result"));
elseif t=="other"then
cmdtag:add_child(e);
end
end
o:add_child(cmdtag);
s.send(o);
return true;
end
return s;
end)
package.preload['util.stanza']=(function(...)
local t=table.insert;
local e=table.concat;
local d=table.remove;
local p=table.concat;
local s=string.format;
local y=string.match;
local u=tostring;
local l=setmetatable;
local e=getmetatable;
local i=pairs;
local n=ipairs;
local o=type;
local e=next;
local e=print;
local e=unpack;
local f=string.gsub;
local e=string.char;
local c=string.find;
local e=os;
local m=not e.getenv("WINDIR");
local r,a;
if m then
local t,e=pcall(require,"util.termcolours");
if t then
r,a=e.getstyle,e.getstring;
else
m=nil;
end
end
local w="urn:ietf:params:xml:ns:xmpp-stanzas";
module"stanza"
stanza_mt={__type="stanza"};
stanza_mt.__index=stanza_mt;
local e=stanza_mt;
function stanza(t,a)
local t={name=t,attr=a or{},tags={}};
return l(t,e);
end
local h=stanza;
function e:query(e)
return self:tag("query",{xmlns=e});
end
function e:body(t,e)
return self:tag("body",e):text(t);
end
function e:tag(a,e)
local a=h(a,e);
local e=self.last_add;
if not e then e={};self.last_add=e;end
(e[#e]or self):add_direct_child(a);
t(e,a);
return self;
end
function e:text(t)
local e=self.last_add;
(e and e[#e]or self):add_direct_child(t);
return self;
end
function e:up()
local e=self.last_add;
if e then d(e);end
return self;
end
function e:reset()
self.last_add=nil;
return self;
end
function e:add_direct_child(e)
if o(e)=="table"then
t(self.tags,e);
end
t(self,e);
end
function e:add_child(t)
local e=self.last_add;
(e and e[#e]or self):add_direct_child(t);
return self;
end
function e:get_child(t,a)
for o,e in n(self.tags)do
if(not t or e.name==t)
and((not a and self.attr.xmlns==e.attr.xmlns)
or e.attr.xmlns==a)then
return e;
end
end
end
function e:get_child_text(e,t)
local e=self:get_child(e,t);
if e then
return e:get_text();
end
return nil;
end
function e:child_with_name(t)
for a,e in n(self.tags)do
if e.name==t then return e;end
end
end
function e:child_with_ns(t)
for a,e in n(self.tags)do
if e.attr.xmlns==t then return e;end
end
end
function e:children()
local e=0;
return function(t)
e=e+1
return t[e];
end,self,e;
end
function e:childtags(a,e)
e=e or self.attr.xmlns;
local t=self.tags;
local o,i=1,#t;
return function()
for i=o,i do
local t=t[i];
if(not a or t.name==a)
and(not e or e==t.attr.xmlns)then
o=i+1;
return t;
end
end
end;
end
function e:maptags(i)
local a,t=self.tags,1;
local n,o=#self,#a;
local e=1;
while t<=o do
if self[e]==a[t]then
local i=i(self[e]);
if i==nil then
d(self,e);
d(a,t);
n=n-1;
o=o-1;
else
self[e]=i;
a[e]=i;
end
e=e+1;
t=t+1;
end
end
return self;
end
local d
do
local t={["'"]="&apos;",["\""]="&quot;",["<"]="&lt;",[">"]="&gt;",["&"]="&amp;"};
function d(e)return(f(e,"['&<>\"]",t));end
_M.xml_escape=d;
end
local function f(a,e,h,o,r)
local n=0;
local s=a.name
t(e,"<"..s);
for a,i in i(a.attr)do
if c(a,"\1",1,true)then
local a,s=y(a,"^([^\1]*)\1?(.*)$");
n=n+1;
t(e," xmlns:ns"..n.."='"..o(a).."' ".."ns"..n..":"..s.."='"..o(i).."'");
elseif not(a=="xmlns"and i==r)then
t(e," "..a.."='"..o(i).."'");
end
end
local i=#a;
if i==0 then
t(e,"/>");
else
t(e,">");
for i=1,i do
local i=a[i];
if i.name then
h(i,e,h,o,a.attr.xmlns);
else
t(e,o(i));
end
end
t(e,"</"..s..">");
end
end
function e.__tostring(t)
local e={};
f(t,e,f,d,nil);
return p(e);
end
function e.top_tag(t)
local e="";
if t.attr then
for t,a in i(t.attr)do if o(t)=="string"then e=e..s(" %s='%s'",t,d(u(a)));end end
end
return s("<%s%s>",t.name,e);
end
function e.get_text(e)
if#e.tags==0 then
return p(e);
end
end
function e.get_error(t)
local o,e,a;
local t=t:get_child("error");
if not t then
return nil,nil,nil;
end
o=t.attr.type;
for t in t:childtags()do
if t.attr.xmlns==w then
if not a and t.name=="text"then
a=t:get_text();
elseif not e then
e=t.name;
end
if e and a then
break;
end
end
end
return o,e or"undefined-condition",a;
end
function e.__add(t,e)
return t:add_direct_child(e);
end
do
local e=0;
function new_id()
e=e+1;
return"lx"..e;
end
end
function preserialize(e)
local a={name=e.name,attr=e.attr};
for i,e in n(e)do
if o(e)=="table"then
t(a,preserialize(e));
else
t(a,e);
end
end
return a;
end
function deserialize(a)
if a then
local s=a.attr;
for e=1,#s do s[e]=nil;end
local h={};
for e in i(s)do
if c(e,"|",1,true)and not c(e,"\1",1,true)then
local t,a=y(e,"^([^|]+)|(.+)$");
h[t.."\1"..a]=s[e];
s[e]=nil;
end
end
for t,e in i(h)do
s[t]=e;
end
l(a,e);
for t,e in n(a)do
if o(e)=="table"then
deserialize(e);
end
end
if not a.tags then
local e={};
for n,i in n(a)do
if o(i)=="table"then
t(e,i);
end
end
a.tags=e;
end
end
return a;
end
local function c(a)
local n,o={},{};
for e,t in i(a.attr)do n[e]=t;end
local i={name=a.name,attr=n,tags=o};
for e=1,#a do
local e=a[e];
if e.name then
e=c(e);
t(o,e);
end
t(i,e);
end
return l(i,e);
end
clone=c;
function message(t,e)
if not e then
return h("message",t);
else
return h("message",t):tag("body"):text(e):up();
end
end
function iq(e)
if e and not e.id then e.id=new_id();end
return h("iq",e or{id=new_id()});
end
function reply(e)
return h(e.name,e.attr and{to=e.attr.from,from=e.attr.to,id=e.attr.id,type=((e.name=="iq"and"result")or e.attr.type)});
end
do
local t={xmlns=w};
function error_reply(e,i,o,a)
local e=reply(e);
e.attr.type="error";
e:tag("error",{type=i})
:tag(o,t):up();
if(a)then e:tag("text",t):text(a):up();end
return e;
end
end
function presence(e)
return h("presence",e);
end
if m then
local c=r("yellow");
local l=r("red");
local h=r("red");
local t=r("magenta");
local r=" "..a(c,"%s")..a(t,"=")..a(l,"'%s'");
local l=a(t,"<")..a(h,"%s").."%s"..a(t,">");
local h=l.."%s"..a(t,"</")..a(h,"%s")..a(t,">");
function e.pretty_print(t)
local e="";
for a,t in n(t)do
if o(t)=="string"then
e=e..d(t);
else
e=e..t:pretty_print();
end
end
local a="";
if t.attr then
for e,t in i(t.attr)do if o(e)=="string"then a=a..s(r,e,u(t));end end
end
return s(h,t.name,a,e,t.name);
end
function e.pretty_top_tag(t)
local e="";
if t.attr then
for t,a in i(t.attr)do if o(t)=="string"then e=e..s(r,t,u(a));end end
end
return s(l,t.name,e);
end
else
e.pretty_print=e.__tostring;
e.pretty_top_tag=e.top_tag;
end
return _M;
end)
package.preload['util.timer']=(function(...)
local d=require"net.server".addtimer;
local i=require"net.server".event;
local l=require"net.server".event_base;
local r=math.min
local u=math.huge
local h=require"socket".gettime;
local s=table.insert;
local e=table.remove;
local e,n=ipairs,pairs;
local c=type;
local a={};
local e={};
module"timer"
local t;
if not i then
function t(t,a)
local o=h();
t=t+o;
if t>=o then
s(e,{t,a});
else
a();
end
end
d(function()
local o=h();
if#e>0 then
for o,t in n(e)do
s(a,t);
end
e={};
end
local e=u;
for s,i in n(a)do
local i,n=i[1],i[2];
if i<=o then
a[s]=nil;
local a=n(o);
if c(a)=="number"then
t(a,n);
e=r(e,a);
end
else
e=r(e,i-o);
end
end
return e;
end);
else
local o=(i.core and i.core.LEAVE)or-1;
function t(a,t)
local e;
e=l:addevent(nil,0,function()
local t=t();
if t then
return 0,t;
elseif e then
return o;
end
end
,a);
end
end
add_task=t;
return _M;
end)
package.preload['util.termcolours']=(function(...)
local h,s=table.concat,table.insert;
local t,r=string.char,string.format;
local l=ipairs;
local d=io.write;
local e;
if os.getenv("WINDIR")then
e=require"util.windows";
end
local o=e and e.get_consolecolor and e.get_consolecolor();
module"termcolours"
local i={
reset=0;bright=1,dim=2,underscore=4,blink=5,reverse=7,hidden=8;
black=30;red=31;green=32;yellow=33;blue=34;magenta=35;cyan=36;white=37;
["black background"]=40;["red background"]=41;["green background"]=42;["yellow background"]=43;["blue background"]=44;["magenta background"]=45;["cyan background"]=46;["white background"]=47;
bold=1,dark=2,underline=4,underlined=4,normal=0;
}
local n={
["0"]=o,
["1"]=7+8,
["1;33"]=2+4+8,
["1;31"]=4+8
}
local a=t(27).."[%sm%s"..t(27).."[0m";
function getstring(t,e)
if t then
return r(a,t,e);
else
return e;
end
end
function getstyle(...)
local e,t={...},{};
for a,e in l(e)do
e=i[e];
if e then
s(t,e);
end
end
return h(t,";");
end
local a="0";
function setstyle(e)
e=e or"0";
if e~=a then
d("\27["..e.."m");
a=e;
end
end
if e then
function setstyle(t)
t=t or"0";
if t~=a then
e.set_consolecolor(n[t]or o);
a=t;
end
end
if not o then
function setstyle(e)end
end
end
return _M;
end)
package.preload['util.uuid']=(function(...)
local e=math.random;
local a=tostring;
local e=os.time;
local i=os.clock;
local n=require"util.hashes".sha1;
module"uuid"
local t=0;
local function o()
local e=e();
if t>=e then e=t+1;end
t=e;
return e;
end
local function e(e)
return n(e..i()..a({}),true);
end
local t=e(o());
local function a(a)
t=e(t..a);
end
local function e(e)
if#t<e then a(o());end
local a=t:sub(0,e);
t=t:sub(e+1);
return a;
end
local function t()
return("%x"):format(e(1):byte()%4+8);
end
function generate()
return e(8).."-"..e(4).."-4"..e(3).."-"..(t())..e(3).."-"..e(12);
end
seed=a;
return _M;
end)
package.preload['net.dns']=(function(...)
local n=require"socket";
local j=require"util.timer";
local e,f=pcall(require,"util.windows");
local E=(e and f)or os.getenv("WINDIR");
local c,_,y,a,r=
coroutine,io,math,string,table;
local w,s,o,u,h,p,k,x,t,e,q=
ipairs,next,pairs,print,setmetatable,tostring,assert,error,unpack,select,type;
local e={
get=function(t,...)
local a=e('#',...);
for a=1,a do
t=t[e(a,...)];
if t==nil then break;end
end
return t;
end;
set=function(a,...)
local n=e('#',...);
local h,o=e(n-1,...);
local t,i;
for n=1,n-2 do
local n=e(n,...)
local e=a[n]
if o==nil then
if e==nil then
return;
elseif s(e,s(e))then
t=nil;i=nil;
elseif t==nil then
t=a;i=n;
end
elseif e==nil then
e={};
a[n]=e;
end
a=e
end
if o==nil and t then
t[i]=nil;
else
a[h]=o;
return o;
end
end;
};
local l,d=e.get,e.set;
local z=15;
module('dns')
local t=_M;
local i=r.insert
local function m(e)
return(e-(e%256))/256;
end
local function b(e)
local t={};
for o,e in o(e)do
t[o]=e;
t[e]=e;
t[a.lower(e)]=e;
end
return t;
end
local function v(i)
local e={};
for o,i in o(i)do
local t=a.char(m(o),o%256);
e[o]=t;
e[i]=t;
e[a.lower(i)]=t;
end
return e;
end
t.types={
'A','NS','MD','MF','CNAME','SOA','MB','MG','MR','NULL','WKS',
'PTR','HINFO','MINFO','MX','TXT',
[28]='AAAA',[29]='LOC',[33]='SRV',
[252]='AXFR',[253]='MAILB',[254]='MAILA',[255]='*'};
t.classes={'IN','CS','CH','HS',[255]='*'};
t.type=b(t.types);
t.class=b(t.classes);
t.typecode=v(t.types);
t.classcode=v(t.classes);
local function g(e,i,o)
if a.byte(e,-1)~=46 then e=e..'.';end
e=a.lower(e);
return e,t.type[i or'A'],t.class[o or'IN'];
end
local function v(t,a,i)
a=a or n.gettime();
for o,e in o(t)do
if e.tod then
e.ttl=y.floor(e.tod-a);
if e.ttl<=0 then
r.remove(t,o);
return v(t,a,i);
end
elseif i=='soft'then
k(e.ttl==0);
t[o]=nil;
end
end
end
local e={};
e.__index=e;
e.timeout=z;
local function z(e)
local e=e.type and e[e.type:lower()];
if q(e)~="string"then
return"<UNKNOWN RDATA TYPE>";
end
return e;
end
local b={
LOC=e.LOC_tostring;
MX=function(e)
return a.format('%2i %s',e.pref,e.mx);
end;
SRV=function(e)
local e=e.srv;
return a.format('%5d %5d %5d %s',e.priority,e.weight,e.port,e.target);
end;
};
local k={};
function k.__tostring(e)
local t=(b[e.type]or z)(e);
return a.format('%2s %-5s %6i %-28s %s',e.class,e.type,e.ttl,e.name,t);
end
local q={};
function q.__tostring(t)
local e={};
for a,t in o(t)do
i(e,p(t)..'\n');
end
return r.concat(e);
end
local b={};
function b.__tostring(t)
local a=n.gettime();
local e={};
for n,t in o(t)do
for n,t in o(t)do
for o,t in o(t)do
v(t,a);
i(e,p(t));
end
end
end
return r.concat(e);
end
function e:new()
local t={active={},cache={},unsorted={}};
h(t,e);
h(t.cache,b);
h(t.unsorted,{__mode='kv'});
return t;
end
function t.random(...)
y.randomseed(y.floor(1e4*n.gettime()));
t.random=y.random;
return t.random(...);
end
local function z(e)
e=e or{};
e.id=e.id or t.random(0,65535);
e.rd=e.rd or 1;
e.tc=e.tc or 0;
e.aa=e.aa or 0;
e.opcode=e.opcode or 0;
e.qr=e.qr or 0;
e.rcode=e.rcode or 0;
e.z=e.z or 0;
e.ra=e.ra or 0;
e.qdcount=e.qdcount or 1;
e.ancount=e.ancount or 0;
e.nscount=e.nscount or 0;
e.arcount=e.arcount or 0;
local t=a.char(
m(e.id),e.id%256,
e.rd+2*e.tc+4*e.aa+8*e.opcode+128*e.qr,
e.rcode+16*e.z+128*e.ra,
m(e.qdcount),e.qdcount%256,
m(e.ancount),e.ancount%256,
m(e.nscount),e.nscount%256,
m(e.arcount),e.arcount%256
);
return t,e.id;
end
local function m(t)
local e={};
for t in a.gmatch(t,'[^.]+')do
i(e,a.char(a.len(t)));
i(e,t);
end
i(e,a.char(0));
return r.concat(e);
end
local function y(o,e,a)
o=m(o);
e=t.typecode[e or'a'];
a=t.classcode[a or'in'];
return o..e..a;
end
function e:byte(e)
e=e or 1;
local o=self.offset;
local t=o+e-1;
if t>#self.packet then
x(a.format('out of bounds: %i>%i',t,#self.packet));
end
self.offset=o+e;
return a.byte(self.packet,o,t);
end
function e:word()
local t,e=self:byte(2);
return 256*t+e;
end
function e:dword()
local o,a,t,e=self:byte(4);
return 16777216*o+65536*a+256*t+e;
end
function e:sub(e)
e=e or 1;
local t=a.sub(self.packet,self.offset,self.offset+e-1);
self.offset=self.offset+e;
return t;
end
function e:header(t)
local e=self:word();
if not self.active[e]and not t then return nil;end
local e={id=e};
local t,a=self:byte(2);
e.rd=t%2;
e.tc=t/2%2;
e.aa=t/4%2;
e.opcode=t/8%16;
e.qr=t/128;
e.rcode=a%16;
e.z=a/16%8;
e.ra=a/128;
e.qdcount=self:word();
e.ancount=self:word();
e.nscount=self:word();
e.arcount=self:word();
for a,t in o(e)do e[a]=t-t%1;end
return e;
end
function e:name()
local t,a=nil,0;
local e=self:byte();
local o={};
while e>0 do
if e>=192 then
a=a+1;
if a>=20 then x('dns error: 20 pointers');end;
local e=((e-192)*256)+self:byte();
t=t or self.offset;
self.offset=e+1;
else
i(o,self:sub(e)..'.');
end
e=self:byte();
end
self.offset=t or self.offset;
return r.concat(o);
end
function e:question()
local e={};
e.name=self:name();
e.type=t.type[self:word()];
e.class=t.class[self:word()];
return e;
end
function e:A(t)
local e,o,i,n=self:byte(4);
t.a=a.format('%i.%i.%i.%i',e,o,i,n);
end
function e:CNAME(e)
e.cname=self:name();
end
function e:MX(e)
e.pref=self:word();
e.mx=self:name();
end
function e:LOC_nibble_power()
local e=self:byte();
return((e-(e%16))/16)*(10^(e%16));
end
function e:LOC(e)
e.version=self:byte();
if e.version==0 then
e.loc=e.loc or{};
e.loc.size=self:LOC_nibble_power();
e.loc.horiz_pre=self:LOC_nibble_power();
e.loc.vert_pre=self:LOC_nibble_power();
e.loc.latitude=self:dword();
e.loc.longitude=self:dword();
e.loc.altitude=self:dword();
end
end
local function m(e,i,t)
e=e-2147483648;
if e<0 then i=t;e=-e;end
local n,o,t;
t=e%6e4;
e=(e-t)/6e4;
o=e%60;
n=(e-o)/60;
return a.format('%3d %2d %2.3f %s',n,o,t/1e3,i);
end
function e.LOC_tostring(e)
local t={};
i(t,a.format(
'%s    %s    %.2fm %.2fm %.2fm %.2fm',
m(e.loc.latitude,'N','S'),
m(e.loc.longitude,'E','W'),
(e.loc.altitude-1e7)/100,
e.loc.size/100,
e.loc.horiz_pre/100,
e.loc.vert_pre/100
));
return r.concat(t);
end
function e:NS(e)
e.ns=self:name();
end
function e:SOA(e)
end
function e:SRV(e)
e.srv={};
e.srv.priority=self:word();
e.srv.weight=self:word();
e.srv.port=self:word();
e.srv.target=self:name();
end
function e:PTR(e)
e.ptr=self:name();
end
function e:TXT(e)
e.txt=self:sub(self:byte());
end
function e:rr()
local e={};
h(e,k);
e.name=self:name(self);
e.type=t.type[self:word()]or e.type;
e.class=t.class[self:word()]or e.class;
e.ttl=65536*self:word()+self:word();
e.rdlength=self:word();
if e.ttl<=0 then
e.tod=self.time+30;
else
e.tod=self.time+e.ttl;
end
local a=self.offset;
local t=self[t.type[e.type]];
if t then t(self,e);end
self.offset=a;
e.rdata=self:sub(e.rdlength);
return e;
end
function e:rrs(t)
local e={};
for t=1,t do i(e,self:rr());end
return e;
end
function e:decode(t,o)
self.packet,self.offset=t,1;
local t=self:header(o);
if not t then return nil;end
local t={header=t};
t.question={};
local n=self.offset;
for e=1,t.header.qdcount do
i(t.question,self:question());
end
t.question.raw=a.sub(self.packet,n,self.offset-1);
if not o then
if not self.active[t.header.id]or not self.active[t.header.id][t.question.raw]then
return nil;
end
end
t.answer=self:rrs(t.header.ancount);
t.authority=self:rrs(t.header.nscount);
t.additional=self:rrs(t.header.arcount);
return t;
end
e.delays={1,3};
function e:addnameserver(e)
self.server=self.server or{};
i(self.server,e);
end
function e:setnameserver(e)
self.server={};
self:addnameserver(e);
end
function e:adddefaultnameservers()
if E then
if f and f.get_nameservers then
for t,e in w(f.get_nameservers())do
self:addnameserver(e);
end
end
if not self.server or#self.server==0 then
self:addnameserver("208.67.222.222");
self:addnameserver("208.67.220.220");
end
else
local e=_.open("/etc/resolv.conf");
if e then
for e in e:lines()do
e=e:gsub("#.*$","")
:match('^%s*nameserver%s+(.*)%s*$');
if e then
e:gsub("%f[%d.](%d+%.%d+%.%d+%.%d+)%f[^%d.]",function(e)
self:addnameserver(e)
end);
end
end
end
if not self.server or#self.server==0 then
self:addnameserver("127.0.0.1");
end
end
end
function e:getsocket(t)
self.socket=self.socket or{};
self.socketset=self.socketset or{};
local e=self.socket[t];
if e then return e;end
local a;
e,a=n.udp();
if not e then
return nil,a;
end
if self.socket_wrapper then e=self.socket_wrapper(e,self);end
e:settimeout(0);
e:setsockname('*',0);
e:setpeername(self.server[t],53);
self.socket[t]=e;
self.socketset[e]=t;
return e;
end
function e:voidsocket(e)
if self.socket[e]then
self.socketset[self.socket[e]]=nil;
self.socket[e]=nil;
elseif self.socketset[e]then
self.socket[self.socketset[e]]=nil;
self.socketset[e]=nil;
end
end
function e:socket_wrapper_set(e)
self.socket_wrapper=e;
end
function e:closeall()
for t,e in w(self.socket)do
self.socket[t]=nil;
self.socketset[e]=nil;
e:close();
end
end
function e:remember(t,e)
local a,n,o=g(t.name,t.type,t.class);
if e~='*'then
e=n;
local e=l(self.cache,o,'*',a);
if e then i(e,t);end
end
self.cache=self.cache or h({},b);
local a=l(self.cache,o,e,a)or
d(self.cache,o,e,a,h({},q));
i(a,t);
if e=='MX'then self.unsorted[a]=true;end
end
local function i(e,t)
return(e.pref==t.pref)and(e.mx<t.mx)or(e.pref<t.pref);
end
function e:peek(o,t,a)
o,t,a=g(o,t,a);
local e=l(self.cache,a,t,o);
if not e then return nil;end
if v(e,n.gettime())and t=='*'or not s(e)then
d(self.cache,a,t,o,nil);
return nil;
end
if self.unsorted[e]then r.sort(e,i);end
return e;
end
function e:purge(e)
if e=='soft'then
self.time=n.gettime();
for t,e in o(self.cache or{})do
for t,e in o(e)do
for t,e in o(e)do
v(e,self.time,'soft')
end
end
end
else self.cache={};end
end
function e:query(a,e,t)
a,e,t=g(a,e,t)
if not self.server then self:adddefaultnameservers();end
local s=y(a,e,t);
local o=self:peek(a,e,t);
if o then return o;end
local o,i=z();
local o={
packet=o..s,
server=self.best_server,
delay=1,
retry=n.gettime()+self.delays[1]
};
self.active[i]=self.active[i]or{};
self.active[i][s]=o;
local n=c.running();
if n then
d(self.wanted,t,e,a,n,true);
end
local i,h=self:getsocket(o.server)
if not i then
return nil,h;
end
i:send(o.packet)
if j and self.timeout then
local r=#self.server;
local s=1;
j.add_task(self.timeout,function()
if l(self.wanted,t,e,a,n)then
if s<r then
s=s+1;
self:servfail(i);
o.server=self.best_server;
i,h=self:getsocket(o.server);
if i then
i:send(o.packet);
return self.timeout;
end
end
self:cancel(t,e,a,n,true);
end
end)
end
return true;
end
function e:servfail(e)
local a=self.socketset[e]
self:voidsocket(e);
self.time=n.gettime();
for e,t in o(self.active)do
for o,e in o(t)do
if e.server==a then
e.server=e.server+1
if e.server>#self.server then
e.server=1;
end
e.retries=(e.retries or 0)+1;
if e.retries>=#self.server then
t[o]=nil;
else
local t=self:getsocket(e.server);
if t then t:send(e.packet);end
end
end
end
end
if a==self.best_server then
self.best_server=self.best_server+1;
if self.best_server>#self.server then
self.best_server=1;
end
end
end
function e:settimeout(e)
self.timeout=e;
end
function e:receive(t)
self.time=n.gettime();
t=t or self.socket;
local e;
for a,t in o(t)do
if self.socketset[t]then
local t=t:receive();
if t then
e=self:decode(t);
if e and self.active[e.header.id]
and self.active[e.header.id][e.question.raw]then
for a,t in o(e.answer)do
if t.name:sub(-#e.question[1].name,-1)==e.question[1].name then
self:remember(t,e.question[1].type)
end
end
local t=self.active[e.header.id];
t[e.question.raw]=nil;
if not s(t)then self.active[e.header.id]=nil;end
if not s(self.active)then self:closeall();end
local e=e.question[1];
local t=l(self.wanted,e.class,e.type,e.name);
if t then
for t in o(t)do
d(self.yielded,t,e.class,e.type,e.name,nil);
if c.status(t)=="suspended"then c.resume(t);end
end
d(self.wanted,e.class,e.type,e.name,nil);
end
end
end
end
end
return e;
end
function e:feed(a,t,e)
self.time=n.gettime();
local e=self:decode(t,e);
if e and self.active[e.header.id]
and self.active[e.header.id][e.question.raw]then
for a,t in o(e.answer)do
self:remember(t,e.question[1].type);
end
local t=self.active[e.header.id];
t[e.question.raw]=nil;
if not s(t)then self.active[e.header.id]=nil;end
if not s(self.active)then self:closeall();end
local e=e.question[1];
if e then
local t=l(self.wanted,e.class,e.type,e.name);
if t then
for t in o(t)do
d(self.yielded,t,e.class,e.type,e.name,nil);
if c.status(t)=="suspended"then c.resume(t);end
end
d(self.wanted,e.class,e.type,e.name,nil);
end
end
end
return e;
end
function e:cancel(i,e,o,t,a)
local e=l(self.wanted,i,e,o);
if e then
if a then
c.resume(t);
end
e[t]=nil;
end
end
function e:pulse()
while self:receive()do end
if not s(self.active)then return nil;end
self.time=n.gettime();
for i,t in o(self.active)do
for a,e in o(t)do
if self.time>=e.retry then
e.server=e.server+1;
if e.server>#self.server then
e.server=1;
e.delay=e.delay+1;
end
if e.delay>#self.delays then
t[a]=nil;
if not s(t)then self.active[i]=nil;end
if not s(self.active)then return nil;end
else
local t=self.socket[e.server];
if t then t:send(e.packet);end
e.retry=self.time+self.delays[e.delay];
end
end
end
end
if s(self.active)then return true;end
return nil;
end
function e:lookup(e,t,a)
self:query(e,t,a)
while self:pulse()do
local e={}
for t,a in w(self.socket)do
e[t]=a
end
n.select(e,nil,4)
end
return self:peek(e,t,a);
end
function e:lookupex(o,a,e,t)
return self:peek(a,e,t)or self:query(a,e,t);
end
function e:tohostname(e)
return t.lookup(e:gsub("(%d+)%.(%d+)%.(%d+)%.(%d+)","%4.%3.%2.%1.in-addr.arpa."),"PTR");
end
local i={
qr={[0]='query','response'},
opcode={[0]='query','inverse query','server status request'},
aa={[0]='non-authoritative','authoritative'},
tc={[0]='complete','truncated'},
rd={[0]='recursion not desired','recursion desired'},
ra={[0]='recursion not available','recursion available'},
z={[0]='(reserved)'},
rcode={[0]='no error','format error','server failure','name error','not implemented'},
type=t.type,
class=t.class
};
local function s(t,e)
return(i[e]and i[e][t[e]])or'';
end
function e.print(e)
for o,t in o{'id','qr','opcode','aa','tc','rd','ra','z',
'rcode','qdcount','ancount','nscount','arcount'}do
u(a.format('%-30s','header.'..t),e.header[t],s(e.header,t));
end
for t,e in w(e.question)do
u(a.format('question[%i].name         ',t),e.name);
u(a.format('question[%i].type         ',t),e.type);
u(a.format('question[%i].class        ',t),e.class);
end
local r={name=1,type=1,class=1,ttl=1,rdlength=1,rdata=1};
local t;
for n,i in o({'answer','authority','additional'})do
for h,n in o(e[i])do
for o,e in o({'name','type','class','ttl','rdlength'})do
t=a.format('%s[%i].%s',i,h,e);
u(a.format('%-30s',t),n[e],s(n,e));
end
for e,o in o(n)do
if not r[e]then
t=a.format('%s[%i].%s',i,h,e);
u(a.format('%-30s  %s',p(t),p(o)));
end
end
end
end
end
function t.resolver()
local t={active={},cache={},unsorted={},wanted={},yielded={},best_server=1};
h(t,e);
h(t.cache,b);
h(t.unsorted,{__mode='kv'});
return t;
end
local e=t.resolver();
t._resolver=e;
function t.lookup(...)
return e:lookup(...);
end
function t.tohostname(...)
return e:tohostname(...);
end
function t.purge(...)
return e:purge(...);
end
function t.peek(...)
return e:peek(...);
end
function t.query(...)
return e:query(...);
end
function t.feed(...)
return e:feed(...);
end
function t.cancel(...)
return e:cancel(...);
end
function t.settimeout(...)
return e:settimeout(...);
end
function t.socket_wrapper_set(...)
return e:socket_wrapper_set(...);
end
return t;
end)
package.preload['net.adns']=(function(...)
local c=require"net.server";
local o=require"net.dns";
local e=require"util.logger".init("adns");
local t,t=table.insert,table.remove;
local a,r,l=coroutine,tostring,pcall;
local function u(a,a,t,e)return(e-t)+1;end
module"adns"
function lookup(d,t,s,h)
return a.wrap(function(i)
if i then
e("debug","Records for %s already cached, using those...",t);
d(i);
return;
end
e("debug","Records for %s not in cache, sending query (%s)...",t,r(a.running()));
local i,n=o.query(t,s,h);
if i then
a.yield({h or"IN",s or"A",t,a.running()});
e("debug","Reply for %s (%s)",t,r(a.running()));
end
if i then
i,n=l(d,o.peek(t,s,h));
else
e("error","Error sending DNS query: %s",n);
i,n=l(d,nil,n);
end
if not i then
e("error","Error in DNS response handler: %s",r(n));
end
end)(o.peek(t,s,h));
end
function cancel(t,a,i)
e("warn","Cancelling DNS lookup for %s",r(t[3]));
o.cancel(t[1],t[2],t[3],t[4],a);
end
function new_async_socket(a,i)
local s="<unknown>";
local n={};
local t={};
function n.onincoming(a,e)
if e then
o.feed(t,e);
end
end
function n.ondisconnect(o,a)
if a then
e("warn","DNS socket for %s disconnected: %s",s,a);
local t=i.server;
if i.socketset[o]==i.best_server and i.best_server==#t then
e("error","Exhausted all %d configured DNS servers, next lookup will try %s again",#t,t[1]);
end
i:servfail(o);
end
end
t=c.wrapclient(a,"dns",53,n);
if not t then
e("warn","handler is nil");
end
t.settimeout=function()end
t.setsockname=function(e,...)return a:setsockname(...);end
t.setpeername=function(o,...)s=(...);local e=a:setpeername(...);o:set_send(u);return e;end
t.connect=function(e,...)return a:connect(...)end
t.send=function(t,o)
local t=a.getpeername;
e("debug","Sending DNS query to %s",(t and t(a))or"<unconnected>");
return a:send(o);
end
return t;
end
o.socket_wrapper_set(new_async_socket);
return _M;
end)
package.preload['net.server']=(function(...)
local h=function(e)
return _G[e]
end
local ie=function(e)
for t,a in pairs(e)do
e[t]=nil
end
end
local H,e=require("util.logger").init("socket"),table.concat;
local i=function(...)return H("debug",e{...});end
local he=function(...)return H("warn",e{...});end
local e=collectgarbage
local ce=1
local R=h"type"
local A=h"pairs"
local me=h"ipairs"
local p=h"tonumber"
local s=h"tostring"
local e=h"collectgarbage"
local o=h"os"
local a=h"table"
local t=h"string"
local e=h"coroutine"
local V=o.difftime
local P=math.min
local ue=math.huge
local se=a.concat
local a=a.remove
local ne=t.len
local we=t.sub
local fe=e.wrap
local ye=e.yield
local k=h"ssl"
local T=h"socket"or require"socket"
local J=T.gettime
local re=(k and k.wrap)
local ve=T.bind
local pe=T.sleep
local be=T.select
local e=(k and k.newcontext)
local G
local B
local le
local Q
local K
local de
local m
local oe
local ae
local te
local Z
local W
local d
local ee
local e
local D
local X
local g
local r
local C
local l
local n
local j
local b
local w
local c
local a
local o
local v
local F
local M
local x
local z
local Y
local u
local E
local _
local O
local S
local N
local L
local U
local q
local I
g={}
r={}
l={}
C={}
n={}
b={}
w={}
j={}
a=0
o=0
v=0
F=0
M=0
x=1
z=0
E=51e3*1024
_=25e3*1024
O=12e5
S=6e4
N=6*60*60
L=false
q=1e3
I=30
te=function(c,t,w,u,p,m,f)
f=f or q
local h=0
local y,e=c.onconnect or c.onincoming,c.ondisconnect
local v=t.accept
local e={}
e.shutdown=function()end
e.ssl=function()
return m~=nil
end
e.sslctx=function()
return m
end
e.remove=function()
h=h-1
end
e.close=function()
for a,e in A(n)do
if e.serverport==u then
e.disconnect(e,"server closed")
e:close(true)
end
end
t:close()
o=d(l,t,o)
a=d(r,t,a)
n[t]=nil
e=nil
t=nil
i"server.lua: closed server handler and removed sockets from list"
end
e.ip=function()
return w
end
e.serverport=function()
return u
end
e.socket=function()
return t
end
e.readbuffer=function()
if h>f then
i("server.lua: refused new client connection: server full")
return false
end
local t,a=v(t)
if t then
local a,o=t:getpeername()
t:settimeout(0)
local e,n,t=D(e,c,t,a,u,o,p,m)
if t then
return false
end
h=h+1
i("server.lua: accepted new client connection from ",s(a),":",s(o)," to ",s(u))
return y(e)
elseif a then
i("server.lua: error with new client connection: ",s(a))
return false
end
end
return e
end
D=function(V,e,t,U,B,W,A,x)
t:settimeout(0)
local y
local z
local q
local T
local D=e.onincoming
local R=e.onstatus
local v=e.ondisconnect
local N=e.ondrain
local p={}
local h=0
local P
local S
local H
local f=0
local g=false
local O=false
local C,Y=0,0
local E=E
local _=_
local e=p
e.dispatch=function()
return D
end
e.disconnect=function()
return v
end
e.setlistener=function(a,t)
D=t.onincoming
v=t.ondisconnect
R=t.onstatus
N=t.ondrain
end
e.getstats=function()
return Y,C
end
e.ssl=function()
return T
end
e.sslctx=function()
return x
end
e.send=function(n,o,i,a)
return y(t,o,i,a)
end
e.receive=function(o,a)
return z(t,o,a)
end
e.shutdown=function(a)
return q(t,a)
end
e.setoption=function(i,a,o)
if t.setoption then
return t:setoption(a,o);
end
return false,"setoption not implemented";
end
e.close=function(u,s)
if not e then return true;end
a=d(r,t,a)
b[e]=nil
if h~=0 then
if not(s or S)then
e.sendbuffer()
if h~=0 then
if e then
e.write=nil
end
P=true
return false
end
else
y(t,se(p,"",1,h),1,f)
end
end
if t then
c=q and q(t)
t:close()
o=d(l,t,o)
n[t]=nil
t=nil
else
i"server.lua: socket already closed"
end
if e then
w[e]=nil
j[e]=nil
e=nil
end
if V then
V.remove()
end
i"server.lua: closed client handler and removed socket from list"
return true
end
e.ip=function()
return U
end
e.serverport=function()
return B
end
e.clientport=function()
return W
end
local j=function(i,a)
f=f+ne(a)
if f>E then
j[e]="send buffer exceeded"
e.write=Q
return false
elseif t and not l[t]then
o=m(l,t,o)
end
h=h+1
p[h]=a
if e then
w[e]=w[e]or u
end
return true
end
e.write=j
e.bufferqueue=function(t)
return p
end
e.socket=function(a)
return t
end
e.set_mode=function(a,t)
A=t or A
return A
end
e.set_send=function(a,t)
y=t or y
return y
end
e.bufferlen=function(o,a,t)
E=t or E
_=a or _
return f,_,E
end
e.lock_read=function(i,o)
if o==true then
local o=a
a=d(r,t,a)
b[e]=nil
if a~=o then
g=true
end
elseif o==false then
if g then
g=false
a=m(r,t,a)
b[e]=u
end
end
return g
end
e.pause=function(t)
return t:lock_read(true);
end
e.resume=function(t)
return t:lock_read(false);
end
e.lock=function(i,a)
e.lock_read(a)
if a==true then
e.write=Q
local a=o
o=d(l,t,o)
w[e]=nil
if o~=a then
O=true
end
elseif a==false then
e.write=j
if O then
O=false
j("")
end
end
return g,O
end
local g=function()
local o,t,a=z(t,A)
if not t or(t=="wantread"or t=="timeout")then
local o=o or a or""
local a=ne(o)
if a>_ then
v(e,"receive buffer exceeded")
e:close(true)
return false
end
local a=a*ce
Y=Y+a
M=M+a
b[e]=u
return D(e,o,t)
else
i("server.lua: client ",s(U),":",s(W)," read error: ",s(t))
S=true
v(e,t)
c=e and e:close()
return false
end
end
local w=function()
local m,a,r,n,b;
local b;
if t then
n=se(p,"",1,h)
m,a,r=y(t,n,1,f)
b=(m or r or 0)*ce
C=C+b
F=F+b
c=L and ie(p)
else
m,a,b=false,"closed",0;
end
if m then
h=0
f=0
o=d(l,t,o)
w[e]=nil
if N then
N(e)
end
c=H and e:starttls(nil)
c=P and e:close()
return true
elseif r and(a=="timeout"or a=="wantwrite")then
n=we(n,r+1,f)
p[1]=n
h=1
f=f-r
w[e]=u
return true
else
i("server.lua: client ",s(U),":",s(W)," write error: ",s(a))
S=true
v(e,a)
c=e and e:close()
return false
end
end
local u;
function e.set_sslctx(n,t)
T=true
x=t;
local h
local f
u=fe(function(t)
local n
for u=1,I do
o=(h and d(l,t,o))or o
a=(f and d(r,t,a))or a
f,h=nil,nil
c,n=t:dohandshake()
if not n then
i("server.lua: ssl handshake done")
e.readbuffer=g
e.sendbuffer=w
c=R and R(e,"ssl-handshake-complete")
a=m(r,t,a)
return true
else
if n=="wantwrite"and not h then
o=m(l,t,o)
h=true
elseif n=="wantread"and not f then
a=m(r,t,a)
f=true
else
i("server.lua: ssl handshake error: ",s(n))
break;
end
ye()
end
end
v(e,"ssl handshake failed")
c=e and e:close(true)
return false
end
)
end
if k then
if x then
e:set_sslctx(x);
i("server.lua: ","starting ssl handshake")
local a
t,a=re(t,x)
if a then
i("server.lua: ssl error: ",s(a))
return nil,nil,a
end
t:settimeout(0)
e.readbuffer=u
e.sendbuffer=u
u(t)
if not t then
return nil,nil,"ssl handshake failed";
end
else
local c;
e.starttls=function(w,f)
if f then
c=f;
e:set_sslctx(c);
end
if h>0 then
i"server.lua: we need to do tls, but delaying until send buffer empty"
H=true
return
end
i("server.lua: attempting to start tls on "..s(t))
local f,h=t
t,h=re(t,c)
if h then
i("server.lua: error while starting tls on client: ",s(h))
return nil,h
end
t:settimeout(0)
y=t.send
z=t.receive
q=G
n[t]=e
a=m(r,t,a)
a=d(r,f,a)
o=d(l,f,o)
n[f]=nil
e.starttls=nil
H=nil
T=true
e.readbuffer=u
e.sendbuffer=u
u(t)
end
e.readbuffer=g
e.sendbuffer=w
end
else
e.readbuffer=g
e.sendbuffer=w
end
y=t.send
z=t.receive
q=(T and G)or t.shutdown
n[t]=e
a=m(r,t,a)
return e,t
end
G=function()
end
Q=function()
return false
end
m=function(t,a,e)
if not t[a]then
e=e+1
t[e]=a
t[a]=e
end
return e;
end
d=function(e,a,t)
local i=e[a]
if i then
e[a]=nil
local o=e[t]
e[t]=nil
if o~=a then
e[o]=i
e[i]=o
end
return t-1
end
return t
end
W=function(e)
o=d(l,e,o)
a=d(r,e,a)
n[e]=nil
e:close()
end
local function f(e,a,o)
local t;
local i=a.sendbuffer;
function a.sendbuffer()
i();
if t and a.bufferlen()<o then
e:lock_read(false);
t=nil;
end
end
local i=e.readbuffer;
function e.readbuffer()
i();
if not t and a.bufferlen()>=o then
t=true;
e:lock_read(true);
end
end
end
oe=function(o,e,d,l,h)
local t
if R(d)~="table"then
t="invalid listener table"
end
if R(e)~="number"or not(e>=0 and e<=65535)then
t="invalid port"
elseif g[e]then
t="listeners on port '"..e.."' already exist"
elseif h and not k then
t="luasec not found"
end
if t then
he("server.lua, port ",e,": ",t)
return nil,t
end
o=o or"*"
local t,s=ve(o,e)
if s then
he("server.lua, port ",e,": ",s)
return nil,s
end
local s,d=te(d,t,o,e,l,h,q)
if not s then
t:close()
return nil,d
end
t:settimeout(0)
a=m(r,t,a)
g[e]=s
n[t]=s
i("server.lua: new "..(h and"ssl "or"").."server listener on '",o,":",e,"'")
return s
end
ae=function(e)
return g[e];
end
ee=function(e)
local t=g[e]
if not t then
return nil,"no server found on port '"..s(e).."'"
end
t:close()
g[e]=nil
return true
end
de=function()
for e,t in A(n)do
t:close()
n[e]=nil
end
a=0
o=0
v=0
g={}
r={}
l={}
C={}
n={}
end
Z=function()
return x,z,E,_,O,S,N,L,q,I
end
X=function(e)
if R(e)~="table"then
return nil,"invalid settings table"
end
x=p(e.timeout)or x
z=p(e.sleeptime)or z
E=p(e.maxsendlen)or E
_=p(e.maxreadlen)or _
O=p(e.checkinterval)or O
S=p(e.sendtimeout)or S
N=p(e.readtimeout)or N
L=e.cleanqueue
q=e._maxclientsperserver or q
I=e._maxsslhandshake or I
return true
end
K=function(e)
if R(e)~="function"then
return nil,"invalid listener function"
end
v=v+1
C[v]=e
return true
end
le=function()
return M,F,a,o,v
end
local t;
setquitting=function(e)
t=not not e;
end
B=function(a)
if t then return"quitting";end
if a then t="once";end
local e=ue;
repeat
local o,a,s=be(r,l,P(x,e))
for e,t in me(a)do
local e=n[t]
if e then
e.sendbuffer()
else
W(t)
i"server.lua: found no handler and closed socket (writelist)"
end
end
for e,t in me(o)do
local e=n[t]
if e then
e.readbuffer()
else
W(t)
i"server.lua: found no handler and closed socket (readlist)"
end
end
for e,t in A(j)do
e.disconnect()(e,t)
e:close(true)
end
ie(j)
u=J()
if u-U>=P(e,1)then
e=ue;
for t=1,v do
local t=C[t](u)
if t then e=P(e,t);end
end
U=u
else
e=e-(u-U);
end
pe(z)
until t;
if a and t=="once"then t=nil;return;end
return"quitting"
end
step=function()
return B(true);
end
local function d()
return"select";
end
local i=function(t,h,i,a,s,e)
local e=D(nil,a,t,h,i,"clientport",s,e)
n[t]=e
o=m(l,t,o)
if a.onconnect then
local t=e.sendbuffer;
e.sendbuffer=function()
e.sendbuffer=t;
a.onconnect(e);
if#e:bufferqueue()>0 then
return t();
end
end
end
return e,t
end
local t=function(a,o,n,r,s)
local e,t=T.tcp()
if t then
return nil,t
end
e:settimeout(0)
c,t=e:connect(a,o)
if t then
local e=i(e,a,o,n)
else
D(nil,n,e,a,o,"clientport",r,s)
end
end
h"setmetatable"(n,{__mode="k"})
h"setmetatable"(b,{__mode="k"})
h"setmetatable"(w,{__mode="k"})
U=J()
Y=J()
K(function()
local e=V(u-Y)
if e>O then
Y=u
for e,t in A(w)do
if V(u-t)>S then
e.disconnect()(e,"send timeout")
e:close(true)
end
end
for e,t in A(b)do
if V(u-t)>N then
e.disconnect()(e,"read timeout")
e:close()
end
end
end
end
)
local function a(e)
local t=H;
if e then
H=e;
end
return t;
end
return{
addclient=t,
wrapclient=i,
loop=B,
link=f,
step=step,
stats=le,
closeall=de,
addtimer=K,
addserver=oe,
getserver=ae,
setlogger=a,
getsettings=Z,
setquitting=setquitting,
removeserver=ee,
get_backend=d,
changesettings=X,
}
end)
package.preload['util.xmppstream']=(function(...)
local t=require"lxp";
local e=require"util.stanza";
local p=e.stanza_mt;
local i=tostring;
local s=table.insert;
local y=table.concat;
local b=table.remove;
local w=setmetatable;
local f=require"util.logger".init("xmppstream");
local o=error;
module"xmppstream"
local c=t.new;
local v={
["http://www.w3.org/XML/1998/namespace"]="xml";
};
local t="http://etherx.jabber.org/streams";
local n="\1";
local l="^([^"..n.."]*)"..n.."?(.*)$";
_M.ns_separator=n;
_M.ns_pattern=l;
function new_sax_handlers(a,e)
local u={};
local j=a.log or f;
local c=e.streamopened;
local m=e.streamclosed;
local d=e.error or function(t,e)o("XML stream error: "..i(e));end;
local g=e.handlestanza;
local t=e.stream_ns or t;
local r=e.stream_tag or"stream";
if t~=""then
r=t..n..r;
end
local k=t..n..(e.error_tag or"error");
local q=e.default_ns;
local h={};
local o,e={};
local i=0;
function u:StartElement(u,t)
if e and#o>0 then
s(e,y(o));
o={};
end
local n,o=u:match(l);
if o==""then
n,o="",n;
end
if n~=q or i>0 then
t.xmlns=n;
i=i+1;
end
for e=1,#t do
local a=t[e];
t[e]=nil;
local e,o=a:match(l);
if o~=""then
e=v[e];
if e then
t[e..":"..o]=t[a];
t[a]=nil;
end
end
end
if not e then
if a.notopen then
if u==r then
i=0;
if c then
c(a,t);
end
else
d(a,"no-stream");
end
return;
end
if n=="jabber:client"and o~="iq"and o~="presence"and o~="message"then
d(a,"invalid-top-level-element");
end
e=w({name=o,attr=t,tags={}},p);
else
s(h,e);
local a=e;
e=w({name=o,attr=t,tags={}},p);
s(a,e);
s(a.tags,e);
end
end
function u:CharacterData(t)
if e then
s(o,t);
end
end
function u:EndElement(t)
if i>0 then
i=i-1;
end
if e then
if#o>0 then
s(e,y(o));
o={};
end
if#h==0 then
if t~=k then
g(a,e);
else
d(a,"stream-error",e);
end
e=nil;
else
e=b(h);
end
else
if t==r then
if m then
m(a);
end
else
local t,e=t:match(l);
if e==""then
t,e="",t;
end
d(a,"parse-error","unexpected-element-close",e);
end
e,o=nil,{};
h={};
end
end
local function i()
e,o=nil,{};
h={};
end
local function t(t,e)
a=e;
j=e.log or f;
end
return u,{reset=i,set_session=t};
end
function new(e,t)
local o,a=new_sax_handlers(e,t);
local e=c(o,n);
local t=e.parse;
return{
reset=function()
e=c(o,n);
t=e.parse;
a.reset();
end,
feed=function(o,a)
return t(e,a);
end,
set_session=a.set_session;
};
end
return _M;
end)
package.preload['util.jid']=(function(...)
local a=string.match;
local h=require"util.encodings".stringprep.nodeprep;
local s=require"util.encodings".stringprep.nameprep;
local n=require"util.encodings".stringprep.resourceprep;
module"jid"
local function t(e)
if not e then return;end
local i,t=a(e,"^([^@/]+)@()");
local t,o=a(e,"^([^@/]+)()",t)
if i and not t then return nil,nil,nil;end
local a=a(e,"^/(.+)$",o);
if(not t)or((not a)and#e>=o)then return nil,nil,nil;end
return i,t,a;
end
split=t;
function bare(e)
local t,e=t(e);
if t and e then
return t.."@"..e;
end
return e;
end
local function o(e)
local a,t,e=t(e);
if t then
t=s(t);
if not t then return;end
if a then
a=h(a);
if not a then return;end
end
if e then
e=n(e);
if not e then return;end
end
return a,t,e;
end
end
prepped_split=o;
function prep(e)
local t,e,a=o(e);
if e then
if t then
e=t.."@"..e;
end
if a then
e=e.."/"..a;
end
end
return e;
end
function join(a,e,t)
if a and e and t then
return a.."@"..e.."/"..t;
elseif a and e then
return a.."@"..e;
elseif e and t then
return e.."/"..t;
elseif e then
return e;
end
return nil;
end
function compare(e,a)
local n,i,o=t(e);
local a,t,e=t(a);
if((a~=nil and a==n)or a==nil)and
((t~=nil and t==i)or t==nil)and
((e~=nil and e==o)or e==nil)then
return true
end
return false
end
return _M;
end)
package.preload['util.events']=(function(...)
local o=pairs;
local r=table.insert;
local s=table.sort;
local h=setmetatable;
local i=next;
module"events"
function new()
local t={};
local e={};
local function n(n,a)
local e=e[a];
if not e or i(e)==nil then return;end
local t={};
for e in o(e)do
r(t,e);
end
s(t,function(a,t)return e[a]>e[t];end);
n[a]=t;
return t;
end;
h(t,{__index=n});
local function s(o,n,i)
local a=e[o];
if a then
a[n]=i or 0;
else
a={[n]=i or 0};
e[o]=a;
end
t[o]=nil;
end;
local function n(a,n)
local o=e[a];
if o then
o[n]=nil;
t[a]=nil;
if i(o)==nil then
e[a]=nil;
end
end
end;
local function a(e)
for t,e in o(e)do
s(t,e);
end
end;
local function i(e)
for e,t in o(e)do
n(e,t);
end
end;
local function o(e,...)
local e=t[e];
if e then
for t=1,#e do
local e=e[t](...);
if e~=nil then return e;end
end
end
end;
return{
add_handler=s;
remove_handler=n;
add_handlers=a;
remove_handlers=i;
fire_event=o;
_handlers=t;
_event_map=e;
};
end
return _M;
end)
package.preload['util.dataforms']=(function(...)
local d=setmetatable;
local e,i=pairs,ipairs;
local s,n=tostring,type;
local r=table.concat;
local o=require"util.stanza";
module"dataforms"
local a='jabber:x:data';
local h={};
local t={__index=h};
function new(e)
return d(e,t);
end
function h.form(t,h,e)
local e=o.stanza("x",{xmlns=a,type=e or"form"});
if t.title then
e:tag("title"):text(t.title):up();
end
if t.instructions then
e:tag("instructions"):text(t.instructions):up();
end
for t,o in i(t)do
local a=o.type or"text-single";
e:tag("field",{type=a,var=o.name,label=o.label});
local t=(h and h[o.name])or o.value;
if t then
if a=="hidden"then
if n(t)=="table"then
e:tag("value")
:add_child(t)
:up();
else
e:tag("value"):text(s(t)):up();
end
elseif a=="boolean"then
e:tag("value"):text((t and"1")or"0"):up();
elseif a=="fixed"then
elseif a=="jid-multi"then
for a,t in i(t)do
e:tag("value"):text(t):up();
end
elseif a=="jid-single"then
e:tag("value"):text(t):up();
elseif a=="text-single"or a=="text-private"then
e:tag("value"):text(t):up();
elseif a=="text-multi"then
for t in t:gmatch("([^\r\n]+)\r?\n*")do
e:tag("value"):text(t):up();
end
elseif a=="list-single"then
local a=false;
if n(t)=="string"then
e:tag("value"):text(t):up();
else
for o,t in i(t)do
if n(t)=="table"then
e:tag("option",{label=t.label}):tag("value"):text(t.value):up():up();
if t.default and(not a)then
e:tag("value"):text(t.value):up();
a=true;
end
else
e:tag("option",{label=t}):tag("value"):text(s(t)):up():up();
end
end
end
elseif a=="list-multi"then
for a,t in i(t)do
if n(t)=="table"then
e:tag("option",{label=t.label}):tag("value"):text(t.value):up():up();
if t.default then
e:tag("value"):text(t.value):up();
end
else
e:tag("option",{label=t}):tag("value"):text(s(t)):up():up();
end
end
end
end
if o.required then
e:tag("required"):up();
end
e:up();
end
return e;
end
local e={};
function h.data(n,t)
local o={};
for t in t:childtags()do
local a;
for o,e in i(n)do
if e.name==t.attr.var then
a=e.type;
break;
end
end
local e=e[a];
if e then
o[t.attr.var]=e(t);
end
end
return o;
end
e["text-single"]=
function(t)
local t=t:child_with_name("value");
if t then
return t[1];
end
end
e["text-private"]=
e["text-single"];
e["jid-single"]=
e["text-single"];
e["jid-multi"]=
function(a)
local t={};
for e in a:childtags()do
if e.name=="value"then
t[#t+1]=e[1];
end
end
return t;
end
e["text-multi"]=
function(a)
local t={};
for e in a:childtags()do
if e.name=="value"then
t[#t+1]=e[1];
end
end
return r(t,"\n");
end
e["list-single"]=
e["text-single"];
e["list-multi"]=
function(a)
local t={};
for e in a:childtags()do
if e.name=="value"then
t[#t+1]=e[1];
end
end
return t;
end
e["boolean"]=
function(t)
local t=t:child_with_name("value");
if t then
if t[1]=="1"or t[1]=="true"then
return true;
else
return false;
end
end
end
e["hidden"]=
function(e)
local e=e:child_with_name("value");
if e then
return e[1];
end
end
return _M;
end)
package.preload['verse.plugins.tls']=(function(...)
local t="urn:ietf:params:xml:ns:xmpp-tls";
function verse.plugins.tls(e)
local function i(a)
if e.authenticated then return;end
if a:get_child("starttls",t)and e.conn.starttls then
e:debug("Negotiating TLS...");
e:send(verse.stanza("starttls",{xmlns=t}));
return true;
elseif not e.conn.starttls and not e.secure then
e:warn("SSL libary (LuaSec) not loaded, so TLS not available");
elseif not e.secure then
e:debug("Server doesn't offer TLS :(");
end
end
local function o(t)
if t.name=="proceed"then
e:debug("Server says proceed, handshake starting...");
e.conn:starttls({mode="client",protocol="sslv23",options="no_sslv2"},true);
end
end
local function a(t)
if t=="ssl-handshake-complete"then
e.secure=true;
e:debug("Re-opening stream...");
e:reopen();
end
end
e:hook("stream-features",i,400);
e:hook("stream/"..t,o);
e:hook("status",a,400);
return true;
end
end)
package.preload['verse.plugins.sasl']=(function(...)
local t=require"mime".b64;
local a="urn:ietf:params:xml:ns:xmpp-sasl";
function verse.plugins.sasl(e)
local function i(o)
if e.authenticated then return;end
e:debug("Authenticating with SASL...");
local o=t("\0"..e.username.."\0"..e.password);
e:debug("Selecting PLAIN mechanism...");
local t=verse.stanza("auth",{xmlns=a,mechanism="PLAIN"});
if o then
t:text(o);
end
e:send(t);
return true;
end
local function o(t)
if t.name=="success"then
e.authenticated=true;
e:event("authentication-success");
elseif t.name=="failure"then
local t=t.tags[1];
e:event("authentication-failure",{condition=t.name});
end
e:reopen();
return true;
end
e:hook("stream-features",i,300);
e:hook("stream/"..a,o);
return true;
end
end)
package.preload['verse.plugins.bind']=(function(...)
local a="urn:ietf:params:xml:ns:xmpp-bind";
function verse.plugins.bind(e)
local function i(t)
if e.bound then return;end
e:debug("Binding resource...");
e:send_iq(verse.iq({type="set"}):tag("bind",{xmlns=a}):tag("resource"):text(e.resource),
function(t)
if t.attr.type=="result"then
local t=t
:get_child("bind",a)
:get_child("jid")
:get_text();
e.username,e.host,e.resource=jid.split(t);
e.jid,e.bound=t,true;
e:event("bind-success",{jid=t});
elseif t.attr.type=="error"then
local a=t:child_with_name("error");
local o,a,t=t:get_error();
e:event("bind-failure",{error=a,text=t,type=o});
end
end);
end
e:hook("stream-features",i,200);
return true;
end
end)
package.preload['verse.plugins.legacy']=(function(...)
local i=require"util.uuid".generate;
local o="jabber:iq:auth";
function verse.plugins.legacy(e)
function handle_auth_form(t)
local a=t:get_child("query",o);
if t.attr.type~="result"or not a then
local t,o,a=t:get_error();
e:debug("warn","%s %s: %s",t,o,a);
end
local t={
username=e.username;
password=e.password;
resource=e.resource or i();
digest=false,sequence=false,token=false;
};
local o=verse.iq({to=e.host,type="set"})
:tag("query",{xmlns=o});
if#a>0 then
for a in a:childtags()do
local a=a.name;
local i=t[a];
if i then
o:tag(a):text(t[a]):up();
elseif i==nil then
local t="feature-not-implemented";
e:event("authentication-failure",{condition=t});
return false;
end
end
else
for t,e in pairs(t)do
if e then
o:tag(t):text(e):up();
end
end
end
e:send_iq(o,function(a)
if a.attr.type=="result"then
e.resource=t.resource;
e.jid=t.username.."@"..e.host.."/"..t.resource;
e:event("authentication-success");
e:event("bind-success",e.jid);
else
local a,t,a=a:get_error();
e:event("authentication-failure",{condition=t});
end
end);
end
function handle_opened(t)
if not t.version then
e:send_iq(verse.iq({type="get"})
:tag("query",{xmlns="jabber:iq:auth"})
:tag("username"):text(e.username),
handle_auth_form);
end
end
e:hook("opened",handle_opened);
end
end)
package.preload['verse.plugins.pubsub']=(function(...)
local n=require"util.jid".bare;
local o="http://jabber.org/protocol/pubsub";
local i="http://jabber.org/protocol/pubsub#event";
local e="http://jabber.org/protocol/pubsub#errors";
local t={};
local a={__index=t};
function verse.plugins.pubsub(e)
e.pubsub=setmetatable({stream=e},a);
e:hook("message",function(a)
for t in a:childtags("event",i)do
local t=t:get_child("items");
if t then
local o=t.attr.node;
for t in t:childtags("item")do
e:event("pubsub/event",{
from=a.attr.from;
node=o;
item=t;
});
end
end
end
end);
return true;
end
function t:subscribe(i,a,t,e)
self.stream:send_iq(verse.iq({to=i,type="set"})
:tag("pubsub",{xmlns=o})
:tag("subscribe",{node=a,jid=t or n(self.stream.jid)})
,function(t)
if e then
if t.attr.type=="result"then
e(true);
else
e(false,t:get_error());
end
end
end
);
end
function t:publish(n,i,t,a,e)
self.stream:send_iq(verse.iq({to=n,type="set"})
:tag("pubsub",{xmlns=o})
:tag("publish",{node=i})
:tag("item",{id=t})
:add_child(a)
,function(t)
if e then
if t.attr.type=="result"then
e(true);
else
e(false,t:get_error());
end
end
end
);
end
end)
package.preload['verse.plugins.version']=(function(...)
local a="jabber:iq:version";
local function o(e,t)
e.name=t.name;
e.version=t.version;
e.platform=t.platform;
end
function verse.plugins.version(e)
e.version={set=o};
e:hook("iq/"..a,function(t)
if t.attr.type~="get"then return;end
local t=verse.reply(t)
:tag("query",{xmlns=a});
if e.version.name then
t:tag("name"):text(tostring(e.version.name)):up();
end
if e.version.version then
t:tag("version"):text(tostring(e.version.version)):up()
end
if e.version.platform then
t:tag("os"):text(e.version.platform);
end
e:send(t);
return true;
end);
function e:query_version(o,t)
t=t or function(t)return e:event("version/response",t);end
e:send_iq(verse.iq({type="get",to=o})
:tag("query",{xmlns=a}),
function(e)
local a=e:get_child("query",a);
if e.attr.type=="result"then
local o=a:get_child("name");
local i=a:get_child("version");
local e=a:get_child("os");
t({
name=o and o:get_text()or nil;
version=i and i:get_text()or nil;
platform=e and e:get_text()or nil;
});
else
local e,o,a=e:get_error();
t({
error=true;
condition=o;
text=a;
type=e;
});
end
end);
end
return true;
end
end)
package.preload['verse.plugins.ping']=(function(...)
local o="urn:xmpp:ping";
function verse.plugins.ping(t)
function t:ping(e,a)
local n=socket.gettime();
t:send_iq(verse.iq{to=e,type="get"}:tag("ping",{xmlns=o}),
function(t)
if t.attr.type=="error"then
local i,t,o=t:get_error();
if t~="service-unavailable"and t~="feature-not-implemented"then
a(nil,e,{type=i,condition=t,text=o});
return;
end
end
a(socket.gettime()-n,e);
end);
end
return true;
end
end)
package.preload['verse.plugins.session']=(function(...)
local a="urn:ietf:params:xml:ns:xmpp-session";
function verse.plugins.session(e)
local function o(t)
local t=t:get_child("session",a);
if t and not t:get_child("optional")then
local function o(t)
e:debug("Establishing Session...");
e:send_iq(verse.iq({type="set"}):tag("session",{xmlns=a}),
function(t)
if t.attr.type=="result"then
e:event("session-success");
elseif t.attr.type=="error"then
local a=t:child_with_name("error");
local a,t,o=t:get_error();
e:event("session-failure",{error=t,text=o,type=a});
end
end);
return true;
end
e:hook("bind-success",o);
end
end
e:hook("stream-features",o);
return true;
end
end)
package.preload['verse.plugins.compression']=(function(...)
local e=require"zlib";
local t="http://jabber.org/features/compress"
local t="http://jabber.org/protocol/compress"
local a="http://etherx.jabber.org/streams";
local o=9;
local function s(a)
local o,e=pcall(e.deflate,o);
if o==false then
local t=verse.stanza("failure",{xmlns=t}):tag("setup-failed");
a:send(t);
a:error("Failed to create zlib.deflate filter: %s",tostring(e));
return
end
return e
end
local function r(a)
local o,e=pcall(e.inflate);
if o==false then
local t=verse.stanza("failure",{xmlns=t}):tag("setup-failed");
a:send(t);
a:error("Failed to create zlib.inflate filter: %s",tostring(e));
return
end
return e
end
local function h(e,o)
function e:send(a)
local o,a,i=pcall(o,tostring(a),'sync');
if o==false then
e:close({
condition="undefined-condition";
text=a;
extra=verse.stanza("failure",{xmlns=t}):tag("processing-failed");
});
e:warn("Compressed send failed: %s",tostring(a));
return;
end
e.conn:write(a);
end;
end
local function o(e,a)
local o=e.data
e.data=function(n,i)
e:debug("Decompressing data...");
local i,a,s=pcall(a,i);
if i==false then
e:close({
condition="undefined-condition";
text=a;
extra=verse.stanza("failure",{xmlns=t}):tag("processing-failed");
});
stream:warn("%s",tostring(a));
return;
end
return o(n,a);
end;
end
function verse.plugins.compression(e)
local function i(a)
if not e.compressed then
local a=a:child_with_name("compression");
if a then
for a in a:children()do
local a=a[1]
if a=="zlib"then
e:send(verse.stanza("compress",{xmlns=t}):tag("method"):text("zlib"))
e:debug("Enabled compression using zlib.")
return true;
end
end
session:debug("Remote server supports no compression algorithm we support.")
end
end
end
local function n(a)
if a.name=="compressed"then
e:debug("Activating compression...")
local a=s(e);
if not a then return end
local t=r(e);
if not t then return end
h(e,a);
o(e,t);
e.compressed=true;
e:reopen();
elseif a.name=="failure"then
e:warn("Failed to establish compression");
end
end
e:hook("stream-features",i,250);
e:hook("stream/"..t,n);
end
end)
package.preload['verse.plugins.blocking']=(function(...)
local a="urn:xmpp:blocking";
function verse.plugins.blocking(e)
e.blocking={};
function e.blocking:block_jid(o,t)
e:send_iq(verse.iq{type="set"}
:tag("block",{xmlns=a})
:tag("item",{jid=o})
,function()return t and t(true);end
,function()return t and t(false);end
);
end
function e.blocking:unblock_jid(o,t)
e:send_iq(verse.iq{type="set"}
:tag("unblock",{xmlns=a})
:tag("item",{jid=o})
,function()return t and t(true);end
,function()return t and t(false);end
);
end
function e.blocking:unblock_all_jids(t)
e:send_iq(verse.iq{type="set"}
:tag("unblock",{xmlns=a})
,function()return t and t(true);end
,function()return t and t(false);end
);
end
function e.blocking:get_blocked_jids(t)
e:send_iq(verse.iq{type="get"}
:tag("blocklist",{xmlns=a})
,function(e)
local a=e:get_child("blocklist",a);
if not a then return t and t(false);end
local e={};
for t in a:childtags()do
e[#e+1]=t.attr.jid;
end
return t and t(e);
end
,function(e)return t and t(false);end
);
end
end
end)
package.preload['verse.plugins.proxy65']=(function(...)
local e=require"util.events";
local r=require"util.uuid";
local h=require"util.sha1";
local i={};
i.__index=i;
local o="http://jabber.org/protocol/bytestreams";
local n;
function verse.plugins.proxy65(t)
t.proxy65=setmetatable({stream=t},i);
t.proxy65.available_streamhosts={};
local e=0;
t:hook("disco/service-discovered/proxy",function(a)
if a.type=="bytestreams"then
e=e+1;
t:send_iq(verse.iq({to=a.jid,type="get"})
:tag("query",{xmlns=o}),function(a)
e=e-1;
if a.attr.type=="result"then
local e=a:get_child("query",o)
:get_child("streamhost").attr;
t.proxy65.available_streamhosts[e.jid]={
jid=e.jid;
host=e.host;
port=tonumber(e.port);
};
end
if e==0 then
t:event("proxy65/discovered-proxies",t.proxy65.available_streamhosts);
end
end);
end
end);
t:hook("iq/"..o,function(a)
local e=verse.new(nil,{
initiator_jid=a.attr.from,
streamhosts={},
current_host=0;
});
for t in a.tags[1]:childtags()do
if t.name=="streamhost"then
table.insert(e.streamhosts,t.attr);
end
end
local function o()
if e.current_host<#e.streamhosts then
e.current_host=e.current_host+1;
e:connect(
e.streamhosts[e.current_host].host,
e.streamhosts[e.current_host].port
);
n(t,e,a.tags[1].attr.sid,a.attr.from,t.jid);
return true;
end
e:unhook("disconnected",o);
t:send(verse.error_reply(a,"cancel","item-not-found"));
end
function e:accept()
e:hook("disconnected",o,100);
e:hook("connected",function()
e:unhook("disconnected",o);
local e=verse.reply(a)
:tag("query",a.tags[1].attr)
:tag("streamhost-used",{jid=e.streamhosts[e.current_host].jid});
t:send(e);
end,100);
o();
end
function e:refuse()
end
t:event("proxy65/request",e);
end);
end
function i:new(t,s)
local e=verse.new(nil,{
target_jid=t;
bytestream_sid=r.generate();
});
local a=verse.iq{type="set",to=t}
:tag("query",{xmlns=o,mode="tcp",sid=e.bytestream_sid});
for t,e in ipairs(s or self.proxies)do
a:tag("streamhost",e):up();
end
self.stream:send_iq(a,function(a)
if a.attr.type=="error"then
local t,o,a=a:get_error();
e:event("connection-failed",{conn=e,type=t,condition=o,text=a});
else
local a=a.tags[1]:get_child("streamhost-used");
if not a then
end
e.streamhost_jid=a.attr.jid;
local a,i;
for o,t in ipairs(s or self.proxies)do
if t.jid==e.streamhost_jid then
a,i=t.host,t.port;
break;
end
end
if not(a and i)then
end
e:connect(a,i);
local function a()
e:unhook("connected",a);
local t=verse.iq{to=e.streamhost_jid,type="set"}
:tag("query",{xmlns=o,sid=e.bytestream_sid})
:tag("activate"):text(t);
self.stream:send_iq(t,function(t)
if t.attr.type=="result"then
e:event("connected",e);
else
end
end);
return true;
end
e:hook("connected",a,100);
n(self.stream,e,e.bytestream_sid,self.stream.jid,t);
end
end);
return e;
end
function n(i,e,a,t,o)
local s=h.sha1(a..t..o);
local function n()
e:unhook("connected",n);
return true;
end
local function i(t)
e:unhook("incoming-raw",i);
if t:sub(1,2)~="\005\000"then
return e:event("error","connection-failure");
end
e:event("connected");
return true;
end
local function a(o)
e:unhook("incoming-raw",a);
if o~="\005\000"then
local t="version-mismatch";
if o:sub(1,1)=="\005"then
t="authentication-failure";
end
return e:event("error",t);
end
e:send(string.char(5,1,0,3,#s)..s.."\0\0");
e:hook("incoming-raw",i,100);
return true;
end
e:hook("connected",n,200);
e:hook("incoming-raw",a,100);
e:send("\005\001\000");
end
end)
package.preload['verse.plugins.jingle']=(function(...)
local e=require"util.sha1".sha1;
local e=require"util.timer";
local i=require"util.uuid".generate;
local o="urn:xmpp:jingle:1";
local s="urn:xmpp:jingle:errors:1";
local t={};
t.__index=t;
local e={};
local e={};
function verse.plugins.jingle(e)
e:hook("ready",function()
e:add_disco_feature(o);
end,10);
function e:jingle(a)
return verse.eventable(setmetatable(base or{
role="initiator";
peer=a;
sid=i();
stream=e;
},t));
end
function e:register_jingle_transport(e)
end
function e:register_jingle_content_type(e)
end
local function l(i)
local h=i:get_child("jingle",o);
local a=h.attr.sid;
local n=h.attr.action;
local a=e:event("jingle/"..a,i);
if a==true then
e:send(verse.reply(i));
return true;
end
if n~="session-initiate"then
local t=verse.error_reply(i,"cancel","item-not-found")
:tag("unknown-session",{xmlns=s}):up();
e:send(t);
return;
end
local d=h.attr.sid;
local a=verse.eventable{
role="receiver";
peer=i.attr.from;
sid=d;
stream=e;
};
setmetatable(a,t);
local n;
local r,s;
for t in h:childtags()do
if t.name=="content"and t.attr.xmlns==o then
local o=t:child_with_name("description");
local i=o.attr.xmlns;
if i then
local e=e:event("jingle/content/"..i,a,o);
if e then
r=e;
end
end
local o=t:child_with_name("transport");
local i=o.attr.xmlns;
s=e:event("jingle/transport/"..i,a,o);
if r and s then
n=t;
break;
end
end
end
if not r then
e:send(verse.error_reply(i,"cancel","feature-not-implemented","The specified content is not supported"));
return;
end
if not s then
e:send(verse.error_reply(i,"cancel","feature-not-implemented","The specified transport is not supported"));
return;
end
e:send(verse.reply(i));
a.content_tag=n;
a.creator,a.name=n.attr.creator,n.attr.name;
a.content,a.transport=r,s;
function a:decline()
end
e:hook("jingle/"..d,function(e)
if e.attr.from~=a.peer then
return false;
end
local e=e:get_child("jingle",o);
return a:handle_command(e);
end);
e:event("jingle",a);
return true;
end
function t:handle_command(a)
local t=a.attr.action;
e:debug("Handling Jingle command: %s",t);
if t=="session-terminate"then
self:destroy();
elseif t=="session-accept"then
self:handle_accepted(a);
elseif t=="transport-info"then
e:debug("Handling transport-info");
self.transport:info_received(a);
elseif t=="transport-replace"then
e:error("Peer wanted to swap transport, not implemented");
else
e:warn("Unhandled Jingle command: %s",t);
return nil;
end
return true;
end
function t:send_command(a,t,e)
local t=verse.iq({to=self.peer,type="set"})
:tag("jingle",{
xmlns=o,
sid=self.sid,
action=a,
initiator=self.role=="initiator"and self.stream.jid or nil,
responder=self.role=="responder"and self.jid or nil,
}):add_child(t);
if not e then
self.stream:send(t);
else
self.stream:send_iq(t,e);
end
end
function t:accept(a)
local t=verse.iq({to=self.peer,type="set"})
:tag("jingle",{
xmlns=o,
sid=self.sid,
action="session-accept",
responder=e.jid,
})
:tag("content",{creator=self.creator,name=self.name});
local o=self.content:generate_accept(self.content_tag:child_with_name("description"),a);
t:add_child(o);
local a=self.transport:generate_accept(self.content_tag:child_with_name("transport"),a);
t:add_child(a);
local a=self;
e:send_iq(t,function(t)
if t.attr.type=="error"then
local a,t,a=t:get_error();
e:error("session-accept rejected: %s",t);
return false;
end
a.transport:connect(function(t)
e:warn("CONNECTED (receiver)!!!");
a.state="active";
a:event("connected",t);
end);
end);
end
e:hook("iq/"..o,l);
return true;
end
function t:offer(t,a)
local e=verse.iq({to=self.peer,type="set"})
:tag("jingle",{xmlns=o,action="session-initiate",
initiator=self.stream.jid,sid=self.sid});
e:tag("content",{creator=self.role,name=t});
local t=self.stream:event("jingle/describe/"..t,a);
if not t then
return false,"Unknown content type";
end
e:add_child(t);
local t=self.stream:event("jingle/transport/".."urn:xmpp:jingle:transports:s5b:1",self);
self.transport=t;
e:add_child(t:generate_initiate());
self.stream:debug("Hooking %s","jingle/"..self.sid);
self.stream:hook("jingle/"..self.sid,function(e)
if e.attr.from~=self.peer then
return false;
end
local e=e:get_child("jingle",o);
return self:handle_command(e)
end);
self.stream:send_iq(e,function(e)
if e.type=="error"then
self.state="terminated";
local a,e,t=e:get_error();
return self:event("error",{type=a,condition=e,text=t});
end
end);
self.state="pending";
end
function t:terminate(e)
local e=verse.stanza("reason"):tag(e or"success");
self:send_command("session-terminate",e,function(e)
self.state="terminated";
self.transport:disconnect();
self:destroy();
end);
end
function t:destroy()
self:event("terminated");
self.stream:unhook("jingle/"..self.sid,self.handle_command);
end
function t:handle_accepted(e)
local e=e:child_with_name("transport");
self.transport:handle_accepted(e);
self.transport:connect(function(e)
print("CONNECTED (initiator)!")
self.state="active";
self:event("connected",e);
end);
end
function t:set_source(a,o)
local function t()
local e,i=a();
if e and e~=""then
self.transport.conn:send(e);
elseif e==""then
return t();
elseif e==nil then
if o then
self:terminate();
end
self.transport.conn:unhook("drained",t);
a=nil;
end
end
self.transport.conn:hook("drained",t);
t();
end
function t:set_sink(t)
self.transport.conn:hook("incoming-raw",t);
self.transport.conn:hook("disconnected",function(e)
self.stream:debug("Closing sink...");
local e=e.reason;
if e=="closed"then e=nil;end
t(nil,e);
end);
end
end)
package.preload['verse.plugins.jingle_ft']=(function(...)
local n=require"ltn12";
local s=package.config:sub(1,1);
local a="urn:xmpp:jingle:apps:file-transfer:1";
local i="http://jabber.org/protocol/si/profile/file-transfer";
function verse.plugins.jingle_ft(t)
t:hook("ready",function()
t:add_disco_feature(a);
end,10);
local o={type="file"};
function o:generate_accept(t,e)
if e and e.save_file then
self.jingle:hook("connected",function()
local e=n.sink.file(io.open(e.save_file,"w+"));
self.jingle:set_sink(e);
end);
end
return t;
end
local o={__index=o};
t:hook("jingle/content/"..a,function(t,e)
local e=e:get_child("offer"):get_child("file",i);
local e={
name=e.attr.name;
size=tonumber(e.attr.size);
};
return setmetatable({jingle=t,file=e},o);
end);
t:hook("jingle/describe/file",function(e)
local t;
if e.timestamp then
t=os.date("!%Y-%m-%dT%H:%M:%SZ",e.timestamp);
end
return verse.stanza("description",{xmlns=a})
:tag("offer")
:tag("file",{xmlns=i,
name=e.filename,
size=e.size,
date=t,
hash=e.hash,
})
:tag("desc"):text(e.description or"");
end);
function t:send_file(i,t)
local e,a=io.open(t);
if not e then return e,a;end
local o=e:seek("end",0);
e:seek("set",0);
local a=n.source.file(e);
local e=self:jingle(i);
e:offer("file",{
filename=t:match("[^"..s.."]+$");
size=o;
});
e:hook("connected",function()
e:set_source(a,true);
end);
return e;
end
end
end)
package.preload['verse.plugins.jingle_s5b']=(function(...)
local a="urn:xmpp:jingle:transports:s5b:1";
local n=require"util.sha1".sha1;
local h=require"util.uuid".generate;
local function s(e,n)
local function i()
e:unhook("connected",i);
return true;
end
local function t(a)
e:unhook("incoming-raw",t);
if a:sub(1,2)~="\005\000"then
return e:event("error","connection-failure");
end
e:event("connected");
return true;
end
local function a(o)
e:unhook("incoming-raw",a);
if o~="\005\000"then
local t="version-mismatch";
if o:sub(1,1)=="\005"then
t="authentication-failure";
end
return e:event("error",t);
end
e:send(string.char(5,1,0,3,#n)..n.."\0\0");
e:hook("incoming-raw",t,100);
return true;
end
e:hook("connected",i,200);
e:hook("incoming-raw",a,100);
e:send("\005\001\000");
end
local function i(a,e,i)
local e=verse.new(nil,{
streamhosts=e,
current_host=0;
});
local function t(o)
if o then
return a(nil,o.reason);
end
if e.current_host<#e.streamhosts then
e.current_host=e.current_host+1;
e:debug("Attempting to connect to "..e.streamhosts[e.current_host].host..":"..e.streamhosts[e.current_host].port.."...");
local t,a=e:connect(
e.streamhosts[e.current_host].host,
e.streamhosts[e.current_host].port
);
if not t then
e:debug("Error connecting to proxy (%s:%s): %s",
e.streamhosts[e.current_host].host,
e.streamhosts[e.current_host].port,
a
);
else
e:debug("Connecting...");
end
s(e,i);
return true;
end
e:unhook("disconnected",t);
return a(nil);
end
e:hook("disconnected",t,100);
e:hook("connected",function()
e:unhook("disconnected",t);
a(e.streamhosts[e.current_host],e);
end,100);
t();
return e;
end
function verse.plugins.jingle_s5b(e)
e:hook("ready",function()
e:add_disco_feature(a);
end,10);
local t={};
function t:generate_initiate()
self.s5b_sid=h();
local i=verse.stanza("transport",{xmlns=a,
mode="tcp",sid=self.s5b_sid});
local t=0;
for a,o in pairs(e.proxy65.available_streamhosts)do
t=t+1;
i:tag("candidate",{jid=a,host=o.host,
port=o.port,cid=a,priority=t,type="proxy"}):up();
end
e:debug("Have %d proxies",t)
return i;
end
function t:generate_accept(e)
local t={};
self.s5b_peer_candidates=t;
self.s5b_mode=e.attr.mode or"tcp";
self.s5b_sid=e.attr.sid or self.jingle.sid;
for e in e:childtags()do
t[e.attr.cid]={
type=e.attr.type;
jid=e.attr.jid;
host=e.attr.host;
port=tonumber(e.attr.port)or 0;
priority=tonumber(e.attr.priority)or 0;
cid=e.attr.cid;
};
end
local e=verse.stanza("transport",{xmlns=a});
return e;
end
function t:connect(o)
e:warn("Connecting!");
local t={};
for a,e in pairs(self.s5b_peer_candidates or{})do
t[#t+1]=e;
end
if#t>0 then
self.connecting_peer_candidates=true;
local function s(e,t)
self.jingle:send_command("transport-info",verse.stanza("content",{creator=self.creator,name=self.name})
:tag("transport",{xmlns=a,sid=self.s5b_sid})
:tag("candidate-used",{cid=e.cid}));
self.onconnect_callback=o;
self.conn=t;
end
local e=n(self.s5b_sid..self.peer..e.jid,true);
i(s,t,e);
else
e:warn("Actually, I'm going to wait for my peer to tell me its streamhost...");
self.onconnect_callback=o;
end
end
function t:info_received(t)
e:warn("Info received");
local s=t:child_with_name("content");
local o=s:child_with_name("transport");
if o:get_child("candidate-used")and not self.connecting_peer_candidates then
local t=o:child_with_name("candidate-used");
if t then
local function h(o,e)
if self.jingle.role=="initiator"then
self.jingle.stream:send_iq(verse.iq({to=o.jid,type="set"})
:tag("query",{xmlns=xmlns_bytestreams,sid=self.s5b_sid})
:tag("activate"):text(self.jingle.peer),function(o)
if o.attr.type=="result"then
self.jingle:send_command("transport-info",verse.stanza("content",s.attr)
:tag("transport",{xmlns=a,sid=self.s5b_sid})
:tag("activated",{cid=t.attr.cid}));
self.conn=e;
self.onconnect_callback(e);
else
self.jingle.stream:error("Failed to activate bytestream");
end
end);
end
end
self.jingle.stream:debug("CID: %s",self.jingle.stream.proxy65.available_streamhosts[t.attr.cid]);
local t={
self.jingle.stream.proxy65.available_streamhosts[t.attr.cid];
};
local e=n(self.s5b_sid..e.jid..self.peer,true);
i(h,t,e);
end
elseif o:get_child("activated")then
self.onconnect_callback(self.conn);
end
end
function t:disconnect()
if self.conn then
self.conn:close();
end
end
function t:handle_accepted(e)
end
local t={__index=t};
e:hook("jingle/transport/"..a,function(e)
return setmetatable({
role=e.role,
peer=e.peer,
stream=e.stream,
jingle=e,
},t);
end);
end
end)
package.preload['verse.plugins.presence']=(function(...)
function verse.plugins.presence(e)
e.last_presence=nil;
e:hook("presence-out",function(t)
if not t.attr.to then
e.last_presence=t;
end
end,1);
function e:resend_presence()
if last_presence then
e:send(last_presence);
end
end
function e:set_status(t)
local a=verse.presence();
if type(t)=="table"then
if t.show then
a:tag("show"):text(t.show):up();
end
if t.prio then
a:tag("priority"):text(tostring(t.prio)):up();
end
if t.msg then
a:tag("status"):text(t.msg):up();
end
end
e:send(a);
end
end
end)
package.preload['verse.plugins.disco']=(function(...)
local i=require("mime").b64
local n=require("util.sha1").sha1
local h="http://jabber.org/protocol/caps";
local e="http://jabber.org/protocol/disco";
local o=e.."#info";
local a=e.."#items";
function verse.plugins.disco(e)
e:add_plugin("presence");
e.disco={cache={},info={}}
e.disco.info.identities={
{category='client',type='pc',name='Verse'},
}
e.disco.info.features={
{var=h},
{var=o},
{var=a},
}
e.disco.items={}
e.disco.nodes={}
e.caps={}
e.caps.node='http://code.matthewwild.co.uk/verse/'
local function s(t,e)
if t.category<e.category then
return true;
elseif e.category<t.category then
return false;
end
if t.type<e.type then
return true;
elseif e.type<t.type then
return false;
end
if(not t['xml:lang']and e['xml:lang'])or
(e['xml:lang']and t['xml:lang']<e['xml:lang'])then
return true
end
return false
end
local function t(t,e)
return t.var<e.var
end
local function r()
table.sort(e.disco.info.identities,s)
table.sort(e.disco.info.features,t)
local t=''
for a,e in pairs(e.disco.info.identities)do
t=t..string.format(
'%s/%s/%s/%s',e.category,e.type,
e['xml:lang']or'',e.name or''
)..'<'
end
for a,e in pairs(e.disco.info.features)do
t=t..e.var..'<'
end
return(i(n(t)))
end
setmetatable(e.caps,{
__call=function(...)
local t=r()
return verse.stanza('c',{
xmlns=h,
hash='sha-1',
node=e.caps.node,
ver=t
})
end
})
function e:add_disco_feature(t)
table.insert(self.disco.info.features,{var=t});
e:resend_presence();
end
function e:remove_disco_feature(a)
for t,o in ipairs(self.disco.info.features)do
if o.var==a then
table.remove(self.disco.info.features,t);
e:resend_presence();
return true;
end
end
end
function e:add_disco_item(a,t)
local e=self.disco.items;
if t then
e=self.disco.nodes[t];
if not e then
e={features={},items={}};
self.disco.nodes[t]=e;
e=e.items;
else
e=e.items;
end
end
table.insert(e,a);
end
function e:jid_has_identity(a,t,e)
local o=self.disco.cache[a];
if not o then
return nil,"no-cache";
end
local a=self.disco.cache[a].identities;
if e then
return a[t.."/"..e]or false;
end
for e in pairs(a)do
if e:match("^(.*)/")==t then
return true;
end
end
end
function e:jid_supports(e,t)
local e=self.disco.cache[e];
if not e or not e.features then
return nil,"no-cache";
end
return e.features[t]or false;
end
function e:get_local_services(a,o)
local e=self.disco.cache[self.host];
if not(e)or not(e.items)then
return nil,"no-cache";
end
local t={};
for i,e in ipairs(e.items)do
if self:jid_has_identity(e.jid,a,o)then
table.insert(t,e.jid);
end
end
return t;
end
function e:disco_local_services(a)
self:disco_items(self.host,nil,function(t)
if not t then
return a({});
end
local e=0;
local function o()
e=e-1;
if e==0 then
return a(t);
end
end
for a,t in ipairs(t)do
if t.jid then
e=e+1;
self:disco_info(t.jid,nil,o);
end
end
if e==0 then
return a(t);
end
end);
end
function e:disco_info(e,t,s)
local a=verse.iq({to=e,type="get"})
:tag("query",{xmlns=o,node=t});
self:send_iq(a,function(a)
if a.attr.type=="error"then
return s(nil,a:get_error());
end
local i,n={},{};
for e in a:get_child("query",o):childtags()do
if e.name=="identity"then
i[e.attr.category.."/"..e.attr.type]=e.attr.name or true;
elseif e.name=="feature"then
n[e.attr.var]=true;
end
end
if not self.disco.cache[e]then
self.disco.cache[e]={nodes={}};
end
if t then
if not self.disco.cache[e].nodes[t]then
self.disco.cache[e].nodes[t]={nodes={}};
end
self.disco.cache[e].nodes[t].identities=i;
self.disco.cache[e].nodes[t].features=n;
else
self.disco.cache[e].identities=i;
self.disco.cache[e].features=n;
end
return s(self.disco.cache[e]);
end);
end
function e:disco_items(t,o,n)
local i=verse.iq({to=t,type="get"})
:tag("query",{xmlns=a,node=o});
self:send_iq(i,function(e)
if e.attr.type=="error"then
return n(nil,e:get_error());
end
local i={};
for e in e:get_child("query",a):childtags()do
if e.name=="item"then
table.insert(i,{
name=e.attr.name;
jid=e.attr.jid;
node=e.attr.node;
});
end
end
if not self.disco.cache[t]then
self.disco.cache[t]={nodes={}};
end
if o then
if not self.disco.cache[t].nodes[o]then
self.disco.cache[t].nodes[o]={nodes={}};
end
self.disco.cache[t].nodes[o].items=i;
else
self.disco.cache[t].items=i;
end
return n(i);
end);
end
e:hook("iq/"..o,function(t)
if t.attr.type=='get'then
local a=t:child_with_name('query')
if not a then return;end
local s
local i
if a.attr.node then
local h=r()
local n=e.disco.nodes[a.attr.node]
if n and n.info then
s=n.info.identities or{}
i=n.info.identities or{}
elseif a.attr.node==e.caps.node..'#'..h then
s=e.disco.info.identities
i=e.disco.info.features
else
local t=verse.stanza('iq',{
to=t.attr.from,
from=t.attr.to,
id=t.attr.id,
type='error'
})
t:tag('query',{xmlns=o}):reset()
t:tag('error',{type='cancel'}):tag(
'item-not-found',{xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'}
)
e:send(t)
return true
end
else
s=e.disco.info.identities
i=e.disco.info.features
end
local a=verse.stanza('query',{
xmlns=o,
node=a.attr.node
})
for t,e in pairs(s)do
a:tag('identity',e):reset()
end
for o,t in pairs(i)do
a:tag('feature',t):reset()
end
e:send(verse.stanza('iq',{
to=t.attr.from,
from=t.attr.to,
id=t.attr.id,
type='result'
}):add_child(a))
return true
end
end);
e:hook("iq/"..a,function(t)
if t.attr.type=='get'then
local o=t:child_with_name('query')
if not o then return;end
local i
if o.attr.node then
local o=e.disco.nodes[o.attr.node]
if o then
i=o.items or{}
else
local t=verse.stanza('iq',{
to=t.attr.from,
from=t.attr.to,
id=t.attr.id,
type='error'
})
t:tag('query',{xmlns=a}):reset()
t:tag('error',{type='cancel'}):tag(
'item-not-found',{xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'}
)
e:send(t)
return true
end
else
i=e.disco.items
end
local a=verse.stanza('query',{
xmlns=a,
node=o.attr.node
})
for o,t in pairs(i)do
a:tag('item',t):reset()
end
e:send(verse.stanza('iq',{
to=t.attr.from,
from=t.attr.to,
id=t.attr.id,
type='result'
}):add_child(a))
return true
end
end);
local t;
e:hook("ready",function()
if t then return;end
t=true;
e:disco_local_services(function(t)
for t,a in ipairs(t)do
local t=e.disco.cache[a.jid];
if t then
for t in pairs(t.identities)do
local t,o=t:match("^(.*)/(.*)$");
e:event("disco/service-discovered/"..t,{
type=o,jid=a.jid;
});
end
end
end
e:event("ready");
end);
return true;
end,5);
e:hook("presence-out",function(t)
if not t:get_child("c",h)then
t:reset():add_child(e:caps()):reset();
end
end,10);
end
end)
package.preload['verse.plugins.pep']=(function(...)
local o="http://jabber.org/protocol/pubsub";
local a=o.."#event";
function verse.plugins.pep(e)
e.pep={};
e:hook("message",function(o)
local t=o:get_child("event",a);
if not t then return;end
local t=t:get_child("items");
if not t then return;end
local i=t.attr.node;
for t in t:childtags()do
if t.name=="item"and t.attr.xmlns==a then
e:event("pep/"..i,{
from=o.attr.from,
item=t.tags[1],
});
end
end
end);
function e:hook_pep(t,o,i)
local a=e.events._handlers["pep/"..t];
if not(a)or#a==0 then
e:add_disco_feature(t.."+notify");
end
e:hook("pep/"..t,o,i);
end
function e:unhook_pep(t,a)
e:unhook("pep/"..t,a);
local a=e.events._handlers["pep/"..t];
if not(a)or#a==0 then
e:remove_disco_feature(t.."+notify");
end
end
function e:publish_pep(t,a)
local t=verse.iq({type="set"})
:tag("pubsub",{xmlns=o})
:tag("publish",{node=a or t.attr.xmlns})
:tag("item")
:add_child(t);
return e:send_iq(t);
end
end
end)
package.preload['verse.plugins.adhoc']=(function(...)
local n=require"lib.adhoc";
local t="http://jabber.org/protocol/commands";
local d="jabber:x:data";
local a={};
a.__index=a;
local o={};
function verse.plugins.adhoc(e)
e:add_disco_feature(t);
function e:query_commands(a,o)
e:disco_items(a,t,function(a)
e:debug("adhoc list returned")
local t={};
for o,a in ipairs(a)do
t[a.node]=a.name;
end
e:debug("adhoc calling callback")
return o(t);
end);
end
function e:execute_command(i,o,t)
local e=setmetatable({
stream=e,jid=i,
command=o,callback=t
},a);
return e:execute();
end
local function r(t,e)
if not(e)or e=="user"then return true;end
if type(e)=="function"then
return e(t);
end
end
function e:add_adhoc_command(i,a,s,h)
o[a]=n.new(i,a,s,h);
e:add_disco_item({jid=e.jid,node=a,name=i},t);
return o[a];
end
local function i(t)
local a=t.tags[1];
local a=a.attr.node;
local a=o[a];
if not a then return;end
if not r(t.attr.from,a.permission)then
e:send(verse.error_reply(t,"auth","forbidden","You don't have permission to execute this command"):up()
:add_child(a:cmdtag("canceled")
:tag("note",{type="error"}):text("You don't have permission to execute this command")));
return true
end
return n.handle_cmd(a,{send=function(t)return e:send(t)end},t);
end
e:hook("iq/"..t,function(e)
local a=e.attr.type;
local t=e.tags[1].name;
if a=="set"and t=="command"then
return i(e);
end
end);
end
function a:_process_response(e)
if e.type=="error"then
self.status="canceled";
self.callback(self,{});
end
local e=e:get_child("command",t);
self.status=e.attr.status;
self.sessionid=e.attr.sessionid;
self.form=e:get_child("x",d);
self.callback(self);
end
function a:execute()
local e=verse.iq({to=self.jid,type="set"})
:tag("command",{xmlns=t,node=self.command});
self.stream:send_iq(e,function(e)
self:_process_response(e);
end);
end
function a:next(a)
local e=verse.iq({to=self.jid,type="set"})
:tag("command",{
xmlns=t,
node=self.command,
sessionid=self.sessionid
});
if a then e:add_child(a);end
self.stream:send_iq(e,function(e)
self:_process_response(e);
end);
end
end)
package.preload['verse.plugins.private']=(function(...)
local a="jabber:iq:private";
function verse.plugins.private(o)
function o:private_set(o,i,e,n)
local t=verse.iq({type="set"})
:tag("query",{xmlns=a});
if e then
if e.name==o and e.attr and e.attr.xmlns==i then
t:add_child(e);
else
t:tag(o,{xmlns=i})
:add_child(e);
end
end
self:send_iq(t,n);
end
function o:private_get(e,o,i)
self:send_iq(verse.iq({type="get"})
:tag("query",{xmlns=a})
:tag(e,{xmlns=o}),
function(t)
if t.attr.type=="result"then
local t=t:get_child("query",a);
local e=t:get_child(e,o);
i(e);
end
end);
end
end
end)
package.preload['verse.plugins.groupchat']=(function(...)
local h=require"events";
local t={};
t.__index=t;
local s="urn:xmpp:delay";
local n="http://jabber.org/protocol/muc";
function verse.plugins.groupchat(o)
o:add_plugin("presence")
o.rooms={};
o:hook("stanza",function(e)
local a=jid.bare(e.attr.from);
if not a then return end
local t=o.rooms[a]
if not t and e.attr.to and a then
t=o.rooms[e.attr.to.." "..a]
end
if t and t.opts.source and e.attr.to~=t.opts.source then return end
if t then
local n=select(3,jid.split(e.attr.from));
local i=e:get_child("body");
local o=e:get_child("delay",s);
local a={
room_jid=a;
room=t;
sender=t.occupants[n];
nick=n;
body=(i and i:get_text())or nil;
stanza=e;
delay=(o and o.attr.stamp);
};
local t=t:event(e.name,a);
return t or(e.name=="message")or nil;
end
end,500);
function o:join_room(i,s,a)
if not s then
return false,"no nickname supplied"
end
a=a or{};
local e=setmetatable({
stream=o,jid=i,nick=s,
subject=nil,
occupants={},
opts=a,
events=h.new()
},t);
if a.source then
self.rooms[a.source.." "..i]=e;
else
self.rooms[i]=e;
end
local a=e.occupants;
e:hook("presence",function(o)
local t=o.nick or s;
if not a[t]and o.stanza.attr.type~="unavailable"then
a[t]={
nick=t;
jid=o.stanza.attr.from;
presence=o.stanza;
};
local o=o.stanza:get_child("x",n.."#user");
if o then
local e=o:get_child("item");
if e and e.attr then
a[t].real_jid=e.attr.jid;
a[t].affiliation=e.attr.affiliation;
a[t].role=e.attr.role;
end
end
if t==e.nick then
e.stream:event("groupchat/joined",e);
else
e:event("occupant-joined",a[t]);
end
elseif a[t]and o.stanza.attr.type=="unavailable"then
if t==e.nick then
e.stream:event("groupchat/left",e);
if e.opts.source then
self.rooms[e.opts.source.." "..i]=nil;
else
self.rooms[i]=nil;
end
else
a[t].presence=o.stanza;
e:event("occupant-left",a[t]);
a[t]=nil;
end
end
end);
e:hook("message",function(t)
local t=t.stanza:get_child("subject");
t=t and t:get_text();
if t then
e.subject=#t>0 and t or nil;
end
end);
local t=verse.presence():tag("x",{xmlns=n}):reset();
self:event("pre-groupchat/joining",t);
e:send(t)
self:event("groupchat/joining",e);
return e;
end
o:hook("presence-out",function(e)
if not e.attr.to then
for a,t in pairs(o.rooms)do
t:send(e);
end
e.attr.to=nil;
end
end);
end
function t:send(e)
if e.name=="message"and not e.attr.type then
e.attr.type="groupchat";
end
if e.name=="presence"then
e.attr.to=self.jid.."/"..self.nick;
end
if e.attr.type=="groupchat"or not e.attr.to then
e.attr.to=self.jid;
end
if self.opts.source then
e.attr.from=self.opts.source
end
self.stream:send(e);
end
function t:send_message(e)
self:send(verse.message():tag("body"):text(e));
end
function t:set_subject(e)
self:send(verse.message():tag("subject"):text(e));
end
function t:leave(e)
self.stream:event("groupchat/leaving",self);
self:send(verse.presence({type="unavailable"}));
end
function t:admin_set(o,a,e,t)
self:send(verse.iq({type="set"})
:query(n.."#admin")
:tag("item",{nick=o,[a]=e})
:tag("reason"):text(t or""));
end
function t:set_role(t,e,a)
self:admin_set(t,"role",e,a);
end
function t:set_affiliation(a,e,t)
self:admin_set(a,"affiliation",e,t);
end
function t:kick(t,e)
self:set_role(t,"none",e);
end
function t:ban(e,t)
self:set_affiliation(e,"outcast",t);
end
function t:event(e,t)
self.stream:debug("Firing room event: %s",e);
return self.events.fire_event(e,t);
end
function t:hook(t,a,e)
return self.events.add_handler(t,a,e);
end
end)
package.preload['verse.plugins.uptime']=(function(...)
local a="jabber:iq:last";
local function t(e,t)
e.starttime=t.starttime;
end
function verse.plugins.uptime(e)
e.uptime={set=t};
e:hook("iq/"..a,function(t)
if t.attr.type~="get"then return;end
local t=verse.reply(t)
:tag("query",{seconds=tostring(os.difftime(os.time(),e.uptime.starttime)),xmlns=a});
e:send(t);
return true;
end);
function e:query_uptime(o,t)
t=t or function(t)return e:event("uptime/response",t);end
e:send_iq(verse.iq({type="get",to=o})
:tag("query",{xmlns=a}),
function(e)
local a=e:get_child("query",a);
if e.attr.type=="result"then
local e=a.attr.seconds;
t({
seconds=e or nil;
});
else
local e,a,o=e:get_error();
t({
error=true;
condition=a;
text=o;
type=e;
});
end
end);
end
return true;
end
end)
package.preload['verse.plugins.smacks']=(function(...)
local a="urn:xmpp:sm:2";
function verse.plugins.smacks(e)
local n={};
local i=0;
local o=0;
local function s(t)
if t.attr.xmlns=="jabber:client"or not t.attr.xmlns then
o=o+1;
e:debug("Increasing handled stanzas to %d for %s",o,t:top_tag());
end
end
local function h()
e:debug("smacks: connection lost");
e.stream_management_supported=nil;
if e.resumption_token then
e:debug("smacks: have resumption token, reconnecting in 1s...");
e.authenticated=nil;
verse.add_task(1,function()
e:connect(e.connect_host or e.host,e.connect_port or 5222);
end);
return true;
end
end
local function r(t)
if t.name=="r"then
e:debug("Ack requested... acking %d handled stanzas",o);
e:send(verse.stanza("a",{xmlns=a,h=tostring(o)}));
elseif t.name=="a"then
local t=tonumber(t.attr.h);
if t>i then
local a=#n;
for t=i+1,t do
table.remove(n,1);
end
e:debug("Received ack: New ack: "..t.." Last ack: "..i.." Unacked stanzas now: "..#n.." (was "..a..")");
i=t;
else
e:warn("Received bad ack for "..t.." when last ack was "..i);
end
elseif t.name=="enabled"then
e.smacks=true;
local i=e.send;
function e.send(o,t)
o:warn("SENDING");
if not t.attr.xmlns then
n[#n+1]=t;
local e=i(o,t);
i(o,verse.stanza("r",{xmlns=a}));
return e;
end
return i(o,t);
end
e:hook("stanza",s);
if t.attr.id then
e.resumption_token=t.attr.id;
e:hook("disconnected",h,100);
end
elseif t.name=="resumed"then
e:debug("Resumed successfully");
e:event("resumed");
else
e:warn("Don't know how to handle "..a.."/"..t.name);
end
end
local function t()
if not e.smacks then
e:debug("smacks: sending enable");
e:send(verse.stanza("enable",{xmlns=a,resume="true"}));
end
end
local function i(i)
if i:get_child("sm",a)then
e.stream_management_supported=true;
if e.smacks and e.bound then
e:debug("Resuming stream with %d handled stanzas",o);
e:send(verse.stanza("resume",{xmlns=a,
h=o,previd=e.resumption_token}));
return true;
else
e:hook("bind-success",t,1);
end
end
end
e:hook("stream-features",i,250);
e:hook("stream/"..a,r);
end
end)
package.preload['verse.plugins.keepalive']=(function(...)
function verse.plugins.keepalive(e)
e.keepalive_timeout=e.keepalive_timeout or 300;
verse.add_task(e.keepalive_timeout,function()
e.conn:write(" ");
return e.keepalive_timeout;
end);
end
end)
package.preload['net.httpclient_listener']=(function(...)
local a=require"util.logger".init("httpclient_listener");
local n=require"net.connlisteners".register;
local t={};
local e={};
local e={default_port=80,default_mode="*a"};
function e.onincoming(o,i)
local e=t[o];
if not e then
a("warn","Received response from connection %s with no request attached!",tostring(o));
return;
end
if i and e.reader then
e:reader(i);
end
end
function e.ondisconnect(a,o)
local e=t[a];
if e and o~="closed"then
e:reader(nil);
end
t[a]=nil;
end
function e.register_request(o,e)
a("debug","Attaching request %s to connection %s",tostring(e.id or e),tostring(o));
t[o]=e;
end
n("httpclient",e);
end)
package.preload['net.connlisteners']=(function(...)
local i=(CFG_SOURCEDIR or".").."/net/";
local h=require"net.server";
local o=require"util.logger".init("connlisteners");
local s=tostring;
local d,l,n=
dofile,xpcall,error
local r=debug.traceback;
module"connlisteners"
local e={};
function register(t,a)
if e[t]and e[t]~=a then
o("debug","Listener %s is already registered, not registering any more",t);
return false;
end
e[t]=a;
o("debug","Registered connection listener %s",t);
return true;
end
function deregister(t)
e[t]=nil;
end
function get(t)
local a=e[t];
if not a then
local n,i=l(function()d(i..t:gsub("[^%w%-]","_").."_listener.lua")end,r);
if not n then
o("error","Error while loading listener '%s': %s",s(t),s(i));
return nil,i;
end
a=e[t];
end
return a;
end
function start(a,e)
local t,o=get(a);
if not t then
n("No such connection module: "..a..(o and(" ("..o..")")or""),0);
end
local i=(e and e.interface)or t.default_interface or"*";
local o=(e and e.port)or t.default_port or n("Can't start listener "..a.." because no port was specified, and it has no default port",0);
local n=(e and e.mode)or t.default_mode or 1;
local a=(e and e.ssl)or nil;
local e=e and e.type=="ssl";
if e and not a then
return nil,"no ssl context";
end
return h.addserver(i,o,t,n,e and a or nil);
end
return _M;
end)
package.preload['util.httpstream']=(function(...)
local t=coroutine;
local n=tonumber;
local r=t.create(function()end);
t.resume(r);
module("httpstream")
local function c(d,o,u)
local e=t.yield();
local function i()
local a=e:find("\r\n",nil,true);
while not a do
e=e..t.yield();
a=e:find("\r\n",nil,true);
end
local t=e:sub(1,a-1);
e=e:sub(a+2);
return t;
end
local function r(a)
while#e<a do
e=e..t.yield();
end
local t=e:sub(1,a);
e=e:sub(a+1);
return t;
end
local function h()
local a={};
while true do
local e=i();
if e==""then break;end
local e,o=e:match("^([^%s:]+): *(.*)$");
if not e then t.yield("invalid-header-line");end
e=e:lower();
a[e]=a[e]and a[e]..","..o or o;
end
return a;
end
if not o or o=="server"then
while true do
local e=i();
local o,a,i=e:match("^(%S+)%s+(%S+)%s+HTTP/(%S+)$");
if not o then t.yield("invalid-status-line");end
a=a:gsub("^//+","/");
local t=h();
local e=n(t["content-length"]);
e=e or 0;
local e=r(e);
d({
method=o;
path=a;
httpversion=i;
headers=t;
body=e;
});
end
elseif o=="client"then
while true do
local a=i();
local l,a,o=a:match("^HTTP/(%S+)%s+(%d%d%d)%s+(.*)$");
a=n(a);
if not a then t.yield("invalid-status-line");end
local s=h();
local u=not
((u and u().method=="HEAD")
or(a==204 or a==304 or a==301)
or(a>=100 and a<200));
local o;
if u then
local a=n(s["content-length"]);
if s["transfer-encoding"]=="chunked"then
o="";
while true do
local e=i():match("^%x+");
if not e then t.yield("invalid-chunk-size");end
e=n(e,16)
if e==0 then break;end
o=o..r(e);
if i()~=""then t.yield("invalid-chunk-ending");end
end
local e=h();
elseif a then
o=r(a);
else
repeat
local t=t.yield();
e=e..t;
until t=="";
o,e=e,"";
end
end
d({
code=a;
httpversion=l;
headers=s;
body=o;
responseversion=l;
responseheaders=s;
});
end
else t.yield("unknown-parser-type");end
end
function new(n,i,o,a)
local e=t.create(c);
t.resume(e,n,o,a)
return{
feed=function(n,a)
if not a then
if o=="client"then t.resume(e,"");end
e=r;
return i();
end
local a,t=t.resume(e,a);
if t then
e=r;
return i(t);
end
end;
};
end
return _M;
end)
package.preload['net.http']=(function(...)
local m=require"socket"
local c=require"mime"
local o=require"socket.url"
local n=require"util.httpstream".new;
local f=require"net.server"
local e=require"net.connlisteners".get;
local h=e("httpclient")or error("No httpclient listener!");
local r,i=table.insert,table.concat;
local d,s=pairs,ipairs;
local a,u,w,p,y,v,t=
tonumber,tostring,xpcall,select,debug.traceback,string.char,string.format;
local l=require"util.logger".init("http");
module"http"
function urlencode(e)return e and(e:gsub("%W",function(e)return t("%%%02x",e:byte());end));end
function urldecode(e)return e and(e:gsub("%%(%x%x)",function(e)return v(a(e,16));end));end
local function a(e)
return e and(e:gsub("%W",function(e)
if e~=" "then
return t("%%%02x",e:byte());
else
return"+";
end
end));
end
function formencode(t)
local e={};
for o,t in s(t)do
r(e,a(t.name).."="..a(t.value));
end
return i(e,"&");
end
local function b(e,i,t)
if not e.parser then
local function o(t)
if e.callback then
for t,a in d(t)do e[t]=a;end
e.callback(t.body,t.code,e);
e.callback=nil;
end
destroy_request(e);
end
local function a(t)
if e.callback then
e.callback(t or"connection-closed",0,e);
e.callback=nil;
end
destroy_request(e);
end
local function t()
return e;
end
e.parser=n(o,a,"client",t);
end
e.parser:feed(i);
end
local function v(e)l("error","Traceback[http]: %s: %s",u(e),y());end
function request(e,t,s)
local e=o.parse(e);
if not(e and e.host)then
s(nil,0,e);
return nil,"invalid-url";
end
if not e.path then
e.path="/";
end
local n,o;
local a={["Host"]=e.host,["User-Agent"]="Prosody XMPP Server"}
if e.userinfo then
a["Authorization"]="Basic "..c.b64(e.userinfo);
end
if t then
n=t.headers;
e.onlystatus=t.onlystatus;
o=t.body;
if o then
e.method="POST ";
a["Content-Length"]=u(#o);
a["Content-Type"]="application/x-www-form-urlencoded";
end
if t.method then e.method=t.method;end
end
e.handler,e.conn=f.wrapclient(m.tcp(),e.host,e.port or 80,h,"*a");
e.write=function(...)return e.handler:write(...);end
e.conn:settimeout(0);
local u,t=e.conn:connect(e.host,e.port or 80);
if not u and t~="timeout"then
s(nil,0,e);
return nil,t;
end
local t={e.method or"GET"," ",e.path," HTTP/1.1\r\n"};
if e.query then
r(t,4,"?");
r(t,5,e.query);
end
e.write(i(t));
local t={[2]=": ",[4]="\r\n"};
if n then
for o,n in d(n)do
t[1],t[3]=o,n;
e.write(i(t));
a[o]=nil;
end
end
for o,n in d(a)do
t[1],t[3]=o,n;
e.write(i(t));
a[o]=nil;
end
e.write("\r\n");
if o then
e.write(o);
end
e.callback=function(a,t,o)l("debug","Calling callback, status %s",t or"---");return p(2,w(function()return s(a,t,o)end,v));end
e.reader=b;
e.state="status";
h.register_request(e.handler,e);
return e;
end
function destroy_request(e)
if e.conn then
e.conn=nil;
e.handler:close()
h.ondisconnect(e.handler,"closed");
end
end
_M.urlencode=urlencode;
return _M;
end)
package.preload['verse.bosh']=(function(...)
local s=require"util.xmppstream".new;
local i=require"util.stanza";
require"net.httpclient_listener";
local a=require"net.http";
local e=setmetatable({},{__index=verse.stream_mt});
e.__index=e;
local h="http://etherx.jabber.org/streams";
local n="http://jabber.org/protocol/httpbind";
local o=5;
function verse.new_bosh(a,t)
local t={
bosh_conn_pool={};
bosh_waiting_requests={};
bosh_rid=math.random(1,999999);
bosh_outgoing_buffer={};
bosh_url=t;
conn={};
};
function t:reopen()
self.bosh_need_restart=true;
self:flush();
end
local t=verse.new(a,t);
return setmetatable(t,e);
end
function e:connect()
self:_send_session_request();
end
function e:send(e)
self:debug("Putting into BOSH send buffer: %s",tostring(e));
self.bosh_outgoing_buffer[#self.bosh_outgoing_buffer+1]=i.clone(e);
self:flush();
end
function e:flush()
if self.connected
and#self.bosh_waiting_requests<self.bosh_max_requests
and(#self.bosh_waiting_requests==0
or#self.bosh_outgoing_buffer>0
or self.bosh_need_restart)then
self:debug("Flushing...");
local e=self:_make_body();
local t=self.bosh_outgoing_buffer;
for o,a in ipairs(t)do
e:add_child(a);
t[o]=nil;
end
self:_make_request(e);
else
self:debug("Decided not to flush.");
end
end
function e:_make_request(i)
local e,t=a.request(self.bosh_url,{body=tostring(i)},function(a,e,t)
if e~=0 then
self.inactive_since=nil;
return self:_handle_response(a,e,t);
end
local e=os.time();
if not self.inactive_since then
self.inactive_since=e;
elseif e-self.inactive_since>self.bosh_max_inactivity then
return self:_disconnected();
else
self:debug("%d seconds left to reconnect, retrying in %d seconds...",
self.bosh_max_inactivity-(e-self.inactive_since),o);
end
timer.add_task(o,function()
self:debug("Retrying request...");
for a,e in ipairs(self.bosh_waiting_requests)do
if e==t then
table.remove(self.bosh_waiting_requests,a);
break;
end
end
self:_make_request(i);
end);
end);
if e then
table.insert(self.bosh_waiting_requests,e);
else
self:warn("Request failed instantly: %s",t);
end
end
function e:_disconnected()
self.connected=nil;
self:event("disconnected");
end
function e:_send_session_request()
local e=self:_make_body();
e.attr.hold="1";
e.attr.wait="60";
e.attr["xml:lang"]="en";
e.attr.ver="1.6";
e.attr.from=self.jid;
e.attr.to=self.host;
e.attr.secure='true';
a.request(self.bosh_url,{body=tostring(e)},function(e,t)
if t==0 then
return self:_disconnected();
end
local e=self:_parse_response(e)
if not e then
self:warn("Invalid session creation response");
self:_disconnected();
return;
end
self.bosh_sid=e.attr.sid;
self.bosh_wait=tonumber(e.attr.wait);
self.bosh_hold=tonumber(e.attr.hold);
self.bosh_max_inactivity=tonumber(e.attr.inactivity);
self.bosh_max_requests=tonumber(e.attr.requests)or self.bosh_hold;
self.connected=true;
self:event("connected");
self:_handle_response_payload(e);
end);
end
function e:_handle_response(o,t,e)
if self.bosh_waiting_requests[1]~=e then
self:warn("Server replied to request that wasn't the oldest");
for t,a in ipairs(self.bosh_waiting_requests)do
if a==e then
self.bosh_waiting_requests[t]=nil;
break;
end
end
else
table.remove(self.bosh_waiting_requests,1);
end
local e=self:_parse_response(o);
if e then
self:_handle_response_payload(e);
end
self:flush();
end
function e:_handle_response_payload(t)
for e in t:childtags()do
if e.attr.xmlns==h then
self:event("stream-"..e.name,e);
elseif e.attr.xmlns then
self:event("stream/"..e.attr.xmlns,e);
else
self:event("stanza",e);
end
end
if t.attr.type=="terminate"then
self:_disconnected({reason=t.attr.condition});
end
end
local a={
stream_ns="http://jabber.org/protocol/httpbind",stream_tag="body",
default_ns="jabber:client",
streamopened=function(e,t)e.notopen=nil;e.payload=verse.stanza("body",t);return true;end;
handlestanza=function(t,e)t.payload:add_child(e);end;
};
function e:_parse_response(e)
self:debug("Parsing response: %s",e);
if e==nil then
self:debug("%s",debug.traceback());
self:_disconnected();
return;
end
local t={notopen=true,log=self.log};
local a=s(t,a);
a:feed(e);
return t.payload;
end
function e:_make_body()
self.bosh_rid=self.bosh_rid+1;
local e=verse.stanza("body",{
xmlns=n;
content="text/xml; charset=utf-8";
sid=self.bosh_sid;
rid=self.bosh_rid;
});
if self.bosh_need_restart then
self.bosh_need_restart=nil;
e.attr.restart='true';
end
return e;
end
end)
package.preload['verse.client']=(function(...)
local t=require"verse";
local o=t.stream_mt;
local r=require"util.jid".split;
local d=require"net.adns";
local e=require"lxp";
local a=require"util.stanza";
t.message,t.presence,t.iq,t.stanza,t.reply,t.error_reply=
a.message,a.presence,a.iq,a.stanza,a.reply,a.error_reply;
local s=require"util.xmppstream".new;
local n="http://etherx.jabber.org/streams";
local function h(e,t)
return e.priority<t.priority or(e.priority==t.priority and e.weight>t.weight);
end
local i={
stream_ns=n,
stream_tag="stream",
default_ns="jabber:client"};
function i.streamopened(e,t)
e.stream_id=t.id;
if not e:event("opened",t)then
e.notopen=nil;
end
return true;
end
function i.streamclosed(e)
return e:event("closed");
end
function i.handlestanza(t,e)
if e.attr.xmlns==n then
return t:event("stream-"..e.name,e);
elseif e.attr.xmlns then
return t:event("stream/"..e.attr.xmlns,e);
end
return t:event("stanza",e);
end
function o:reset()
if self.stream then
self.stream:reset();
else
self.stream=s(self,i);
end
self.notopen=true;
return true;
end
function o:connect_client(e,a)
self.jid,self.password=e,a;
self.username,self.host,self.resource=r(e);
self:add_plugin("tls");
self:add_plugin("sasl");
self:add_plugin("bind");
self:add_plugin("session");
function self.data(t,e)
local a,t=self.stream:feed(e);
if a then return;end
self:debug("debug","Received invalid XML (%s) %d bytes: %s",tostring(t),#e,e:sub(1,300):gsub("[\r\n]+"," "));
self:close("xml-not-well-formed");
end
self:hook("connected",function()self:reopen();end);
self:hook("incoming-raw",function(e)return self.data(self.conn,e);end);
self.curr_id=0;
self.tracked_iqs={};
self:hook("stanza",function(e)
local t,a=e.attr.id,e.attr.type;
if t and e.name=="iq"and(a=="result"or a=="error")and self.tracked_iqs[t]then
self.tracked_iqs[t](e);
self.tracked_iqs[t]=nil;
return true;
end
end);
self:hook("stanza",function(e)
if e.attr.xmlns==nil or e.attr.xmlns=="jabber:client"then
if e.name=="iq"and(e.attr.type=="get"or e.attr.type=="set")then
local a=e.tags[1]and e.tags[1].attr.xmlns;
if a then
ret=self:event("iq/"..a,e);
if not ret then
ret=self:event("iq",e);
end
end
if ret==nil then
self:send(t.error_reply(e,"cancel","service-unavailable"));
return true;
end
else
ret=self:event(e.name,e);
end
end
return ret;
end,-1);
self:hook("outgoing",function(e)
if e.name then
self:event("stanza-out",e);
end
end);
self:hook("stanza-out",function(e)
if not e.attr.xmlns then
self:event(e.name.."-out",e);
end
end);
local function e()
self:event("ready");
end
self:hook("session-success",e,-1)
self:hook("bind-success",e,-1);
local e=self.close;
function self:close(t)
if not self.notopen then
self:send("</stream:stream>");
end
return e(self);
end
local function a()
self:connect(self.connect_host or self.host,self.connect_port or 5222);
end
if not(self.connect_host or self.connect_port)then
d.lookup(function(t)
if t then
local e={};
self.srv_hosts=e;
for a,t in ipairs(t)do
table.insert(e,t.srv);
end
table.sort(e,h);
local t=e[1];
self.srv_choice=1;
if t then
self.connect_host,self.connect_port=t.target,t.port;
self:debug("Best record found, will connect to %s:%d",self.connect_host or self.host,self.connect_port or 5222);
end
self:hook("disconnected",function()
if self.srv_hosts and self.srv_choice<#self.srv_hosts then
self.srv_choice=self.srv_choice+1;
local e=e[self.srv_choice];
self.connect_host,self.connect_port=e.target,e.port;
a();
return true;
end
end,1e3);
self:hook("connected",function()
self.srv_hosts=nil;
end,1e3);
end
a();
end,"_xmpp-client._tcp."..(self.host)..".","SRV");
else
a();
end
end
function o:reopen()
self:reset();
self:send(a.stanza("stream:stream",{to=self.host,["xmlns:stream"]='http://etherx.jabber.org/streams',
xmlns="jabber:client",version="1.0"}):top_tag());
end
function o:send_iq(t,a)
local e=self:new_id();
self.tracked_iqs[e]=a;
t.attr.id=e;
self:send(t);
end
function o:new_id()
self.curr_id=self.curr_id+1;
return tostring(self.curr_id);
end
end)
package.preload['verse.component']=(function(...)
local a=require"verse";
local t=a.stream_mt;
local d=require"util.jid".split;
local e=require"lxp";
local o=require"util.stanza";
local r=require"util.sha1".sha1;
a.message,a.presence,a.iq,a.stanza,a.reply,a.error_reply=
o.message,o.presence,o.iq,o.stanza,o.reply,o.error_reply;
local h=require"util.xmppstream".new;
local s="http://etherx.jabber.org/streams";
local i="jabber:component:accept";
local n={
stream_ns=s,
stream_tag="stream",
default_ns=i};
function n.streamopened(e,t)
e.stream_id=t.id;
if not e:event("opened",t)then
e.notopen=nil;
end
return true;
end
function n.streamclosed(e)
return e:event("closed");
end
function n.handlestanza(t,e)
if e.attr.xmlns==s then
return t:event("stream-"..e.name,e);
elseif e.attr.xmlns or e.name=="handshake"then
return t:event("stream/"..(e.attr.xmlns or i),e);
end
return t:event("stanza",e);
end
function t:reset()
if self.stream then
self.stream:reset();
else
self.stream=h(self,n);
end
self.notopen=true;
return true;
end
function t:connect_component(e,n)
self.jid,self.password=e,n;
self.username,self.host,self.resource=d(e);
function self.data(a,e)
local o,a=self.stream:feed(e);
if o then return;end
t:debug("debug","Received invalid XML (%s) %d bytes: %s",tostring(a),#e,e:sub(1,300):gsub("[\r\n]+"," "));
t:close("xml-not-well-formed");
end
self:hook("incoming-raw",function(e)return self.data(self.conn,e);end);
self.curr_id=0;
self.tracked_iqs={};
self:hook("stanza",function(e)
local t,a=e.attr.id,e.attr.type;
if t and e.name=="iq"and(a=="result"or a=="error")and self.tracked_iqs[t]then
self.tracked_iqs[t](e);
self.tracked_iqs[t]=nil;
return true;
end
end);
self:hook("stanza",function(e)
if e.attr.xmlns==nil or e.attr.xmlns=="jabber:client"then
if e.name=="iq"and(e.attr.type=="get"or e.attr.type=="set")then
local t=e.tags[1]and e.tags[1].attr.xmlns;
if t then
ret=self:event("iq/"..t,e);
if not ret then
ret=self:event("iq",e);
end
end
if ret==nil then
self:send(a.error_reply(e,"cancel","service-unavailable"));
return true;
end
else
ret=self:event(e.name,e);
end
end
return ret;
end,-1);
self:hook("opened",function(e)
print(self.jid,self.stream_id,e.id);
local e=r(self.stream_id..n,true);
self:send(o.stanza("handshake",{xmlns=i}):text(e));
self:hook("stream/"..i,function(e)
if e.name=="handshake"then
self:event("authentication-success");
end
end);
end);
local function e()
self:event("ready");
end
self:hook("authentication-success",e,-1);
self:connect(self.connect_host or self.host,self.connect_port or 5347);
self:reopen();
end
function t:reopen()
self:reset();
self:send(o.stanza("stream:stream",{to=self.host,["xmlns:stream"]='http://etherx.jabber.org/streams',
xmlns=i,version="1.0"}):top_tag());
end
function t:close(t)
if not self.notopen then
self:send("</stream:stream>");
end
local e=self.conn.disconnect();
self.conn:close();
e(conn,t);
end
function t:send_iq(t,a)
local e=self:new_id();
self.tracked_iqs[e]=a;
t.attr.id=e;
self:send(t);
end
function t:new_id()
self.curr_id=self.curr_id+1;
return tostring(self.curr_id);
end
end)
pcall(require,"luarocks.require");
pcall(require,"ssl");
local a=require"net.server";
local n=require"util.events";
module("verse",package.seeall);
local e=_M;
_M.server=a;
local t={};
t.__index=t;
stream_mt=t;
e.plugins={};
function e.new(a,o)
local t=setmetatable(o or{},t);
t.id=tostring(t):match("%x*$");
t:set_logger(a,true);
t.events=n.new();
t.plugins={};
return t;
end
e.add_task=require"util.timer".add_task;
e.logger=logger.init;
e.log=e.logger("verse");
function e.set_logger(t)
e.log=t;
a.setlogger(t);
end
function e.filter_log(t,o)
local e={};
for a,t in ipairs(t)do
e[t]=true;
end
return function(t,a,...)
if e[t]then
return o(t,a,...);
end
end;
end
local function o(t)
e.log("error","Error: %s",t);
e.log("error","Traceback: %s",debug.traceback());
end
function e.set_error_handler(e)
o=e;
end
function e.loop()
return xpcall(a.loop,o);
end
function e.step()
return xpcall(a.step,o);
end
function e.quit()
return a.setquitting(true);
end
function t:connect(t,o)
t=t or"localhost";
o=tonumber(o)or 5222;
local i=socket.tcp()
i:settimeout(0);
local n,e=i:connect(t,o);
if not n and e~="timeout"then
self:warn("connect() to %s:%d failed: %s",t,o,e);
return self:event("disconnected",{reason=e})or false,e;
end
local t=a.wrapclient(i,t,o,new_listener(self),"*a");
if not t then
self:warn("connection initialisation failed: %s",e);
return self:event("disconnected",{reason=e})or false,e;
end
self.conn=t;
self.send=function(a,e)
self:event("outgoing",e);
e=tostring(e);
self:event("outgoing-raw",e);
return t:write(e);
end;
return true;
end
function t:close()
if not self.conn then
e.log("error","Attempt to close disconnected connection - possibly a bug");
return;
end
local e=self.conn.disconnect();
self.conn:close();
e(conn,reason);
end
function t:debug(...)
if self.logger and self.log.debug then
return self.logger("debug",...);
end
end
function t:warn(...)
if self.logger and self.log.warn then
return self.logger("warn",...);
end
end
function t:error(...)
if self.logger and self.log.error then
return self.logger("error",...);
end
end
function t:set_logger(t,e)
local a=self.logger;
if t then
self.logger=t;
end
if e then
if e==true then
e={"debug","info","warn","error"};
end
self.log={};
for t,e in ipairs(e)do
self.log[e]=true;
end
end
return a;
end
function stream_mt:set_log_levels(e)
self:set_logger(nil,e);
end
function t:event(e,...)
self:debug("Firing event: "..tostring(e));
return self.events.fire_event(e,...);
end
function t:hook(e,...)
return self.events.add_handler(e,...);
end
function t:unhook(e,t)
return self.events.remove_handler(e,t);
end
function e.eventable(e)
e.events=n.new();
e.hook,e.unhook=t.hook,t.unhook;
local t=e.events.fire_event;
function e:event(e,...)
return t(e,...);
end
return e;
end
function t:add_plugin(t)
if self.plugins[t]then return true;end
if require("verse.plugins."..t)then
local a,e=e.plugins[t](self);
if a~=false then
self:debug("Loaded %s plugin",t);
self.plugins[t]=true;
else
self:warn("Failed to load %s plugin: %s",t,e);
end
end
return self;
end
function new_listener(e)
local t={};
function t.onconnect(t)
e.connected=true;
e:event("connected");
end
function t.onincoming(a,t)
e:event("incoming-raw",t);
end
function t.ondisconnect(a,t)
e.connected=false;
e:event("disconnected",{reason=t});
end
function t.ondrain(t)
e:event("drained");
end
function t.onstatus(a,t)
e:event("status",t);
end
return t;
end
local t=require"util.logger".init("verse");
return e;
