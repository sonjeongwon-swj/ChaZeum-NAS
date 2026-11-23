#include "base.h"
#include <stdio.h>
#include <stdlib.h>
static W autoCmd[1100],marker[1024],result[500];
static T self=L"C:\\My NAS\\Chazeum-NAS_USER.exe";
static int failRun,permitted=3;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"startup check failed at %d\n",__LINE__);exit(2);}}while(0)
static W*slot(T k,T n){if(eq(k,CLIKEY)&&eq(n,L"StartupCopy"))return marker;if(eq(n,L"ChaZeumNAS5"))return autoCmd;return 0;}
int getstr(H r,T k,T n,W*b,int z){(void)r;W*p=slot(k,n);return p&&p[0]&&cp(b,z,p);}
int putstr(H r,T k,T n,T s){(void)r;W*p=slot(k,n);if(p==autoCmd&&failRun)return 0;return p&&cp(p,p==marker?1024:1100,s);}
int delstr(H r,T k,T n){(void)r;W*p=slot(k,n);CHECK(p);p[0]=0;return 1;}
U32 GetModuleFileNameW(H i,W*b,U32 z){(void)i;CHECK(cp(b,z,self));return sz(b);}
U32 GetDriveTypeW(T d){(void)d;return permitted;}
I32 SHGetFolderPathW(H w,I32 v,H u,U32 f,W*b){(void)w;(void)v;(void)u;(void)f;cp(b,260,L"C:\\Home\\AppData\\Local");return 0;}
int main(void){
 CHECK(!startup_legacy()&&!autoCmd[0]);CHECK(setstartup(1,result,500)&&eq(marker,self));
 CHECK(eq(autoCmd,L"\"C:\\My NAS\\Chazeum-NAS_USER.exe\" /reconnect"));CHECK(!startup_legacy());
 CHECK(setstartup(1,result,500)&&setstartup(0,result,500)&&!autoCmd[0]);
 T old=L"C:\\Home\\AppData\\Local\\ChaZeumNAS5\\Client-0123456789abcdef0123456789abcdef.exe";
 cp(marker,1024,old);cp(autoCmd,1100,L"\"");cat(autoCmd,1100,old);cat(autoCmd,1100,L"\" /reconnect");
 CHECK(startup_legacy());failRun=1;CHECK(!setstartup(1,result,500)&&eq(marker,old)&&startup_legacy());
 failRun=0;CHECK(setstartup(1,result,500)&&eq(marker,self)&&!startup_legacy());
 cp(autoCmd,1100,L"unrelated.exe");CHECK(!setstartup(1,result,500)&&!setstartup(0,result,500)&&eq(autoCmd,L"unrelated.exe"));
 autoCmd[0]=0;self=L"Z:\\Other.exe";CHECK(!setstartup(1,result,500)&&!autoCmd[0]);
 self=L"Z:\\Chazeum-NAS_USER.exe";permitted=4;CHECK(!setstartup(1,result,500)&&!autoCmd[0]);
 puts("Opt-in reconnect: old copy migration, rollback, two same-location upgrades, cancellation/foreign registrations, local path enforcement passed.");
}
