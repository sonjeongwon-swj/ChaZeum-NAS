/* Opt-in, current-user login reconnect. A stable name lets Explorer replace
   the EXE without a helper, service, download, or startup-copy refresh. */
#include "base.h"
extern U32 GetDriveTypeW(T);
extern I32 SHGetFolderPathW(H,I32,H,U32,W*);
static T runkey=L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", runname=L"ChaZeumNAS5";
static T basename5=L"Chazeum-NAS_USER.exe";
static int command(W*out,int cap,T path){return cp(out,cap,L"\"")&&cat(out,cap,path)&&cat(out,cap,L"\" /reconnect");}
static int localapp(T path){
 if(!path||sz(path)<4||path[1]!=':'||path[2]!='\\')return 0;
 W root[]={path[0],':','\\',0};if(GetDriveTypeW(root)!=3)return 0;T tail=path;
 for(T q=path;*q;q++){if(*q=='"')return 0;if(*q=='\\')tail=q+1;}
 return eq(tail,basename5);
}
static int legacy(T path){
 W home[512];if(SHGetFolderPathW(0,0x1c,0,0,home)||!cat(home,CNT(home),L"\\ChaZeumNAS5\\Client-"))return 0;
 int n=sz(home);if(sz(path)!=n+36)return 0;
 W prefix[512];if(n>=CNT(prefix))return 0;memcpy(prefix,path,n*sizeof(W));prefix[n]=0;
 if(!eq(prefix,home)||!eq(path+n+32,L".exe"))return 0;
 for(int i=0;i<32;i++){W c=path[n+i];if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')))return 0;}
 return 1;
}
/* 0 off, 1 stable location, 2 old 5.0.0.3 copy, -1 unknown/mismatched.
   Never change an unrelated Run value or delete any legacy or user file. */
static int entry(W*prev,int n){
 W actual[1100],expect[1100];if(!getstr(HKCU,runkey,runname,actual,CNT(actual)))return 0;
 if(!getstr(HKCU,CLIKEY,L"StartupCopy",prev,n)||!command(expect,CNT(expect),prev)||!eq(expect,actual))return -1;
 return legacy(prev)?2:localapp(prev)?1:-1;
}
int startup_legacy(void){W old[1024],self[1024];U32 n=GetModuleFileNameW(0,self,CNT(self));return n&&n<CNT(self)&&localapp(self)&&entry(old,CNT(old))==2;}
int setstartup(int enable,W*out,int cap){
 W old[1024],self[1024],line[1100];int state=entry(old,CNT(old));
 if(state<0){cp(out,cap,L"기존 자동 연결 항목이 이 앱 소유인지 확인할 수 없어 변경하지 않았습니다.");return 0;}
 if(!enable){
  if(state&&!delstr(HKCU,runkey,runname)){cp(out,cap,L"자동 연결 해제 실패. 기존 설정을 확인하세요.");return 0;}
  return cp(out,cap,L"로그인 때 자동 연결을 끕니다. 저장한 드라이브나 로그인 정보는 지우지 않습니다.");
 }
 U32 n=GetModuleFileNameW(0,self,CNT(self));
 if(!n||n>=CNT(self)||!localapp(self)||!command(line,CNT(line),self)||sz(line)>=260){
  cp(out,cap,L"로그인 때 사용할 Chazeum-NAS_USER.exe를 이 PC의 짧은 로컬 경로에 두고 다시 여세요. 기존 자동 연결은 그대로입니다.");return 0;}
 if(state==1&&eq(old,self))return cp(out,cap,L"자동 연결이 이미 이 파일을 사용합니다. 다음 버전도 이 파일만 교체하세요.");
 int had=getstr(HKCU,CLIKEY,L"StartupCopy",old,CNT(old));
 if(!putstr(HKCU,CLIKEY,L"StartupCopy",self)){cp(out,cap,L"자동 연결 경로를 저장하지 못했습니다.");return 0;}
 if(!putstr(HKCU,runkey,runname,line)){
  int back=had?putstr(HKCU,CLIKEY,L"StartupCopy",old):delstr(HKCU,CLIKEY,L"StartupCopy");
  cp(out,cap,back?L"자동 연결 등록에 실패했습니다. 기존 설정을 유지했습니다.":L"자동 연결 경로를 복원하지 못했습니다. 자동 연결 설정을 확인하세요.");return 0;}
 return cp(out,cap,L"다음 Windows 로그인부터 이 파일로 자동 연결합니다. 새 버전도 이 위치에 덮어쓰세요.");
}
