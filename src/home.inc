enum { PREVR=240,NEXTR,PREVC,NEXTC,HIDESTATE,GETCARD,SETCARD,ADDPATH,ADDID,CELL=610 };
static void page(int);
extern uintptr_t SetTimer(H,uintptr_t,U32,H);extern I32 KillTimer(H,uintptr_t);
static int reading;static H mini;
static H box(T s,int x,int y,int w,int h,int id){return widget(L"EDIT",s,0x804|0x40|0x200000,x,y,w,h,id);}
#if SERVER
#include "board.h"
static Board disk,shown;static int row,col,reqR,reqC;static H online,numbers,headers[3],rows[4],tiles[4][3],paging[4];static W card[80];
static U32 fetchhome(H a){(void)a;readboard(&disk,reqR,reqC);PostMessageW(win,0x8004,0,0);return 0;}
static void easy(void){
 H t=label(L"내 NAS 현황",25,63,734,39);SendMessageW(t,0x30,(uintptr_t)titlefont,1);mini=box(L"Windows에서 공유 상태를 읽고 있습니다…",25,105,738,70,507);online=widget(L"STATIC",L"현재 접속한 NAS 아이디: 확인 중",0,26,180,735,24,508);
 btn(L"+ 드라이브·폴더",25,209,179,ADDPATH);btn(L"+ 아이디·권한",214,209,169,ADDID);btn(L"연결 정보 복사",395,209,177,GETCARD);btn(L"새로 고침",582,209,178,SVCVIEW);
 numbers=widget(L"STATIC",L"차즘 공유 권한",0,26,249,739,24,509);label(L"공유 이름 / 원래 위치",26,278,222,25);
 for(int c=0;c<3;c++)headers[c]=widget(L"STATIC",L"",0x80,265+165*c,278,158,27,550+c);
 for(int r=0;r<4;r++){rows[r]=widget(L"STATIC",L"",0x80,26,304+r*38,232,37,560+r);for(int c=0;c<3;c++){tiles[r][c]=widget(L"BUTTON",L"",0x10000,263+c*165,304+r*38,159,33,CELL+3*r+c);ShowWindow(tiles[r][c],0);}}
 T labels[]={L"이전 공유",L"다음 공유",L"이전 아이디",L"다음 아이디"};for(int i=0;i<4;i++){paging[i]=btn(labels[i],25+i*185,459,176,PREVR+i);EnableWindow(paging[i],0);}btn(L"부팅 때 공유 시작",25,502,219,BOOTSHARE);btn(L"수동 시작으로 변경",256,502,235,MANUALSHARE);btn(L"트레이로 숨기기",505,502,254,TOTRAY);
 status=box(message[0]?message:L"드라이브나 폴더를 추가한 뒤 표에서 권한을 고르세요. 파일과 파티션은 그대로 둡니다.",25,546,737,45,501);
}
static void applyhome(void){shown=disk;Board*b=&shown;W v[650];SetWindowTextW(mini,b->summary);cp(v,650,L"현재 접속한 NAS 아이디: ");cat(v,650,!b->okLive?L"조회할 수 없음":b->nlive?b->live:L"없음");SetWindowTextW(online,v);
 if(b->okU&&b->okS)_snwprintf(v,650,L"차즘 공유 권한   ·   공유 %d개   /   아이디 %d개",b->totalR,b->totalC);else cp(v,650,L"차즘 공유 권한   ·   목록 일부를 확인하지 못했습니다.");SetWindowTextW(numbers,v);
 for(int c=0;c<3;c++)SetWindowTextW(headers[c],c<b->nc?b->user[c]:L"");
 for(int r=0;r<4;r++){v[0]=0;if(r<b->nr){cp(v,650,b->share[r]);cat(v,650,L"\r\n");cat(v,650,b->path[r]);}else if(!r&&b->okS&&!b->totalR)cp(v,650,L"아직 공유가 없습니다.");SetWindowTextW(rows[r],v);for(int c=0;c<3;c++){int show=r<b->nr&&c<b->nc,m=b->mode[r][c];SetWindowTextW(tiles[r][c],m==0?L"접근 안 됨":m==1?L"읽기":m==2?L"읽기·쓰기":L"확인 필요");ShowWindow(tiles[r][c],show?5:0);}}
 int e[]={row>0,row+4<b->totalR,col>0,col+3<b->totalC};for(int i=0;i<4;i++)EnableWindow(paging[i],e[i]);}
