/* Replay the real page builder against a recording Win32 adapter. No Windows,
   disk sharing, credentials, networking or screenshots are involved. */
#include "../src/ui.c"
#include <stdio.h>
#include <stdlib.h>
typedef struct {W kind[20],text[660];U32 flags;int x,y,w,h,id,title,ro,enabled,visible,sel,count;W items[32][100];N keys[32];} Control;
static Control records[65];static int count;
H CreateWindowExW(U32 ex,T cls,T title,U32 style,I32 x,I32 y,I32 w,I32 h,H parent,H id,H instance,H param){
 (void)ex;(void)parent;(void)instance;(void)param;if(count>=64)exit(2);
 Control*c=&records[count++];cp(c->kind,20,cls);cp(c->text,660,title);c->flags=style;c->x=x;c->y=y;c->w=w;c->h=h;c->id=(int)(N)id;c->enabled=c->visible=1;c->sel=-1;return c;
}
H CreateThread(H a,size_t b,U32(*c)(H),H d,U32 e,U32*f){(void)a;(void)b;(void)e;(void)f;c(d);return (H)2;}I32 CloseHandle(H h){(void)h;return 1;}I32 PostMessageW(H h,U32 m,uintptr_t w,N l){(void)h;(void)w;(void)l;if(m==0x8004)donehome();return 1;}I32 ShowWindow(H h,I32 v){if(h)((Control*)h)->visible=!!v;return 1;}
int _snwprintf(W*b,size_t n,T fmt,...){(void)fmt;return cp(b,(int)n,L"차즘 공유 권한   ·   공유 2개   /   아이디 2개");}
I32 DestroyWindow(H v){(void)v;return 1;}I32 EnableWindow(H v,I32 u){if(v)((Control*)v)->enabled=u;return 1;}
N SendMessageW(H v,U32 msg,uintptr_t a,N b){Control*c=v;if(!c)return 0;
 switch(msg){
 case 0x30:c->title=(H)a==titlefont;break;
 case 0xcf:c->ro=!!a;break;
 case 0x143:if(c->count>=32)return -1;cp(c->items[c->count],100,(T)b);return c->count++;
 case 0x14b:c->count=0;c->sel=-1;c->text[0]=0;break;
 case 0x14e:c->sel=(int)a;if(c->sel>=0&&c->sel<c->count)cp(c->text,660,c->items[c->sel]);else c->text[0]=0;break;
 case 0x147:return c->sel;
 case 0x151:if(a<32)c->keys[a]=b;break;case 0x150:return a<32?c->keys[a]:-1;
 }return 0;
}
I32 SetWindowTextW(H v,T t){if(v)cp(((Control*)v)->text,660,t);return 1;}
I32 GetComputerNameW(W*b,U32*n){return cp(b,*n,L"NAS-PC");}
#if SERVER
int server_snapshot(W*b,int n){return cp(b,n,L"파일 공유 서비스: 실행 중   |   부팅 때 시작: 자동\r\n이 앱으로 등록한 공유: 1개 (예시)\r\n외부 접속: 별도 시험 필요");}
void readboard(Board*b,int r,int c){(void)r;(void)c;memset(b,0,sizeof(*b));b->okU=b->okS=b->okLive=b->nlive=1;b->nr=b->nc=b->totalR=b->totalC=2;cp(b->summary,640,L"파일 공유 서비스: 실행 중   |   부팅 때 시작: 자동\r\n이 앱으로 등록한 공유: 2개\r\n외부 접속: 별도 시험 필요");cp(b->live,420,L"demo");cp(b->user[0],21,L"demo");cp(b->user[1],21,L"guest01");cp(b->share[0],61,L"영상자료");cp(b->share[1],61,L"강의원고");cp(b->path[0],261,L"D:\\");cp(b->path[1],261,L"H:\\강의원고");b->mode[0][0]=2;b->mode[0][1]=b->mode[1][0]=1;}
int datavolumes(VolumePick*v,int cap){if(cap<1)return 0;cp(v[0].root,4,L"D:\\");cp(v[0].label,64,L"자료 디스크");cp(v[0].id,72,L"sample-volume");return 1;}
int choices(int people,W*b,int n){return cp(b,n,people?L"demo\n":L"자료창고\n");}
#else
int mapstatus(const Endpoint*e,W*b,int n){(void)e;return cp(b,n,L"X:   |   영상자료   |   탐색기에 연결됨");}
int getmap(T a,W*b,int n){return eq(a,L"X:")&&cp(b,n,L"sample");}
int getstr(H a,T b,T c,W*d,int n){(void)a;(void)b;(void)c;(void)d;(void)n;return 0;}
#endif
static void string(T t){putchar('"');while(*t)printf("\\u%04x",(unsigned)*t++);putchar('"');}
int main(int argc,char**argv){
 if(argc!=2)return 2;int p=atoi(argv[1]);if(p<0||p>5)return 2;font=(H)1;titlefont=(H)2;
 if(!SERVER&&p==5){cp(known.host,254,L"@WG:123456789abcdef0123456789abcdef0");cp(known.ip,16,L"192.168.1.30");cp(known.machine,16,L"NAS-PC");}
 page(p);
#if SERVER
 if(p==1){SendMessageW(field[5],0x14e,0,0);pickwhole();pre(1,L"자료창고");}
 if(p==2){SendMessageW(field[0],0x14e,0,0);SendMessageW(field[2],0x14e,0,0);SendMessageW(field[3],0x14e,1,0);}
#else
 if(p==2||p==5){pre(0,L"demo");pre(2,p==5?L"영상자료":L"자료창고");SendMessageW(field[3],0x14e,23,0);}
#endif
 printf("[");for(int i=0,j=0;i<count;i++){Control*c=&records[i];if(!c->visible)continue;if(j++)putchar(',');printf("{\"kind\":");string(c->kind);printf(",\"text\":");string(c->text);printf(",\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"id\":%d,\"flags\":%u,\"title\":%d,\"ro\":%d,\"enabled\":%d}",c->x,c->y,c->w,c->h,c->id,c->flags,c->title,c->ro,c->enabled);}puts("]");return 0;
}
