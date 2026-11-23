/* Calls the production click handlers with an untrusted/unsigned self-result.
   Win32 and operation backends are simulated. No host changes are made. */
#include "base.h"
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include "../src/ui.c"
static int assertions,accept=1,prompts,queued,runs,quitcode,autoreconnect;
static jmp_buf escape;
static W selItems[6][27][80];static int countSel[6],selected[6];
static int workShares,workMaps,workRights,vpnOK=1,wmode,whome;
static W seenName[100],seenWhere[260],seenVol[72],seenWho[80],seenLetter[5];
static W textfield[6][260],asked[850];static int volRow=-1;static N itemKey[26];static int volItems;
static int control(H a){N k=(N)a;return k>=400&&k<406?(int)k-400:-1;}
static int contains(T a,T b){int n=sz(b);for(;*a;a++){int i=0;while(i<n&&a[i]&&a[i]==b[i])i++;if(i==n)return 1;}return 0;}

#define CHECK(v) do{assertions++;if(!(v)){fprintf(stderr,"UI approval failed, role %d line %d: %s\n",SERVER,__LINE__,#v);exit(1);}}while(0)
int _snwprintf(W*b,size_t n,T f,...){(void)b;(void)n;(void)f;return 0;}
H CreateWindowExW(U32 a,T b,T c,U32 d,I32 e,I32 f,I32 g,I32 h,H i,H j,H k,H l){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;(void)i;(void)k;(void)l;int ix=control(j);if(ix>=0){selected[ix]=-1;countSel[ix]=0;textfield[ix][0]=0;}return ix>=0?j:(H)1;}
H CreateThread(H a,size_t b,U32(*c)(H),H d,U32 e,U32*f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;queued++;return (H)1;}
I32 CloseHandle(H a){(void)a;return 1;}
I32 DestroyWindow(H a){(void)a;return 1;}I32 EnableWindow(H a,I32 b){(void)a;(void)b;return 1;}
N SendMessageW(H a,U32 b,uintptr_t c,N d){
 if(control(a)==5){
 if(b==0x14b){volItems=0;volRow=-1;}else if(b==0x143)return volItems==26?-1:volItems++;
 else if(b==0x151&&c<26)itemKey[c]=d;else if(b==0x14e)volRow=(int)c;else if(b==0x147)return volRow;
 else if(b==0x150)return c<(U32)volItems?itemKey[c]:-1;
 }else {int i=control(a);if(i>=0){
 if(b==0x143&&countSel[i]<27){cp(selItems[i][countSel[i]],80,(T)d);return countSel[i]++;}
 if(b==0x14e)selected[i]=(int)c;else if(b==0x147)return selected[i];
 else if(b==0x149||b==0x148){if(c>=(U32)countSel[i])return -1;T v=selItems[i][c];if(b==0x148)cp((W*)d,sz(v)+1,v);return sz(v);}
 }}return 0;}
