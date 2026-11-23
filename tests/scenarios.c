/* Control-flow tests run the actual C operations. Win32 boundaries have explicit fakes. */
#include "base.h"
#include "admin.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
int cases=0;W out[4096];int lastline;
#define CHECK(x) do{cases++;if(!(x)){fprintf(stderr,"FAIL at line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
int _snwprintf(W*out,size_t cap,T fmt,...){va_list ap;va_start(ap,fmt);size_t i=0;for(;*fmt&&i+1<cap;fmt++){if(*fmt!='%'){out[i++]=*fmt;continue;}fmt++;int prec=0,digits=0,lim=0;while(*fmt=='.'||(*fmt>='0'&&*fmt<='9')){if(*fmt=='.')prec=1;else digits=digits*10+*fmt-'0';fmt++;}if(*fmt=='l'&&fmt[1]=='s'){fmt++;T s=va_arg(ap,T);while(*s&&i+1<cap&&(!prec||lim++<digits))out[i++]=*s++;}else if(*fmt=='u'||*fmt=='x'){U32 v=va_arg(ap,U32);char b[25];snprintf(b,sizeof b,*fmt=='u'?"%u":"%x",v);int len=strlen(b);while(len++<digits&&i+1<cap)out[i++]='0';for(char*p=b;*p&&i+1<cap;p++)out[i++]=*p;}}out[i]=0;va_end(ap);return (int)i;}
int errnum(W*b,int n,T why,U32 e){(void)e;cp(b,n,why);return 0;}
I32 GetComputerNameW(W*b,U32*n){return cp(b,*n,L"NASBOX");}U32 GetLastError(void){return 5;}
int Na,Nenable,Ndel,fadd,flsa,fenable,removed[2],added[2],modeerr,sessions,shareowner=1,activeuser=1;
U32 NetUserAdd(T a,U32 lev,U8*b,U32*arg){(void)a;(void)arg;User1*u=(User1*)b;CHECK(lev==1&&(u->flags&2)&&eq(u->comment,MARK));Na++;return fadd;}
U32 NetUserDel(T a,T b){(void)a;(void)b;Ndel++;return 0;}U32 NetUserSetInfo(T a,T b,U32 lev,U8*x,U32*arg){(void)a;(void)b;(void)arg;CHECK(lev==1008&&!(*(U32*)x&2));Nenable++;return fenable;}
I32 LookupAccountNameW(T a,T b,H c,U32*d,W*f,U32*l,U32*t){(void)a;(void)b;(void)d;(void)f;(void)l;(void)t;memset(c,1,12);return 1;}
U32 LsaOpenPolicy(LsaStr*a,LsaObj*b,U32 c,H*d){(void)a;(void)b;CHECK((c&0x810)==0x810);*d=(H)1;return 0;}U32 LsaClose(H h){(void)h;return 0;}U32 LsaNtStatusToWinError(U32 x){return x;}U32 LsaAddAccountRights(H a,H b,LsaStr*c,U32 n){(void)a;(void)b;CHECK(n==4&&eq(c[0].data,L"SeDenyInteractiveLogonRight")&&eq(c[1].data,L"SeDenyRemoteInteractiveLogonRight"));return flsa;}
W groupTag[]=L"ChaZeumNAS5:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";U32 NetUserGetInfo(T h,T n,U32 l,U8**r){(void)h;(void)n;(void)l;static User1 u;u.comment=activeuser?(W*)MARK:L"not the app";u.flags=0x201;*r=(U8*)&u;return 0;}
U32 NetShareGetInfo(T h,T n,U32 l,U8**r){(void)h;(void)n;(void)l;static Share502 s;s.comment=shareowner?groupTag:L"other";*r=(U8*)&s;return 0;}
U32 NetLocalGroupGetInfo(T h,T n,U32 l,U8**r){(void)h;(void)n;(void)l;static Group1 g;g.comment=groupTag;*r=(U8*)&g;return 0;}U32 NetApiBufferFree(H v){(void)v;return 0;}
U32 NetLocalGroupDelMembers(T h,T g,U32 l,U8*b,U32 n){(void)h;(void)l;(void)b;CHECK(n==1);removed[g[3]=='W']++;return 1377;}
U32 NetLocalGroupAddMembers(T h,T g,U32 l,U8*b,U32 n){(void)h;(void)l;(void)b;CHECK(n==1);added[g[3]=='W']++;return modeerr;}
U32 NetSessionEnum(T x,T y,T z,U32 l,U8**b,U32 size,U32*n,U32*t,U32*r){(void)x;(void)y;(void)z;(void)l;(void)size;(void)t;(void)r;*b=0;*n=0;sessions++;return 0;}U32 NetSessionDel(T x,T y,T z){(void)x;(void)y;(void)z;return 0;}
int occupied,openok=1,maprc,addedMap,cancelled,creds,registered,existing;W gotProfile[465],gotDest[140],gotUser[80],fakeowner[80]=L"NASBOX\\alpha";
I32 LogonUserW(T a,T b,T c,U32 d,U32 e,H*f){(void)a;(void)b;(void)c;CHECK(d==9&&e==3);*f=(H)11;return 1;}
I32 ImpersonateLoggedOnUser(H h){CHECK(h==(H)11);return 1;}I32 RevertToSelf(void){return 1;}void ExitProcess(U32 c){exit(c);}
U32 WNetGetConnectionW(T l,W*r,U32*n){(void)l;if(existing){cp(r,*n,existing==1?L"\\\\192.168.1.90\\자료":L"\\\\192.168.1.91\\other");return 0;}return 2250;}
U32 WNetGetUserW(T l,W*r,U32*n){(void)l;cp(r,*n,fakeowner);return 0;}
U32 WNetAddConnection2W(Resource*r,T pass,T who,U32 flags){CHECK(flags==0&&pass);if(!sz(pass))return 1326;addedMap++;cp(gotUser,CNT(gotUser),who);cp(gotDest,CNT(gotDest),r->remote);return maprc;}
U32 WNetCancelConnection2W(T l,U32 f,I32 force){(void)l;CHECK(!f&&!force);cancelled++;return 0;}U32 GetLogicalDrives(void){return occupied?(1u<<23):0;}
H CreateFileW(T n,U32 a,U32 b,H c,U32 d,U32 e,H f){(void)n;(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return openok?(H)20:BAD;}I32 CloseHandle(H h){(void)h;return 1;}
I32 CredWriteW(Cred*c,U32 f){CHECK(c->type==1&&c->persist==2&&!f&&c->blob&&c->bytes);creds++;return 1;}int putstr(H h,T k,T n,T v){(void)h;(void)k;(void)n;cp(gotProfile,CNT(gotProfile),v);registered++;return 1;}
int main(void){W hp[254],ip[16],pc[16],buf[320],pass[]=L"synthetic-test-data";Endpoint ep={L"home.example.net",L"192.168.1.90",L"NASBOX"};
T goodh[]={L"nas.example.net",L"8.8.8.8",L"xn--x.example",L"a-b.example"};for(int i=0;i<CNT(goodh);i++)CHECK(hostok(goodh[i]));
T badh[]={L"192.168.0.1",L"100.64.1.5",L"127.0.0.1",L"0.0.0.0",L"10.0.0.22",L"169.254.1.1",L"home",L"home..test",L"-hi.example",L"hi-.test",L"https://host.example",L"home.example:445",L"home.example/a",L"foo bar.example",L"3.4.5.999"};for(int i=0;i<CNT(badh);i++)CHECK(!hostok(badh[i]));
T goodip[]={L"10.1.2.3",L"192.168.0.22",L"172.16.0.2",L"172.31.255.254"};for(int i=0;i<CNT(goodip);i++)CHECK(ipprivate(goodip[i]));
T badip[]={L"8.8.8.8",L"127.0.0.1",L"172.32.0.1",L"192.169.2.3",L"10.01.2.3",L"10.0.1.265",L"10.0.0.1:445",L"fe80::1"};for(int i=0;i<CNT(badip);i++)CHECK(!ipprivate(badip[i]));
CHECK(invite(buf,CNT(buf),ep.host,ep.ip,ep.machine)&&pair(buf,hp,ip,pc)&&eq(hp,ep.host)&&eq(ip,ep.ip)&&eq(pc,ep.machine));CHECK(!pair(L"CZ5|home.example.net|192.168.0.2|NAS|extra",hp,ip,pc));CHECK(!pair(L"CZ5|home.example.net|8.8.8.8|NAS",hp,ip,pc));CHECK(machineok(L"강의PC"));CHECK(!machineok(L"aaa/bb"));CHECK(shareok(L"강의 자료"));CHECK(!shareok(L"folder\\path"));CHECK(!shareok(L"C$"));CHECK(!shareok(L".."));CHECK(!idok(L"Administrator"));CHECK(!idok(L"Guest"));CHECK(!idok(L"Name|more"));CHECK(idok(L"강의 자료"));CHECK(idok(L"_강사"));CHECK(idok(L"홍.길동"));CHECK(!idok(L"홍."));
for(int i=0;i<26;i++){W d[]={(W)('A'+i),':',0};CHECK(letterok(d));}CHECK(!letterok(L"/:"));CHECK(!letterok(L"XYZ"));
CHECK(subnet(L"192.168.0.0/24"));CHECK(subnet(L"172.18.1.17/32"));CHECK(!subnet(L"0.0.0.0/0"));CHECK(!subnet(L"192.168.0.4/24"));CHECK(!subnet(L"8.8.0.0/16"));CHECK(!subnet(L"10.0.0.0/8"));CHECK(!subnet(L"192.168.0.0/33"));
CHECK(!nativeuser(L"guest",pass,out,CNT(out))&&!Na);fadd=2224;CHECK(!nativeuser(L"alpha",pass,out,CNT(out))&&!Nenable&&!Ndel);fadd=0;flsa=5;CHECK(!nativeuser(L"alpha",pass,out,CNT(out))&&!Nenable&&Ndel==1);flsa=0;CHECK(nativeuser(L"alpha",pass,out,CNT(out))&&Nenable==1);W shortpw[]=L"7";CHECK(nativeuser(L"한글 아이디",shortpw,out,CNT(out)));fadd=2245;CHECK(!nativeuser(L"_강사",shortpw,out,CNT(out)));fadd=0;
shareowner=0;CHECK(!rights(L"자료",L"alpha",2,out,CNT(out))&&!added[1]);shareowner=1;activeuser=0;CHECK(!rights(L"자료",L"alpha",2,out,CNT(out))&&!added[1]);activeuser=1;
CHECK(rights(L"자료",L"alpha",2,out,CNT(out))&&removed[0]==1&&added[1]==1&&sessions==1);CHECK(rights(L"자료",L"alpha",1,out,CNT(out))&&removed[1]==1&&added[0]==1&&sessions==2);CHECK(rights(L"자료",L"alpha",0,out,CNT(out))&&removed[0]==2&&removed[1]==2&&sessions==3);modeerr=5;CHECK(!rights(L"자료",L"alpha",1,out,CNT(out))&&sessions==4);
occupied=1;CHECK(!mapdrive(&ep,L"자료",L"alpha",L"X:",pass,1,out,CNT(out))&&!addedMap&&!creds);occupied=0;existing=2;CHECK(!mapdrive(&ep,L"자료",L"alpha",L"X:",pass,1,out,CNT(out))&&!addedMap&&!cancelled);existing=1;cp(fakeowner,CNT(fakeowner),L"OTHER\\user");CHECK(!mapdrive(&ep,L"자료",L"alpha",L"X:",pass,1,out,CNT(out))&&!creds);existing=0;openok=0;CHECK(!mapdrive(&ep,L"자료",L"alpha",L"X:",pass,1,out,CNT(out))&&cancelled==0&&!creds);openok=1;maprc=1219;CHECK(!mapdrive(&ep,L"자료",L"alpha",L"X:",pass,1,out,CNT(out))&&!creds);maprc=0;CHECK(mapdrive(&ep,L"자료",L"alpha",L"X:",pass,1,out,CNT(out))&&creds==1&&registered==1);CHECK(eq(gotDest,L"\\\\192.168.1.90\\자료")&&eq(gotUser,L"NASBOX\\alpha")&&eq(gotProfile,L"CZ5|home.example.net|192.168.1.90|NASBOX|자료|alpha"));CHECK(!mapdrive(&ep,L"자료",L"alpha",L"Y:",L"",1,out,CNT(out))&&creds==1);
printf("%d policy / operation checks passed (Win32 boundaries simulated)\n",cases);return 0;}
