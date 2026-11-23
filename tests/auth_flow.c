/* Production client operations with separate fake logon sessions.
   These are control-flow regressions, not tests on Windows or over the Internet. */
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
static int checks,isolated,tokens,mainCancels,tempCancels,credWrites,regWrites;
static int mapped=1,proofOpen=1,normalOpen=1,logonOk=1,impOk=1,revertOk=1;
static int enumError,tempAddError,cleanupError,tokenCloses,terminated;
static W supplied[257],owner[80]=L"NASBOX\\alpha",out[2048];
static Endpoint ep={L"nas.example.net",L"192.168.1.90",L"NASBOX"};
static jmp_buf fatal;
#define CHECK(x) do { checks++; if(!(x)){fprintf(stderr,"auth regression line %d: %s\n",__LINE__,#x);exit(1);} } while(0)
int _snwprintf(W*b,size_t cap,T fmt,...){va_list ap;va_start(ap,fmt);size_t n=0;for(;*fmt&&n+1<cap;fmt++){if(*fmt!='%'){b[n++]=*fmt;continue;}fmt++;if(*fmt=='l'&&fmt[1]=='s'){fmt++;T v=va_arg(ap,T);while(*v&&n+1<cap)b[n++]=*v++;}else if(*fmt=='u'){U32 v=va_arg(ap,U32);char s[30];snprintf(s,sizeof(s),"%u",v);for(char*p=s;*p&&n+1<cap;p++)b[n++]=*p;}}b[n]=0;va_end(ap);return n;}
int errnum(W*b,int cap,T why,U32 e){(void)e;cp(b,cap,why);return 0;}
U32 GetLastError(void){return 1326;}
I32 LogonUserW(T id,T domain,T pass,U32 kind,U32 provider,H*token){CHECK(eq(id,L"alpha")&&eq(domain,L"NASBOX")&&kind==9&&provider==3);cp(supplied,CNT(supplied),pass);if(!logonOk)return 0;tokens++;*token=(H)11;return 1;}
I32 ImpersonateLoggedOnUser(H token){CHECK(token==(H)11);if(!impOk)return 0;isolated=1;return 1;}
I32 RevertToSelf(void){CHECK(isolated);if(!revertOk)return 0;isolated=0;return 1;}
void ExitProcess(U32 status){terminated=status;longjmp(fatal,1);}
I32 CloseHandle(H h){if(h==(H)11){tokens--;tokenCloses++;}return 1;}
H CreateFileW(T path,U32 access,U32 share,H sa,U32 disposition,U32 flags,H template){(void)path;(void)access;(void)share;(void)sa;(void)disposition;(void)flags;(void)template;return (isolated?(proofOpen&&eq(supplied,L"correct-test-password")):normalOpen)?(H)21:BAD;}
U32 WNetGetConnectionW(T drive,W*b,U32*n){(void)drive;if(!mapped)return 2250;cp(b,*n,L"\\\\192.168.1.90\\자료");return 0;}
U32 WNetGetUserW(T drive,W*b,U32*n){(void)drive;cp(b,*n,owner);return 0;}
U32 WNetAddConnection2W(Resource*r,T pass,T user,U32 flags){CHECK(!flags&&pass&&user);if(isolated){CHECK(r->local==0);return tempAddError;}CHECK(r->local!=0);mapped=1;return 0;}
U32 WNetCancelConnection2W(T resource,U32 flags,I32 force){CHECK(!flags&&!force);if(isolated){CHECK(eq(resource,L"\\\\192.168.1.90\\IPC$"));tempCancels++;return cleanupError;}mainCancels++;mapped=0;return 0;}
U32 GetLogicalDrives(void){return 0;}
I32 CredWriteW(Cred*c,U32 flags){CHECK(!isolated&&!flags&&c->bytes&&c->blob);credWrites++;return 1;}
I32 CredDeleteW(T a,U32 b,U32 c){(void)a;(void)b;(void)c;CHECK(!isolated);return 1;}
int putstr(H a,T b,T c,T d){(void)a;(void)b;(void)c;(void)d;CHECK(!isolated);regWrites++;return 1;}
int delstr(H a,T b,T c){(void)a;(void)b;(void)c;regWrites++;return 1;}
int getstr(H a,T b,T name,W*value,int cap){(void)a;(void)b;if(!eq(name,L"Drive-X"))return 0;return cp(value,cap,L"CZ5|nas.example.net|192.168.1.90|NASBOX|자료|alpha");}
int loadendpoint(H a,T b,Endpoint*e){(void)a;(void)b;*e=ep;return 1;}
U32 NetShareEnum(T server,U32 level,U8**buf,U32 size,U32*n,U32*total,U32*resume){(void)server;(void)size;(void)resume;static Share1 shares[]={{L"자료",0,L"ChaZeumNAS5:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"},{L"Unmanaged",0,L"other"}};CHECK(level==1);*buf=enumError?0:(U8*)shares;*n=*total=enumError?0:2;return enumError;}
U32 NetApiBufferFree(H p){(void)p;return 0;}
static void reset(void){isolated=tokens=mainCancels=tempCancels=credWrites=regWrites=tokenCloses=terminated=0;mapped=proofOpen=normalOpen=logonOk=impOk=revertOk=1;enumError=tempAddError=cleanupError=0;supplied[0]=0;cp(owner,CNT(owner),L"NASBOX\\alpha");}
static void wrongpass(void){reset();W wrong[]=L"wrong-test-password";CHECK(!mapdrive(&ep,L"자료",L"alpha",L"X:",wrong,1,out,CNT(out)));CHECK(!credWrites&&!regWrites&&!mainCancels&&mapped&&!isolated&&!tokens);}
static void scanisolated(void){reset();CHECK(scanonly(&ep,L"alpha",L"correct-test-password",out,CNT(out)));CHECK(!mainCancels&&tempCancels==1&&!isolated&&!tokens&&tokenCloses==1);}
static void detachowner(void){reset();cp(owner,CNT(owner),L"NASBOX\\someone-else");CHECK(!detach(L"X:",out,CNT(out)));CHECK(!mainCancels&&!regWrites&&mapped);}
int main(int argc,char**argv){if(argc==2){if(!strcmp(argv[1],"wrong-password"))wrongpass();else if(!strcmp(argv[1],"scan-session"))scanisolated();else if(!strcmp(argv[1],"detach-owner"))detachowner();else return 2;return 0;}
wrongpass();scanisolated();detachowner();
reset();W correct[]=L"correct-test-password";CHECK(mapdrive(&ep,L"자료",L"alpha",L"X:",correct,1,out,CNT(out)));CHECK(credWrites==1&&regWrites==1&&!isolated&&!tokens&&!mainCancels);
reset();mapped=0;CHECK(mapdrive(&ep,L"자료",L"alpha",L"X:",correct,1,out,CNT(out)));CHECK(mapped&&credWrites==1&&!isolated&&!tokens);
reset();mapped=0;normalOpen=0;CHECK(!mapdrive(&ep,L"자료",L"alpha",L"X:",correct,1,out,CNT(out)));CHECK(mainCancels==1&&!mapped&&!credWrites&&!regWrites);
reset();proofOpen=0;CHECK(!mapdrive(&ep,L"자료",L"alpha",L"X:",correct,1,out,CNT(out)));CHECK(mapped&&!credWrites&&!regWrites&&!isolated&&!tokens);
reset();logonOk=0;CHECK(!mapdrive(&ep,L"자료",L"alpha",L"X:",correct,1,out,CNT(out)));CHECK(!credWrites&&!tokens&&!isolated);
reset();impOk=0;CHECK(!mapdrive(&ep,L"자료",L"alpha",L"X:",correct,1,out,CNT(out)));CHECK(!credWrites&&!tokens&&!isolated&&tokenCloses==1);
reset();enumError=5;CHECK(!scanonly(&ep,L"alpha",correct,out,CNT(out)));CHECK(tempCancels==1&&!mainCancels&&!isolated&&!tokens);
reset();tempAddError=1326;CHECK(!scanonly(&ep,L"alpha",correct,out,CNT(out)));CHECK(!mainCancels&&!tempCancels&&!isolated&&!tokens);
reset();cleanupError=2401;CHECK(!scanonly(&ep,L"alpha",correct,out,CNT(out)));CHECK(!mainCancels&&!isolated&&!tokens);
reset();CHECK(detach(L"X:",out,CNT(out)));CHECK(mainCancels==1&&regWrites==1&&!mapped);
reset();revertOk=0;if(!setjmp(fatal)){mapdrive(&ep,L"자료",L"alpha",L"X:",correct,1,out,CNT(out));CHECK(0);}CHECK(terminated==5&&!credWrites&&!regWrites&&!tokens);
printf("%d credential isolation, password rejection, cleanup and drive ownership checks passed (Windows APIs simulated).\n",checks);return 0;}
