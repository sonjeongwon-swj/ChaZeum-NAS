#ifndef CZ5_BASE
#define CZ5_BASE
#include <stddef.h>
#include <stdint.h>
typedef uint32_t U32; typedef int32_t I32; typedef uint16_t U16; typedef uint8_t U8; typedef uint64_t U64; typedef intptr_t N; typedef void *H; typedef wchar_t W; typedef const W *T;
#define CNT(a) ((int)(sizeof(a)/sizeof((a)[0])))
#define BAD ((H)(N)-1)
#define HKCU ((H)(N)(I32)0x80000001)
#define HKLM ((H)(N)(I32)0x80000002)
#define PRODUCT L"차즘 NAS 5.0"
#define SRVKEY L"Software\\ChazeumNAS\\Five\\Server"
#define CLIKEY L"Software\\ChazeumNAS\\Five\\Client"
int rootpath(T); H brandicon(H inst,int small);
#define TITLE_SERVER L"Chazeum-NAS_SERVER 5.0.1.0 · 서버용"
#define TITLE_CLIENT L"Chazeum-NAS_USER 5.0.1.0 · 유저용"
#define MARK L"ChaZeumNAS5 network account"
typedef struct {U32 a; U16 b,c; U8 d[8];} Guid;
typedef struct {H window; U32 msg; uintptr_t wp; N lp; U32 time; I32 x,y; U32 spare;} Msg;
typedef struct {U32 length,style; N(*handler)(H,U32,uintptr_t,N); I32 a,b; H instance,icon,cursor,brush;T menu,cls;H small;} Class;
typedef struct {U32 n;W *res,*desktop,*title;U32 x,y,width,height,cx,cy,fill,flags;U16 show,num;U8*bytes;H input,output,error;} Startup;
typedef struct {H process,thread;U32 pid,tid;} Process;
typedef struct {U32 n;H sd;I32 inherit;} Security;
typedef struct {H owner,root;W *display;T title;U32 flags;H cb;N arg;I32 image;} Browse;
typedef struct {U32 size;T path;H file;const Guid *type;} TrustFile;
typedef struct {U32 size;H cb,sip;U32 ui,revoke,choice;H file;U32 action;H state;W *url;U32 flags,context;H sig;} TrustData;
typedef struct {U32 flags,type;W *target,*comment;U64 time;U32 bytes;U8 *blob;U32 persist,count;H attrs;W *alias,*user;} Cred;
typedef struct {U32 scope,type,display,use;W *local,*remote,*comment,*provider;} Resource;
typedef struct {W *name,*password;U32 age,priv;W *home,*comment;U32 flags;W *script;} User1;
typedef struct {U16 len,max;W *data;} LsaStr;
typedef struct {U32 len;H root,name;U32 attr;H sd,qos;} LsaObj;
typedef struct {U32 size,mask;W user[257],pass[257],domain[16];} RasCred;
typedef struct {U32 size;W entry[257],phone[129],back[129],user[257],pass[257],domain[16];U32 sub;uintptr_t cb;U32 ifIndex;W *enc;} RasParams;
typedef struct {W *name;U32 type;W *remark;} Share1;
extern void *memset(void*,int,size_t); extern void *memcpy(void*,const void*,size_t); extern int memcmp(const void*,const void*,size_t);
extern int _snwprintf(W*,size_t,T,...); extern int _wcsicmp(T,T);
extern H GetModuleHandleW(T),GetProcessHeap(void),HeapAlloc(H,U32,size_t),CreateThread(H,size_t,U32(*)(H),H,U32,U32*);
extern I32 HeapFree(H,U32,H),CloseHandle(H),ReadFile(H,H,U32,U32*,H),WriteFile(H,const void*,U32,U32*,H),CreatePipe(H*,H*,Security*,U32),SetHandleInformation(H,U32,U32);
extern U32 GetModuleFileNameW(H,W*,U32),GetSystemDirectoryW(W*,U32),GetLastError(void),WaitForSingleObject(H,U32),GetLogicalDrives(void),GetFileAttributesW(T);
extern I32 CreateProcessW(T,W*,H,H,I32,U32,H,T,Startup*,Process*),GetExitCodeProcess(H,U32*),GetComputerNameW(W*,U32*),DeleteFileW(T),FlushFileBuffers(H),GetFileSizeEx(H,int64_t*);extern H CreateFileW(T,U32,U32,H,U32,U32,H);
extern void ExitProcess(U32),Sleep(U32);extern U64 GetTickCount64(void); extern H VirtualAlloc(H,size_t,U32,U32);extern I32 VirtualFree(H,size_t,U32);
extern W *GetCommandLineW(void);extern W **CommandLineToArgvW(T,I32*);extern H LocalFree(H);
extern U32 RegOpenKeyExW(H,T,U32,U32,H*),RegCreateKeyExW(H,T,U32,W*,U32,U32,H,H*,U32*),RegCloseKey(H),RegGetValueW(H,T,T,U32,U32*,H,U32*),RegSetValueExW(H,T,U32,U32,const U8*,U32),RegDeleteValueW(H,T);
extern I32 WinVerifyTrust(H,const Guid*,TrustData*),CoInitializeEx(H,U32),CoCreateGuid(Guid*);extern void CoTaskMemFree(H);
extern H GetStockObject(I32),CreateFontW(I32,I32,I32,I32,I32,U32,U32,U32,U32,U32,U32,U32,U32,T),LoadCursorW(H,T),CreateWindowExW(U32,T,T,U32,I32,I32,I32,I32,H,H,H,H);
extern U16 RegisterClassExW(Class*);extern I32 DestroyWindow(H),GetMessageW(Msg*,H,U32,U32),TranslateMessage(Msg*),ShowWindow(H,I32),UpdateWindow(H),IsDialogMessageW(H,Msg*),SetWindowTextW(H,T),GetWindowTextW(H,W*,I32),MessageBoxW(H,T,T,U32),PostMessageW(H,U32,uintptr_t,N),EnableWindow(H,I32);
extern H SetFocus(H);extern N DefWindowProcW(H,U32,uintptr_t,N),DispatchMessageW(Msg*),SendMessageW(H,U32,uintptr_t,N);extern void PostQuitMessage(I32);
extern H SHBrowseForFolderW(Browse*);extern I32 SHGetPathFromIDListW(H,W*);extern H ShellExecuteW(H,T,T,T,T,I32);
extern H GlobalAlloc(U32,size_t),GlobalLock(H),GlobalFree(H),SetClipboardData(U32,H);extern I32 OpenClipboard(H),EmptyClipboard(void),GlobalUnlock(H),CloseClipboard(void);
extern U32 NetUserAdd(T,U32,U8*,U32*),NetUserDel(T,T),NetUserSetInfo(T,T,U32,U8*,U32*),NetShareEnum(T,U32,U8**,U32,U32*,U32*,U32*),NetApiBufferFree(H);
extern I32 LookupAccountNameW(T,T,H,U32*,W*,U32*,U32*);extern U32 LsaOpenPolicy(LsaStr*,LsaObj*,U32,H*),LsaAddAccountRights(H,H,LsaStr*,U32),LsaNtStatusToWinError(U32),LsaClose(H);
extern I32 CredWriteW(Cred*,U32),CredReadW(T,U32,U32,Cred**),CredDeleteW(T,U32,U32);extern void CredFree(H);
extern U32 RasSetCredentialsW(T,T,RasCred*,I32),RasGetCredentialsW(T,T,RasCred*),RasDialW(H,T,RasParams*,U32,H,H*),RasHangUpW(H);
extern U32 WNetAddConnection2W(Resource*,T,T,U32),WNetCancelConnection2W(T,U32,I32),WNetGetConnectionW(T,W*,U32*);
extern I32 BCryptGenRandom(H,U8*,U32,U32);extern I32 VirtualLock(H,size_t),VirtualUnlock(H,size_t);
extern H CreateMutexW(Security*,I32,T);extern I32 ReleaseMutex(H);extern I32 MultiByteToWideChar(U32,U32,const char*,I32,W*,I32);
extern I32 IsUserAnAdmin(void);
_Static_assert(sizeof(W)==2 && sizeof(void*)==8 && sizeof(Msg)==48 && sizeof(Class)==80 && sizeof(Cred)==80 && sizeof(Startup)==104 && sizeof(TrustData)==88 && sizeof(User1)==56,"Win64 ABI");
int sz(T);int cp(W*,int,T);int cat(W*,int,T);int eq(T,T);void erase(void*,size_t);int ipprivate(T);int hostok(T);int machineok(T);int idok(T);int shareok(T);int letterok(T);int pair(T,W*,W*,W*);int invite(W*,int,T,T,T);
typedef struct {W host[254],ip[16],machine[16];} Endpoint;
int samehost(T,T); int tailip(T); int securepin(T); int securetarget(T,T);
int pinroute(Endpoint*,W*,int); int checkpin(const Endpoint*,W*,int);
int getstr(H,T,T,W*,int);int putstr(H,T,T,T);int delstr(H,T,T);int trusted(void);int loadendpoint(H,T,Endpoint*);int saveendpoint(H,T,const Endpoint*);int validend(const Endpoint*);int errnum(W*,int,T,U32);
typedef struct {int n;T v[8];} Fields;
int shellop(T,const Fields*,W*,int);
extern const W *ops[];
enum {STATUS,SHAREADD,SHAREREMOVE,RIGHTS,SERVERNET,IPLIST,FWADD,VPNMAKE,VPNCHECK,VPNREMOVE,VPNINFO};
int nativeuser(T,W*,W*,int);int accountcreds(T,T,W*,int,int);
int vpninstall(const Endpoint*,T,W*,W*,W*,int);int connectnas(const Endpoint*,int,W*,int);int mapdrive(const Endpoint*,T,T,T,W*,int,W*,int);int scan(const Endpoint*,T,W*,W*,int);int detach(T,W*,int);int restore(W*,int);int driveprobe(const Endpoint*,T,W*,int);int startup_legacy(void);int setstartup(int,W*,int);int forgetvpn(W*,int);int guidtext(W*,int);
int makeguide(W*,int);void winmain(void);void start(void);
_Static_assert(sizeof(RasParams)==2128 && offsetof(RasParams,enc)==2120 && sizeof(RasCred)==1068,"RAS credentials ABI");
#endif