#else
extern int mapstatus(const Endpoint*,W*,int);
static Endpoint lookup;static W miniState[3800];static int hiddenMini,waiting;static H showbutton;
static U32 fetchhome(H a){(void)a;mapstatus(&lookup,miniState,CNT(miniState));PostMessageW(win,0x8004,0,0);return 0;}
static void applyhome(void){SetWindowTextW(mini,miniState);}
static void easy(void){
 W hide[4]={0};hiddenMini=getstr(HKCU,CLIKEY,L"HideMini",hide,4)&&eq(hide,L"1");H h=label(L"내 NAS 드라이브",26,64,746,38);SendMessageW(h,0x30,(uintptr_t)titlefont,1);
 if(!validend(&known)){label(L"처음 한 번만, 서버에서 복사한 연결 정보를 붙여넣으세요.",26,104,737,27);field[6]=widget(L"EDIT",L"",0x10080,26,136,521,30,406);SendMessageW(field[6],0xc5,85,0);btn(L"이 서버 등록",557,131,205,SETCARD);}else{W v[94]=L"내 서버:  ";cat(v,94,known.machine);cat(v,94,L"   ·   ");cat(v,94,known.ip);label(v,26,113,730,31);}
 line(0,L"NAS 아이디",175,0,256);line(1,L"암호",210,1,256);combo(3,L"내 PC 드라이브 문자",246);for(int i=0;i<26;i++){W d[]={(W)('A'+i),':',0};item(3,d);}pickline(2,L"공유 이름 (자동 찾기)",285);
 if(validend(&known)&&!securepin(known.host))chk(4,L"지금은 집 안의 신뢰하는 개인망에서 연결",318);else label(L"공유가 한 개면 자동으로, 여러 개면 선택해서 연결합니다.",26,319,730,27);
 btn(L"드라이브 연결",25,357,174,MAP);btn(L"저장한 연결 해제",210,357,175,UNMAP);btn(L"로그인 때 자동 연결",396,357,188,AUTOON);btn(L"자동 연결 끄기",596,357,166,AUTOOFF);
 showbutton=btn(hiddenMini?L"현황 보이기":L"현황 숨기기",25,409,157,HIDESTATE);btn(L"저장한 연결 다시 시도",194,409,218,RETRY);btn(L"트레이로 숨기기",426,409,180,TOTRAY);
 mini=box(L"탐색기 연결을 확인하고 있습니다.",25,453,737,62,507);ShowWindow(mini,hiddenMini?0:5);status=box(message[0]?message:L"아이디와 암호를 입력하고 이 PC에서 쓸 문자를 정하세요.",25,528,737,62,501);
}
static int selection(T t,W*first){int count=0;for(T p=t;*p;){T e=p;while(*e&&*e!='\r'&&*e!='\n')e++;if(e-p>=4&&e-p<=63&&p[0]==' '&&p[1]==0x2022&&p[2]==' '){W n[61];int l=(int)(e-p)-3;memcpy(n,p+3,l*sizeof(W));n[l]=0;if(shareok(n)){if(!count)cp(first,61,n);count++;}}p=e;while(*p=='\r'||*p=='\n')p++;}return count;}
#endif
static void updatehome(void){if(np!=5||busy||reading)return;
#if SERVER
 reqR=row;reqC=col;
#else
 if(hiddenMini)return;lookup=known;
#endif
 reading=1;H t=CreateThread(0,0,fetchhome,0,0,0);if(t)CloseHandle(t);else{reading=0;SetWindowTextW(mini,L"상태 조회를 시작하지 못했습니다.");}}
static void donehome(void){reading=0;if(np!=5||busy)return;
#if SERVER
 if(row&&row>=disk.totalR)row=0;if(col&&col>=disk.totalC)col=0;if(reqR!=row||reqC!=col){updatehome();return;}
#else
 if(!samehost(lookup.host,known.host)||!eq(lookup.ip,known.ip)||!eq(lookup.machine,known.machine)){updatehome();return;}
#endif
 applyhome();}
