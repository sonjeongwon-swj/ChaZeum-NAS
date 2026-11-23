#include "base.h"
#if SERVER
#include "admin.h"
#include "serverlife.h"
#else
#include "client.h"
#endif
extern I32 GetWindowTextLengthW(H);
static H win,inst,font,titlefont,kids[64],field[8],status,nav[6];static int nk,np,busy,verified,lastop;
static unsigned listOnly;
static Endpoint known;static W message[12288];
enum {TABS=100,PICK=202,SAVE=203,COPY=204,LIST=205,ADD=206,REMOVE=207,USER=208,RIGHT=209,IPLIST5=210,FWR=211,PASTE=212,VPN=213,DELVPN=214,DISC=215,MAP=216,UNMAP=217,RETRY=218,AUTOON=219,AUTOOFF=220,TEST=221,HELP=222,FWDEL=223,FWLIST=224,BYUSER=225,PWRESET=226,VOLREFRESH=227,SVCVIEW=228,BOOTSHARE=229,MANUALSHARE=230,TOTRAY=231};
typedef struct {int op,choice,home,remember;Endpoint e;W v[6][520];W volume[72];} Work;static Work job;
static H widget(T cls,T text,U32 plus,int x,int y,int w,int h,int id){H v=CreateWindowExW(eq(cls,L"EDIT")?0x200:0,cls,text,0x50000000u|plus,x,y,w,h,win,(H)(N)id,inst,0);if(v){if(nk<CNT(kids))kids[nk++]=v;SendMessageW(v,0x30,(uintptr_t)font,1);}return v;}
static H label(T t,int x,int y,int w,int h){return widget(L"STATIC",t,0,x,y,w,h,0);}
static H btn(T t,int x,int y,int w,int id){return widget(L"BUTTON",t,0x10000,x,y,w,35,id);}
static void line(int index,T name,int y,int secret,int max){label(name,25,y+4,164,25);field[index]=widget(L"EDIT",L"",0x10000|0x80|(secret?0x20:0),192,y,571,30,400+index);SendMessageW(field[index],0xc5,max,0);}
static void combo(int index,T name,int y){listOnly|=1u<<index;label(name,25,y+4,164,24);field[index]=widget(L"COMBOBOX",L"",0x10000|3|0x200000,192,y,240,225,400+index);}
static void item(int n,T t){SendMessageW(field[n],0x143,0,(N)t);}

static void pickline(int ix,T lbl,int y){label(lbl,25,y+4,164,24);field[ix]=widget(L"COMBOBOX",L"",0x10000|2|0x200000|0x40,192,y,560,210,400+ix);SendMessageW(field[ix],0x141,ix==0?500:60,0);}
static void addlines(int ix,W*data,int bullets){W*p=data;while(*p){while(*p=='\r'||*p=='\n')p++;if(!*p)break;W*q=p;while(*q&&*q!='\r'&&*q!='\n')q++;W end=*q;*q=0;if(bullets){if(*p==' '&&p[1]==0x2022&&p[2]==' '&&shareok(p+3))item(ix,p+3);}else item(ix,p);if(!end)break;p=q+1;}}
static void chk(int n,T t,int y){field[n]=widget(L"BUTTON",t,0x10000|3,27,y,740,27,400+n);}
static int checked(int n){return field[n]&&SendMessageW(field[n],0xf0,0,0)==1;}
static int val(int i,W*b,int n){if(!field[i]){*b=0;return 1;}if(listOnly&(1u<<i)){
 N row=SendMessageW(field[i],0x147,0,0);*b=0;if(row<0)return 1;
 N len=SendMessageW(field[i],0x149,(uintptr_t)row,0);if(len<0||len>=n)return 0;
 return SendMessageW(field[i],0x148,(uintptr_t)row,(N)b)==len;
 }if(GetWindowTextLengthW(field[i])>=n){*b=0;return 0;}GetWindowTextW(field[i],b,n);return 1;}