I32 GetWindowTextW(H a,W*b,I32 n){int i=control(a);cp(b,n,i<0?L"":textfield[i]);return sz(b);}
I32 GetWindowTextLengthW(H a){int i=control(a);return i<0?0:sz(textfield[i]);}I32 SetWindowTextW(H a,T b){int i=control(a);if(i>=0)cp(textfield[i],260,b);return 1;}
I32 MessageBoxW(H a,T b,T c,U32 d){(void)a;(void)c;cp(asked,CNT(asked),b);CHECK(d==0x124);prompts++;return accept?6:7;}
I32 PostMessageW(H a,U32 b,uintptr_t c,N d){(void)a;(void)b;(void)c;(void)d;return 1;}
int makeguide(W*a,int b){(void)a;(void)b;return 1;}
int getstr(H a,T b,T c,W*d,int e){(void)a;(void)b;(void)c;(void)d;(void)e;return 0;}
int putstr(H a,T b,T c,T d){(void)a;(void)b;(void)c;(void)d;return 1;}
int loadendpoint(H a,T b,Endpoint*e){(void)a;(void)b;cp(e->host,254,L"home.example.net");cp(e->ip,16,L"192.168.1.25");cp(e->machine,16,L"NASBOX");return 1;}
int saveendpoint(H a,T b,const Endpoint*c){(void)a;(void)b;(void)c;return 1;}
int cmdlet(T a,W*b,int c){(void)a;(void)b;(void)c;return 1;}
H brandicon(H i,int small){(void)i;(void)small;return (H)1;}
int trusted(void){return 0;}H GetModuleHandleW(T a){(void)a;return (H)1;}I32 CoInitializeEx(H a,U32 b){(void)a;(void)b;return 0;}
H GlobalAlloc(U32 a,size_t b){(void)a;return malloc(b);}H GlobalLock(H a){return a;}I32 GlobalUnlock(H a){(void)a;return 1;}H GlobalFree(H a){free(a);return 0;}I32 OpenClipboard(H a){(void)a;return 1;}I32 EmptyClipboard(void){return 1;}H SetClipboardData(U32 a,H b){(void)a;return b;}I32 CloseClipboard(void){return 1;}
H GetStockObject(I32 a){(void)a;return (H)1;}
H CreateFontW(I32 a,I32 b,I32 c,I32 d,I32 e,U32 f,U32 g,U32 h,U32 i,U32 j,U32 k,U32 l,U32 m,T n){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;(void)i;(void)j;(void)k;(void)l;(void)m;(void)n;return (H)1;}
H LoadCursorW(H a,T b){(void)a;(void)b;return (H)1;}U16 RegisterClassExW(Class*a){(void)a;return 1;}
I32 ShowWindow(H a,I32 b){(void)a;(void)b;return 1;}I32 UpdateWindow(H a){(void)a;return 1;}I32 GetMessageW(Msg*a,H b,U32 c,U32 d){(void)a;(void)b;(void)c;(void)d;return 0;}I32 IsDialogMessageW(H a,Msg*b){(void)a;(void)b;return 0;}I32 TranslateMessage(Msg*a){(void)a;return 1;}N DispatchMessageW(Msg*a){(void)a;return 0;}N DefWindowProcW(H a,U32 b,uintptr_t c,N d){(void)a;(void)b;(void)c;(void)d;return 0;}void PostQuitMessage(I32 a){(void)a;}
void ExitProcess(U32 c){quitcode=c;longjmp(escape,1);}
#if SERVER
static int settingCalls,lastMode;
int server_snapshot(W*b,int n){return cp(b,n,L"test snapshot");}int server_service(int mode,W*b,int n){settingCalls++;lastMode=mode;return cp(b,n,L"test result");}
void tray_init(void){}void tray_done(H h){(void)h;}void tray_event(H h,U32 m,N l){(void)h;(void)m;(void)l;}int tray_hide(H h){(void)h;return 0;}
int choices(int a,W*b,int c){(void)a;return cp(b,c,L"");}int byuser(T a,W*b,int c){(void)a;(void)b;(void)c;return 1;}
static T chosenFolder;
int datavolumes(VolumePick*r,int cap){if(cap<2)return 0;cp(r[0].root,4,L"D:\\");cp(r[0].id,72,L"vol-D");cp(r[0].label,64,L"자료");cp(r[1].root,4,L"F:\\");cp(r[1].id,72,L"vol-F");cp(r[1].label,64,L"백업");return 2;}
H SHBrowseForFolderW(Browse*a){(void)a;return chosenFolder?(H)7:0;}I32 SHGetPathFromIDListW(H a,W*b){(void)a;return cp(b,260,chosenFolder);}void CoTaskMemFree(H a){(void)a;}
I32 GetComputerNameW(W*b,U32*n){return cp(b,*n,L"NASBOX");}
int listip(W*a,int b){(void)a;(void)b;return 1;}int islocal(T a){(void)a;return 1;}
int inventory(W*a,int b){(void)a;(void)b;return 1;}int shareaction(T a,T b,int c,T volume,W*d,int e){(void)c;workShares++;cp(seenName,100,a);cp(seenWhere,260,b);cp(seenVol,72,volume);return cp(d,e,L"sample");}
int nativeuser(T a,W*b,W*c,int d){(void)a;(void)b;(void)c;(void)d;return 1;}int changepw(T a,W*b,W*c,int d){(void)a;(void)b;(void)c;(void)d;return 1;}int rights(T a,T b,int c,W*d,int e){workRights++;cp(seenName,100,a);cp(seenWho,80,b);wmode=c;return cp(d,e,L"sample");}int firewall(T a,T b,W*c,int d){(void)a;(void)b;(void)c;(void)d;return 1;}
#else
int getprofile(W*a,int b){(void)a;(void)b;return 0;}int getmap(T a,W*b,int c){(void)a;(void)b;(void)c;return 0;}
int vpninstall(const Endpoint*a,T b,W*c,W*d,W*e,int f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return 1;}int forgetvpn(W*a,int b){(void)a;(void)b;return 1;}
int connectnas(const Endpoint*a,int b,W*c,int d){(void)a;(void)c;(void)d;whome=b;return vpnOK;}
int mapdrive(const Endpoint*a,T b,T c,T d,W*e,int f,W*g,int h){(void)a;CHECK(f==1&&eq(e,L"sample-only"));workMaps++;cp(seenName,100,b);cp(seenWho,80,c);cp(seenLetter,5,d);return cp(g,h,L"sample");}int scanonly(const Endpoint*a,T b,T c,W*d,int e){(void)a;(void)b;(void)c;(void)d;(void)e;return 1;}
int detach(T a,W*b,int c){(void)a;(void)b;(void)c;return 1;}int restore(W*a,int b){runs++;return cp(a,b,L"restored");}int startup_legacy(void){return 0;}int setstartup(int a,W*b,int c){(void)a;(void)b;(void)c;return 1;}
int driveprobe(const Endpoint*a,T b,W*c,int d){(void)a;(void)b;(void)c;(void)d;return 1;}
W*GetCommandLineW(void){return L"client.exe /reconnect";}W**CommandLineToArgvW(T a,I32*n){(void)a;static W*v[]={L"client.exe",L"/reconnect"};*n=autoreconnect?2:1;return v;}H LocalFree(H a){(void)a;return 0;}
H CreateMutexW(Security*a,I32 b,T c){(void)a;(void)b;(void)c;return (H)1;}U32 GetLastError(void){return 0;}I32 ReleaseMutex(H a){(void)a;return 1;}void Sleep(U32 n){(void)n;}int transient5(void){return 0;}
#endif
static void testclick(int op){verified=0;busy=0;accept=0;int q=queued,p=prompts;act(op);CHECK(prompts==p+1);CHECK(queued==q&&!busy);accept=1;act(op);CHECK(prompts==p+2);CHECK(queued==q+1&&busy&&job.op==op);busy=0;erase(&job,sizeof(job));}
int main(void){
#if SERVER
page(1);CHECK(volItems==2&&volRow==-1&&!pickedVolume[0]&&!textfield[0][0]);int p0=prompts,q0=queued;act(ADD);CHECK(prompts==p0&&queued==q0); /* No default volume */
volRow=1;proc(0,0x111,405|((uintptr_t)1<<16),(N)field[5]);CHECK(eq(textfield[0],L"F:\\")&&eq(pickedVolume,L"vol-F")&&queued==q0);act(ADD);CHECK(prompts==p0&&queued==q0);
pre(1,L"백업자료");testclick(ADD);CHECK(contains(asked,L"드라이브 전체")&&contains(asked,L"F:\\")&&contains(asked,L"백업자료"));
busy=0;act(ADD);CHECK(busy&&eq(job.volume,L"vol-F")&&eq(job.v[0],L"F:\\")&&eq(job.v[1],L"백업자료"));dowork(0);busy=0;CHECK(workShares==1&&eq(seenWhere,L"F:\\")&&eq(seenName,L"백업자료")&&eq(seenVol,L"vol-F"));
chosenFolder=L"D:\\강의";act(PICK);CHECK(volRow==-1&&eq(textfield[0],L"D:\\강의")&&!pickedVolume[0]);testclick(ADD);CHECK(contains(asked,L"이 폴더")&&contains(asked,L"D:\\강의"));
chosenFolder=L"D:\\";act(PICK);CHECK(!textfield[0][0]&&!pickedVolume[0]);q0=queued;act(ADD);CHECK(queued==q0);
volRow=0;pickwhole();act(VOLREFRESH);CHECK(volRow==-1&&!textfield[0][0]&&!pickedVolume[0]); /* Explicit refresh invalidates whole-disk selection. */
pre(1,L"only-test");testclick(REMOVE);testclick(USER);pre(1,L"only-test");testclick(PWRESET);testclick(RIGHT);testclick(FWR);testclick(FWDEL);page(2);pre(0,L"demo");pre(2,L"자료창고");
for(int z=0;z<=2;z++){SendMessageW(field[3],0x14e,z,0);act(RIGHT);dowork(0);busy=0;CHECK(workRights==z+1&&wmode==z&&eq(seenWho,L"demo")&&eq(seenName,L"자료창고"));}
if(!setjmp(escape))start();CHECK(np==5&&quitcode==0); /* startup displays real status before any change */
testclick(BOOTSHARE);testclick(MANUALSHARE);act(BOOTSHARE);dowork(0);busy=0;CHECK(settingCalls==1&&lastMode==1);act(MANUALSHARE);dowork(0);busy=0;CHECK(settingCalls==2&&lastMode==0);
#else
testclick(VPN);testclick(DELVPN);testclick(MAP);testclick(UNMAP);testclick(AUTOON);testclick(AUTOOFF);testclick(TEST);
page(2);pre(0,L"demo");pre(2,L"자료창고");SendMessageW(field[3],0x14e,23,0);
pre(1,L"sample-only");vpnOK=0;act(MAP);dowork(0);busy=0;CHECK(!workMaps&&!whome);
for(int z=0;z<2;z++){vpnOK=1;SendMessageW(field[3],0x14e,23+z,0);pre(1,L"sample-only");act(MAP);dowork(0);busy=0;CHECK(workMaps==z+1&&eq(seenLetter,z?L"Y:":L"X:")&&eq(seenWho,L"demo")&&eq(seenName,L"자료창고")&&!textfield[1][0]);}
autoreconnect=1;if(!setjmp(escape))start();CHECK(runs==1&&quitcode==0);
#endif
printf("%d actual UI approval/dispatch checks passed for %s (Windows APIs simulated).\n",assertions,SERVER?"server":"client");return 0;
}
