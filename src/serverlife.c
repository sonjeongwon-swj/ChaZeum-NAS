#include "serverlife.h"
typedef struct { U32 type,state,accepted,exit,specific,check,hint; } SvcState;
typedef struct { U32 type,start,error; W *bin,*group; U32 tag; W *deps,*user,*name; } SvcConfig;
_Static_assert(sizeof(SvcState)==28 && sizeof(SvcConfig)==64,"Windows service ABI");
extern H OpenSCManagerW(T,T,U32),OpenServiceW(H,T,U32);
extern I32 CloseServiceHandle(H),QueryServiceStatus(H,SvcState*),QueryServiceConfigW(H,SvcConfig*,U32,U32*);
extern I32 ChangeServiceConfigW(H,U32,U32,U32,T,T,U32*,T,T,T,T),StartServiceW(H,U32,const W**);
enum { AUTO=2, MANUAL=3, DISABLED=4, STOPPED=1, STARTING=2, RUNNING=4 };
static H openserver(U32 rights,U32 *err){
 H scm=OpenSCManagerW(0,0,1); if(!scm){*err=GetLastError();return 0;}
 H h=OpenServiceW(scm,L"LanmanServer",rights); *err=h?0:GetLastError();CloseServiceHandle(scm);return h;
}
static U32 config(H h,U32 *start){
 union { SvcConfig cfg; U8 space[8192]; } b; U32 needed=0;
 if(!QueryServiceConfigW(h,&b.cfg,sizeof(b),&needed))return GetLastError();
 *start=b.cfg.start;return 0;
}
static T runtext(U32 state){
 switch(state){case RUNNING:return L"실행 중";case STOPPED:return L"중지됨";case STARTING:return L"시작 중";
 case 3:return L"중지 중";case 5:return L"재개 중";case 6:return L"일시 중지 중";case 7:return L"일시 중지됨";default:return L"확인 필요";}
}
int server_snapshot(W*out,int cap){
 U32 er=0,mode=0,n=0;SvcState s={0};H h=openserver(1|4,&er);
 if(!h)return errnum(out,cap,L"Windows 파일 공유 서비스 상태를 읽을 수 없습니다",er);
 er=config(h,&mode);if(!er&&!QueryServiceStatus(h,&s))er=GetLastError();CloseServiceHandle(h);
 if(er)return errnum(out,cap,L"Windows 파일 공유 서비스 조회 실패",er);
 cp(out,cap,L"파일 공유 서비스: ");cat(out,cap,runtext(s.state));cat(out,cap,L"    |    부팅 때 시작: ");
 cat(out,cap,mode==AUTO?L"자동":mode==MANUAL?L"수동":mode==DISABLED?L"사용 안 함":L"확인 필요");
 cat(out,cap,L"\r\n이 앱으로 등록한 공유: ");
 if(s.state!=RUNNING)cat(out,cap,L"서비스 실행 후 확인 가능");
 else if(!count_owned_shares(&n))cat(out,cap,L"목록 조회 실패");
 else{W digits[24];_snwprintf(digits,CNT(digits),L"%u개",n);cat(out,cap,digits);
      if(!n)cat(out,cap,L"  —  ‘2. 공유할 위치’에서 첫 폴더를 추가하세요.");}
 cat(out,cap,L"\r\n외부 접속: 별도 시험 필요");return 1;
}
static int settings(H h,U32 mode){return ChangeServiceConfigW(h,0xffffffffu,mode,0xffffffffu,0,0,0,0,0,0,0);}
int server_service(int automatic,W*out,int cap){
 U32 er=0,old=0,now=0;int changed=0,ok=0;SvcState s={0};
 H h=openserver(1|2|4|(automatic?16:0),&er);
 if(!h)return errnum(out,cap,L"파일 공유 서비스 설정을 열 수 없습니다",er);
 er=config(h,&old);if(er)goto bad;
 if(old!=AUTO&&old!=MANUAL){cp(out,cap,L"Windows에서 사용 안 함 또는 정책으로 설정된 서비스입니다. 변경하지 않았습니다.");goto end;}
 if(!QueryServiceStatus(h,&s)){er=GetLastError();goto bad;}
 if(automatic&&s.state!=RUNNING&&s.state!=STARTING&&s.state!=STOPPED){
  cp(out,cap,L"서비스가 중지 또는 전환 중입니다. 상태를 새로 확인한 뒤 다시 시도하세요. 설정은 바꾸지 않았습니다.");goto end;}
 U32 want=automatic?AUTO:MANUAL;
 if(old!=want){if(!settings(h,want)){er=GetLastError();goto bad;}changed=1;}
 if(automatic&&s.state==STOPPED&&!StartServiceW(h,0,0)){
  er=GetLastError();if(er!=1056){
   errnum(out,cap,L"서비스를 시작할 수 없습니다",er);
   if(changed){if(!config(h,&now)&&now==want&&settings(h,old))cat(out,cap,L" 이전 시작 유형을 복원했습니다.");
               else cat(out,cap,L" 시작 유형을 복원했는지 상태를 확인하세요.");}
   goto end;
  }
 }
 er=config(h,&now);if(er)goto bad;
 if(now!=want){cp(out,cap,L"요청한 시작 유형이 적용되지 않았습니다. Windows 정책을 확인하세요.");goto end;}
 if(!automatic){ok=cp(out,cap,L"시작 유형: 수동. 현재 서비스나 연결은 중지하지 않았습니다.");goto end;}
 if(!QueryServiceStatus(h,&s)){er=GetLastError();goto bad;}
 for(int i=0;i<24&&s.state==STARTING;i++){Sleep(250);if(!QueryServiceStatus(h,&s)){er=GetLastError();goto bad;}}
 if(s.state==RUNNING)ok=cp(out,cap,L"부팅 때 자동으로 시작합니다. 파일 공유 서비스가 실행 중입니다.");
 else{cp(out,cap,L"부팅 때 시작: 자동. 현재 서비스 상태: ");cat(out,cap,runtext(s.state));cat(out,cap,L". 상태 새로 확인을 눌러 주세요.");}
 goto end;
bad:errnum(out,cap,L"Windows 파일 공유 서비스 설정 확인 실패",er);
end:CloseServiceHandle(h);return ok;
}