static void pre(int i,T s){if(field[i]&&s)SetWindowTextW(field[i],s);}
static void speak(T t){SetWindowTextW(status,t);}
static int ask(T s){return MessageBoxW(win,s,PRODUCT,0x124)==6;}
static int clip(T s){int n=2*(sz(s)+1);H h=GlobalAlloc(2,n);if(!h)return 0;H p=GlobalLock(h);if(!p){GlobalFree(h);return 0;}memcpy(p,s,n);GlobalUnlock(h);if(!OpenClipboard(win)){GlobalFree(h);return 0;}int ok=EmptyClipboard()&&SetClipboardData(13,h)!=0;CloseClipboard();if(!ok)GlobalFree(h);return ok;}
#if SERVER
/* Fixed index from CB_SETITEMDATA; neither volume label nor displayed path is a credential. */
static VolumePick drives[26];static int driveCount;static W pickedVolume[72];
static void loaddrives(void){
 SendMessageW(field[5],0x14b,0,0); /* CB_RESETCONTENT */
 driveCount=datavolumes(drives,CNT(drives));
 for(int i=0;i<driveCount;i++){
  W show[100];cp(show,CNT(show),drives[i].root);cat(show,CNT(show),L"   ");cat(show,CNT(show),drives[i].label);
  N n=SendMessageW(field[5],0x143,0,(N)show);if(n>=0)SendMessageW(field[5],0x151,(uintptr_t)n,i);
 }
 SendMessageW(field[5],0x14e,(uintptr_t)-1,0); /* CB_SETCURSEL: never pick a disk by default */
}
static void cleartarget(void){pickedVolume[0]=0;pre(0,L"");}
static void pickwhole(void){
 N row=SendMessageW(field[5],0x147,0,0),i=row<0?-1:SendMessageW(field[5],0x150,(uintptr_t)row,0);
 cleartarget();
 if(i<0||i>=driveCount){speak(L"전체를 공유할 데이터 드라이브를 목록에서 선택하세요.");return;}
 cp(pickedVolume,CNT(pickedVolume),drives[i].id);pre(0,drives[i].root);
 speak(L"드라이브 전체를 골랐습니다. 공유 이름을 정하고 ‘공유 추가’를 눌러야 공유가 시작됩니다.");
}
static int approveTarget(void){
 W path[260],name[64],text[850];
 if(!val(0,path,CNT(path))||!*path){speak(L"먼저 데이터 드라이브 또는 폴더를 선택하세요.");return 0;}
 if(!val(1,name,CNT(name))||!shareok(name)){speak(L"접속할 때 보일 공유 이름을 입력하세요. 예: 강의자료");return 0;}
 if(rootpath(path)&&!pickedVolume[0]){speak(L"드라이브 전체는 위 목록에서 직접 선택하세요.");return 0;}
 if(rootpath(path)){N row=SendMessageW(field[5],0x147,0,0),ix=row<0?-1:SendMessageW(field[5],0x150,(uintptr_t)row,0);
 if(ix<0||ix>=driveCount||!eq(drives[ix].root,path)||!eq(drives[ix].id,pickedVolume)){speak(L"드라이브 전체는 위 목록에서 다시 선택하세요.");return 0;}}
else if(pickedVolume[0]){pickedVolume[0]=0;SendMessageW(field[5],0x14e,(uintptr_t)-1,0);}
cp(text,CNT(text),rootpath(path)?L"이 드라이브 전체를 공유할까요?":L"이 폴더를 공유할까요?");
 cat(text,CNT(text),L"\r\n\r\n서버 위치: ");cat(text,CNT(text),path);cat(text,CNT(text),L"\r\n접속할 때 보이는 이름: ");cat(text,CNT(text),name);
 cat(text,CNT(text),L"\r\n\r\n이 위치와 권한을 상속하는 하위 항목에 NAS 권한을 추가합니다. 계정별 읽기·쓰기는 ‘3. 아이디·권한’에서 정합니다.");
 return ask(text);
}
#endif
#if SERVER
static H dashboard;
static void refreshdash(void){W b[640];if(np==5 && dashboard){server_snapshot(b,CNT(b));SetWindowTextW(dashboard,b);}}
#endif
static void page(int p){while(nk)DestroyWindow(kids[--nk]);memset(field,0,sizeof(field));listOnly=0;np=p;
#if SERVER
T tabs[]={L"1. 서버 주소",L"2. 공유할 위치",L"3. 아이디·권한",L"4. 연결 확인",L"사용 안내",L"서버 상태"};
T headings[]={L"집 밖 접속에 쓸 주소",L"드라이브 전체 또는 폴더만 공유",L"누가 어느 폴더를 쓸 수 있나요?",L"접속 상태와 방화벽",L"처음부터 보는 그림 안내",L"내 PC의 파일 공유 상태"};
T hint[]={L"집 밖 접속용 주소입니다. 집 안에서 폴더를 공유하려면 바로 ‘2. 공유할 위치’로 가세요.",L"드라이브를 고르거나 폴더를 찾아보세요. 폴더 경로를 붙여넣어도 됩니다. 공유 이름은 직접 정하세요.",L"NAS 전용 아이디를 만든 뒤, 아래에서 폴더별로 허용 범위를 정하세요.",L"먼저 기존 설정으로 시험하세요. 필요한 경우에만 사설 주소 범위를 직접 허용하세요.",L"공유기 설정, PC 드라이브 연결, 아이폰·아이패드 파일 앱을 그림으로 설명합니다.",L"파일은 Windows가 공유합니다. 이 창을 닫아도 이미 등록한 공유는 유지됩니다."};
#else
T tabs[]={L"1. 서버 주소",L"2. 외부 VPN",L"3. 드라이브",L"4. 확인·자동 연결",L"사용 안내"};
T headings[]={L"내가 연결할 집 PC",L"집 밖에서 집으로 가는 보안 연결",L"탐색기 드라이브 연결",L"자동 연결과 실제 복사 시험",L"처음부터 보는 그림 안내"};
T hint[]={L"서버에서 복사한 접속 정보를 붙여넣거나, 아래 세 칸을 직접 입력하세요.",L"공유기에 만든 VPN 계정을 한 번 등록하세요. NAS 폴더의 아이디와는 별개입니다.",L"서버가 공유한 드라이브나 폴더를 이 PC에서 쓸 빈 문자에 연결하세요. 서버 문자와 달라도 됩니다.",L"등록한 드라이브를 다시 연결하거나, 실제로 파일을 쓰고 읽어 속도를 볼 수 있습니다.",L"아이폰·아이패드에는 별도 파일 관리 앱 없이 기본 파일 앱을 씁니다."};
#endif
#if SERVER
dashboard=0;
static const int ord[]={5,0,1,2,3,4};
for(int k=0;k<6;k++){int i=ord[k];nav[i]=btn(tabs[i],16+k*127,17,122,TABS+i);EnableWindow(nav[i],i!=p);}
#else
for(int i=0;i<5;i++){nav[i]=btn(tabs[i],22+i*152,17,145,TABS+i);EnableWindow(nav[i],i!=p);}
#endif
H ttl=label(headings[p],25,71,745,38);SendMessageW(ttl,0x30,(uintptr_t)titlefont,1);label(hint[p],26,114,744,43);
if(p==0){
#if SERVER
line(0,L"공유기 도메인 / 공인 IP",169,0,128);line(1,L"3080 PC 집 안 IP (IPv4)",211,0,15);line(2,L"이 PC의 이름",253,0,15);W host[16];U32 len=16;if(GetComputerNameW(host,&len))pre(2,host);SendMessageW(field[2],0xcf,1,0);pre(0,known.host);pre(1,known.ip);btn(L"내 PC 주소 보기",26,307,158,IPLIST5);btn(L"주소 저장",197,307,142,SAVE);btn(L"접속 정보 복사",354,307,160,COPY);
#else
line(3,L"서버의 접속 정보",155,0,309);btn(L"입력칸에 채우기",193,192,164,PASTE);line(0,L"공유기 도메인 / 공인 IP",237,0,128);line(1,L"3080 PC 집 안 IPv4",278,0,15);line(2,L"서버 PC 이름",319,0,15);pre(0,known.host);pre(1,known.ip);pre(2,known.machine);btn(L"서버 주소 저장",193,362,160,SAVE);
#endif
}
#if SERVER
if(p==5){
 dashboard=widget(L"EDIT",L"",0x10000|0x800|0x4,25,154,738,85,507);refreshdash();
 btn(L"상태 새로 확인",25,254,150,SVCVIEW);btn(L"부팅 때 공유 시작",184,254,184,BOOTSHARE);
 btn(L"수동 시작으로 변경",378,254,176,MANUALSHARE);btn(L"트레이로 숨기기",564,254,182,TOTRAY);
 label(known.host[0]?L"외부 주소: 저장됨  ·  ‘1. 서버 주소’에서 확인할 수 있습니다.":L"외부 주소: 아직 등록하지 않았습니다. 집 안 공유부터 설정할 수 있습니다.",26,311,744,32);
 label(L"PC를 끄거나 절전 상태로 두면 접속할 수 없습니다. 트레이 아이콘을 누르면 이 창이 다시 열립니다.",26,347,737,50);
}
if(p==1){
 pickedVolume[0]=0;
 combo(5,L"드라이브 전체",155);loaddrives();btn(L"목록 새로 고침",447,152,153,VOLREFRESH);
 label(L"폴더만",25,207,164,24);btn(L"폴더 찾아보기…",192,197,177,PICK);
 line(0,L"폴더 경로 / 선택한 위치",247,0,245);
 line(1,L"공유 이름",287,0,60);
 btn(L"공유 추가",27,342,139,ADD);btn(L"공유 해제",177,342,139,REMOVE);btn(L"공유 목록 보기",329,342,161,LIST);
}
if(p==2){extern int choices(int,W*,int);W listbuf[4096];pickline(0,L"NAS 아이디",151);if(choices(1,listbuf,CNT(listbuf)))addlines(0,listbuf,0);line(1,L"새 계정 암호",189,1,500);btn(L"이 아이디 새로 만들기",192,225,190,USER);btn(L"기존 암호 바꾸기",393,225,168,PWRESET);pickline(2,L"권한을 줄 공유 이름",274);if(choices(0,listbuf,CNT(listbuf)))addlines(2,listbuf,0);combo(3,L"이 아이디의 권한",312);item(3,L"접근 안 됨");item(3,L"읽기");item(3,L"읽기·쓰기·삭제");btn(L"권한 저장",191,354,150,RIGHT);btn(L"이 아이디 권한",349,354,160,BYUSER);btn(L"전체 권한 보기",518,354,155,LIST);}
if(p==3){line(0,L"허용할 사설 대역",160,0,22);label(L"예: 집 안 또는 공유기 VPN의 실제 대역. 인터넷 전체는 허용할 수 없습니다.",27,204,740,28);btn(L"방화벽 규칙 추가",27,251,167,FWR);btn(L"이 앱 규칙 보기",206,251,162,FWLIST);btn(L"이 앱 규칙 제거",380,251,162,FWDEL);btn(L"공유·권한 보기",27,301,167,LIST);}
#else
if(p==1){line(0,L"공유기 VPN 아이디",156,0,128);line(1,L"공유기 VPN 암호",200,1,127);line(2,L"IPsec 공유 키",244,1,127);btn(L"이 PC에 VPN 등록",192,300,186,VPN);btn(L"이 앱의 VPN 해제",390,300,174,DELVPN);}
if(p==2){line(0,L"NAS 아이디",148,0,500);line(1,L"NAS 암호",184,1,500);pickline(2,L"열고 싶은 공유 이름",221);combo(3,L"이 PC에서 쓸 문자",257);for(int i=0;i<26;i++){W d[]={(W)('A'+i),':',0};item(3,d);}chk(4,L"지금은 집 안의 신뢰하는 개인망에서 직접 연결",294);btn(L"공유 목록 보기",25,341,170,DISC);btn(L"드라이브 연결·저장",207,341,187,MAP);btn(L"저장한 연결 해제",407,341,173,UNMAP);}
if(p==3){combo(0,L"시험할 드라이브",156);for(int i=0;i<26;i++){W d[]={(W)('A'+i),':',0},v[465];if(getmap(d,v,CNT(v)))item(0,d);}btn(L"256 MiB 시험",193,201,173,TEST);btn(L"저장한 드라이브 연결",25,260,196,RETRY);btn(L"로그인 때 자동 연결",235,260,187,AUTOON);btn(L"자동 연결 끄기",435,260,153,AUTOOFF);label(L"시험은 새 임시 파일만 만들며, 읽기 전용 폴더에서는 실행되지 않습니다.",27,320,732,40);}
#endif
if(p==4){label(L"1   집의 공유기:    외부 연결을 위한 암호화된 통로를 엽니다.\r\n\r\n2   3080 PC:           실제 파일과 폴더별 권한을 관리합니다.\r\n\r\n3   접속 기기:         Windows 탐색기 또는 Apple 파일 앱에서 엽니다.",27,159,729,143);btn(L"그림 설명서 열기 (PDF)",27,324,241,HELP);}
/* The OS decides whether this executable may run. Self-signature is informational;
   native privilege checks, credentials, route validation and explicit change consent
   still apply to every operation. Nothing here alters Windows application control. */
label(verified?L"결과 / 다음 단계":L"결과 / 다음 단계   ·   미서명 시험판",26,412,720,24);
T next=hint[p];
#if SERVER
if(p==5)next=L"공유 서비스가 실행 중이어도 폴더와 권한을 등록해야 합니다. ‘2. 공유할 위치’에서 시작하세요.";
#endif
status=widget(L"EDIT",message[0]?message:next,0x10000|0x200000|0x804|0x40,23,440,742,142,501);
#if !SERVER
if(p==3){W last[3800];if(getstr(HKCU,CLIKEY,L"LastAuto",last,CNT(last))){W label[3930]=L"최근 로그인 자동 연결 결과:\r\n";cat(label,CNT(label),last);speak(label);}}
#endif
}
static T confirm(int op){switch(op){
/* ADD has a location-specific review in approveTarget(). */
case BOOTSHARE:return L"이 PC의 Windows 파일 공유 서비스(LanmanServer)를 부팅 때 자동으로 시작하도록 하고, 중지되어 있으면 지금 시작합니다. 이 PC의 기존 Windows 공유에도 적용됩니다. 방화벽과 공유기는 바꾸지 않습니다. 적용할까요?";
case MANUALSHARE:return L"이 PC 전체의 Windows 파일 공유 서비스를 수동 시작으로 바꿉니다. 다음 부팅 때 기존 공유도 사용할 수 없을 수 있습니다. 지금 연결된 공유는 중지하지 않습니다. 적용할까요?";
case REMOVE:return L"이 공유와 전용 권한 그룹을 해제합니다. 저장된 실제 파일은 지우지 않습니다. 열려 있는 파일을 먼저 닫으세요. 계속할까요?";
case USER:return L"이 서버 PC에 파일 공유 전용 로컬 계정을 만듭니다. Windows 화면 로그인과 원격 데스크톱은 허용하지 않습니다. 만들까요?";
case PWRESET:return L"이 NAS 아이디의 암호를 바꾸고 해당 아이디의 기존 파일 연결을 종료합니다. 열려 있는 파일을 저장한 뒤 변경할까요?";
case RIGHT:return L"입력한 아이디의 해당 폴더 권한을 바꾸고, 새 권한 적용을 위해 이 서버에서 그 아이디의 기존 파일 연결을 닫습니다. 저장을 마친 뒤 진행하세요. 적용할까요?";
case FWR:return L"이 서버의 개인 네트워크에서, 입력한 사설 대역에만 TCP 445를 허용합니다. 공유기는 변경하지 않습니다. 추가할까요?";
case FWDEL:return L"차즘 NAS 5.0이 만든 개인망 방화벽 규칙을 모두 제거합니다. 계속할까요?";
case VPN:return L"Windows에 이 사용자 전용 L2TP/IPsec VPN과 NAS 한 대로 가는 경로를 만듭니다. 암호와 공유 키는 Windows에 보관합니다. 등록할까요?";
case DELVPN:return L"이 앱의 VPN 연결을 종료하고 개인 VPN 프로필을 제거합니다. 열려 있는 파일을 닫은 뒤 진행하세요. 해제할까요?";
case MAP:return L"선택한 문자에 폴더를 연결하고 NAS 로그인을 Windows 자격 증명 관리자에 보관합니다. 연결할까요?";
case AUTOON:return L"다음 Windows 로그인 때 이 위치의 Chazeum-NAS_USER.exe로 VPN과 드라이브를 연결합니다. 파일을 이 위치에 두세요. 등록할까요?";
case AUTOOFF:return L"차즘 NAS의 로그인 자동 연결을 해제할까요?";
case UNMAP:return L"선택한 드라이브 연결과 이 앱이 보관한 해당 암호를 지울까요?";
case TEST:return L"이 폴더에 새 256 MiB 파일을 쓰고 다시 읽어 전체 내용을 확인한 뒤 지웁니다. 다운로드·업로드 합계 약 512 MiB가 전송됩니다. 시험할까요?";
default:return 0;}}
static U32 dowork(H unused){(void)unused;W *out=message;int cap=CNT(message);int okay=0;out[0]=0;
#if SERVER
switch(job.op){case BOOTSHARE:case MANUALSHARE:okay=server_service(job.op==BOOTSHARE,out,cap);break;case SAVE:if(validend(&job.e)&&sz(job.e.host)<=128&&islocal(job.e.ip)&&saveendpoint(HKLM,SRVKEY,&job.e)){known=job.e;okay=cp(out,cap,L"서버 주소를 저장했습니다. ‘접속 정보 복사’로 유저용에 전달할 수 있습니다.");}else cp(out,cap,L"올바른 공유기 도메인(또는 공인 IPv4), 이 PC에 실제 할당된 사설 IPv4를 확인하세요.");break;
case BYUSER:{extern int byuser(T,W*,int);okay=byuser(job.v[0],out,cap);break;}case IPLIST5:okay=listip(out,cap);break;case LIST:okay=inventory(out,cap);break;case ADD:case REMOVE:okay=shareaction(job.v[1],job.v[0],job.op==REMOVE,job.volume,out,cap);break;case USER:okay=nativeuser(job.v[0],job.v[1],out,cap);break;case PWRESET:{extern int changepw(T,W*,W*,int);okay=changepw(job.v[0],job.v[1],out,cap);break;}case RIGHT:okay=rights(job.v[2],job.v[0],job.choice,out,cap);break;case FWR:okay=firewall(known.ip,job.v[0],out,cap);break;
case FWLIST:okay=cmdlet(L"Get-NetFirewallRule -Group 'ChaZeum NAS 5' -ErrorAction SilentlyContinue | Get-NetFirewallAddressFilter | ForEach-Object { $_.RemoteAddress }",out,cap);if(okay&&!out[0])cp(out,cap,L"이 앱이 추가한 규칙이 없습니다.");break;case FWDEL:okay=cmdlet(L"Get-NetFirewallRule -Group 'ChaZeum NAS 5' -ErrorAction SilentlyContinue | Where-Object { $_.Name -like 'ChaZeumNAS5-*' } | Remove-NetFirewallRule -ErrorAction Stop",out,cap);if(okay)cp(out,cap,L"5.0 방화벽 규칙을 제거했습니다.");break;}
#else
switch(job.op){case SAVE:{W vpn[48],b[465];int used=getprofile(vpn,CNT(vpn));for(int i=0;i<26&&!used;i++){W d[]={(W)('A'+i),':',0};used=getmap(d,b,CNT(b));}if(used&&(!eq(known.host,job.e.host)||!eq(known.ip,job.e.ip)||!eq(known.machine,job.e.machine)))cp(out,cap,L"다른 서버로 바꾸려면 기존 드라이브와 이 앱의 VPN을 먼저 해제하세요.");else if(validend(&job.e)&&sz(job.e.host)<=128&&saveendpoint(HKCU,CLIKEY,&job.e)){known=job.e;okay=cp(out,cap,L"서버 주소를 저장했습니다. 집 밖에서 쓸 때는 ‘외부 VPN’을 등록하세요.");}else cp(out,cap,L"공유기 도메인·서버 PC의 사설 IPv4·PC 이름을 확인하세요.");break;}
case VPN:okay=vpninstall(&known,job.v[0],job.v[1],job.v[2],out,cap);break;case DELVPN:okay=forgetvpn(out,cap);break;
case MAP:case DISC:if(connectnas(&known,job.home,out,cap)){if(job.op==MAP)okay=mapdrive(&known,job.v[2],job.v[0],job.v[3],job.v[1],1,out,cap);else okay=scanonly(&known,job.v[0],job.v[1],out,cap);}break;
case UNMAP:okay=detach(job.v[3],out,cap);break;case RETRY:okay=restore(out,cap);break;case AUTOON:case AUTOOFF:okay=setstartup(job.op==AUTOON,out,cap);break;case TEST:okay=driveprobe(&known,job.v[0],out,cap);break;}
#endif
if(!out[0])cp(out,cap,okay?L"완료했습니다.":L"작업을 마치지 못했습니다. 서버 주소, 권한 및 연결을 확인하세요.");erase(&job,sizeof(job));PostMessageW(win,0x8001,0,0);return 0;}
static void act(int id){if(busy)return;if(id>=TABS&&id<TABS+(SERVER?6:5)){page(id-TABS);return;}if(id==HELP){W detail[400];makeguide(detail,CNT(detail));speak(detail);return;}
#if SERVER
if(id==SVCVIEW){refreshdash();speak(L"이 PC의 파일 공유 서비스와 부팅 설정을 다시 읽었습니다.");return;}
if(id==TOTRAY){if(!tray_hide(win))speak(L"알림 영역에 아이콘을 만들지 못했습니다. 관리 창을 그대로 표시합니다.");return;}
if(id==VOLREFRESH){if(pickedVolume[0])cleartarget();loaddrives();speak(driveCount?L"드라이브 목록을 새로 확인했습니다. 사용할 드라이브를 선택하세요.":L"공유할 로컬 고정 NTFS 데이터 드라이브가 없습니다. 폴더만 공유할 수도 있습니다.");return;}
if(id==PICK){W out[260],show[260];Browse b={win,0,show,L"공유할 폴더 하나를 선택하세요. 드라이브 전체는 앞 화면의 목록에서 고릅니다.",0x241,0,0,0};H pid=SHBrowseForFolderW(&b);if(pid){
 SendMessageW(field[5],0x14e,(uintptr_t)-1,0);cleartarget();
 if(SHGetPathFromIDListW(pid,out)&&*out&&!rootpath(out)&&sz(out)<=245){pre(0,out);speak(L"이 폴더만 선택했습니다. 공유 이름을 입력하고 ‘공유 추가’를 누르세요.");}
 else speak(L"폴더 하나를 선택하세요. 드라이브 전체를 공유하려면 위 목록에서 고르세요.");CoTaskMemFree(pid);}return;}
if(id==ADD&&!approveTarget())return;
if((id==USER||id==PWRESET)&&field[1]&&!GetWindowTextLengthW(field[1])){
 if(!ask(L"암호가 비어 있습니다. Windows 기본 설정에서는 빈 암호로 다른 기기에서 접속할 수 없습니다. 그래도 빈 암호를 사용하시겠습니까?"))return;
}
if(id==COPY){W code[320];if(!invite(code,CNT(code),known.host,known.ip,known.machine))speak(L"먼저 서버 주소를 저장하세요.");else speak(clip(code)?L"접속 정보를 복사했습니다. 유저용 첫 화면에 붙여넣으세요. 암호는 포함되지 않습니다.":L"클립보드 복사 실패");return;}
#else
if(id==PASTE){W s[320];Endpoint e;if(val(3,s,CNT(s))&&pair(s,e.host,e.ip,e.machine)){pre(0,e.host);pre(1,e.ip);pre(2,e.machine);speak(L"세 칸을 채웠습니다. 내용이 맞으면 ‘서버 주소 저장’을 누르세요.");}else speak(L"CZ5| 로 시작하는 서버의 접속 정보 전체를 넣으세요.");return;}
#endif
T warning=confirm(id);if(warning&&!ask(warning))return;memset(&job,0,sizeof(job));job.op=id;lastop=id;
#if SERVER
if(id==ADD)cp(job.volume,CNT(job.volume),pickedVolume);
#endif
for(int i=0;i<4;i++)if(field[i]&&!val(i,job.v[i],CNT(job.v[i]))){speak(L"입력값이 너무 깁니다.");erase(&job,sizeof(job));return;}
if(id==SAVE){if(!cp(job.e.host,CNT(job.e.host),job.v[0])||!cp(job.e.ip,CNT(job.e.ip),job.v[1])||!cp(job.e.machine,CNT(job.e.machine),job.v[2])){erase(&job,sizeof(job));speak(L"서버 주소 또는 PC 이름의 길이를 확인하세요.");return;}}
job.choice=field[3]?(int)SendMessageW(field[3],0x147,0,0):-1;job.home=checked(4);if(id==USER||id==PWRESET||id==VPN||id==MAP)pre(1,L"");if(id==VPN)pre(2,L"");busy=1;for(int i=0;i<nk;i++)if(kids[i]!=status)EnableWindow(kids[i],0);speak(L"확인 중입니다…");H thread=CreateThread(0,0,dowork,0,0,0);if(thread)CloseHandle(thread);else {erase(&job,sizeof(job));PostMessageW(win,0x8001,0,0);cp(message,CNT(message),L"작업을 시작하지 못했습니다.");}}
static N proc(H h,U32 m,uintptr_t w,N l){if(m==0x111){int i=(int)(w&65535);
#if SERVER
if(i==405&&(w>>16)==1&&np==1&&!busy&&field[5]&&(H)l==field[5]){pickwhole();return 0;}
#endif
if((w>>16)==0&&((i>=100&&i<100+(SERVER?6:5))||(i>=202&&i<=231)))act(i);return 0;}if(m==0x8001){busy=0;for(int i=0;i<nk;i++)EnableWindow(kids[i],1);EnableWindow(nav[np],0);
#if !SERVER
if(lastop==DISC&&np==2){W copy[12288];if(cp(copy,CNT(copy),message)){W keep[61];val(2,keep,CNT(keep));SendMessageW(field[2],0x14b,0,0);addlines(2,copy,1);pre(2,keep);}}
#endif
speak(message);
#if SERVER
refreshdash();
#endif
return 0;}
#if SERVER
tray_event(h,m,l);if(m==2)tray_done(h);
#endif
if(m==0x10&&busy){MessageBoxW(h,L"작업이 끝나면 닫을 수 있습니다.",PRODUCT,0x40);return 0;}if(m==2){PostQuitMessage(0);return 0;}return DefWindowProcW(h,m,w,l);}
void start(void){inst=GetModuleHandleW(0);CoInitializeEx(0,2);verified=trusted();
#if SERVER
tray_init();T title=TITLE_SERVER;loadendpoint(HKLM,SRVKEY,&known);
#else
T title=TITLE_CLIENT;loadendpoint(HKCU,CLIKEY,&known);int argc=0;W**av=CommandLineToArgvW(GetCommandLineW(),&argc);int autoRun=av&&argc==2&&eq(av[1],L"/reconnect");if(av)LocalFree(av);if(autoRun){H sync=CreateMutexW(0,1,L"Local\\ChaZeumNAS5Reconnect");if(!sync||GetLastError()==183)ExitProcess(0);W report[3800];Sleep(3000);int ok=restore(report,CNT(report));extern int transient5(void);for(int i=0;!ok&&i<2&&transient5();i++){Sleep(i?20000:7000);ok=restore(report,CNT(report));}putstr(HKCU,CLIKEY,L"LastAuto",report);ReleaseMutex(sync);CloseHandle(sync);ExitProcess(ok?0:1);}
#endif
font=CreateFontW(-16,0,0,0,400,0,0,0,1,0,0,5,0,L"Malgun Gothic");if(!font)font=GetStockObject(17);titlefont=CreateFontW(-27,0,0,0,650,0,0,0,1,0,0,5,0,L"Malgun Gothic");if(!titlefont)titlefont=font;Class c={0};c.length=sizeof(c);c.handler=proc;c.instance=inst;c.brush=(H)16;c.cursor=LoadCursorW(0,(T)(N)32512);c.icon=brandicon(inst,0);c.small=brandicon(inst,1);c.cls=SERVER?L"ChazeumNAS5Server":L"ChazeumNAS5Client";if(!RegisterClassExW(&c))ExitProcess(3);win=CreateWindowExW(0,c.cls,title,0x00ca0000,(int)0x80000000,(int)0x80000000,810,644,0,0,inst,0);if(!win)ExitProcess(4);page(SERVER?5:(validend(&known)?2:0));ShowWindow(win,5);UpdateWindow(win);
#if !SERVER
if(startup_legacy()&&MessageBoxW(win,L"이전 버전의 로그인 자동 연결이 켜져 있습니다. 앞으로는 지금 연 유저용 파일을 사용하도록 바꿀까요? 기존 드라이브와 계정은 유지됩니다.",TITLE_CLIENT,0x124)==6){
 W result[500];setstartup(1,result,CNT(result));cp(message,CNT(message),result);speak(message);}
#endif
Msg msg;while(GetMessageW(&msg,0,0,0)>0)if(!IsDialogMessageW(win,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}ExitProcess(0);}
