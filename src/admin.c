#include "admin.h"
#define WHY(s) (cp(out,cap,L##s),0)
static const W head[]=L"ChaZeumNAS5:";
static int validtag(T s){if(!s||sz(s)!=44)return 0;for(int i=0;i<12;i++)if(s[i]!=head[i])return 0;for(int i=12;i<44;i++)if(!((s[i]>='0'&&s[i]<='9')||(s[i]>='a'&&s[i]<='f')))return 0;return 1;}
int count_owned_shares(U32 *out){
 U32 resumed=0,total=0; *out=0;
 do{U32 n=0,all=0,was=resumed;Share1 *p=0;U32 rc=NetShareEnum(0,1,(U8**)&p,0xffffffffu,&n,&all,&resumed);
 if(rc && rc!=234){if(p)NetApiBufferFree(p);return 0;}
 for(U32 i=0;i<n;i++)if(validtag(p[i].remark) && (p[i].type&255)==0)total++;
 if(p)NetApiBufferFree(p);if(!rc){*out=total;return 1;}if(was==resumed)return 0;
 }while(1);
}
static int names(T tag,W*r,W*w){if(!validtag(tag))return 0;_snwprintf(r,30,L"CZ5R_%.12ls",tag+12);_snwprintf(w,30,L"CZ5W_%.12ls",tag+12);return 1;}
static int namedSID(T id,U8 sid[128]){W pc[16],full[96],dom[256];U32 n=CNT(pc),ns=128,nd=CNT(dom),ty=0;if(!GetComputerNameW(pc,&n))return 0;_snwprintf(full,CNT(full),L"%ls\\%ls",pc,id);return LookupAccountNameW(0,full,sid,&ns,dom,&nd,&ty);}
static int managedUser(T user){User1*x=0;if(!idok(user)||NetUserGetInfo(0,user,1,(U8**)&x))return 0;int ok=eq(x->comment,MARK)&&!(x->flags&2);NetApiBufferFree(x);return ok;}
static int account_error(W*out,int cap,U32 e,T fallback){
 if(e==2245||e==1325||e==615||e==86||e==2203)return errnum(out,cap,L"이 PC의 Windows 암호 정책에서 거절했습니다. 다른 암호를 입력하세요.",e);
 if(e==2224||e==1379)return errnum(out,cap,L"이미 사용 중인 Windows 이름입니다. 다른 NAS 아이디를 정하세요.",e);
 if(e==2202||e==123)return errnum(out,cap,L"Windows가 이 계정 이름을 사용할 수 없습니다. 다른 이름을 입력하세요.",e);
 return errnum(out,cap,fallback,e);
}
int nativeuser(T id,W*pass,W*out,int cap){if(!sz(id))return WHY("만들 NAS 아이디를 입력하세요.");if(sz(id)>20)return WHY("Windows 로컬 계정 이름은 20자까지 사용할 수 있습니다.");if(!idok(id))return WHY("Windows NAS 계정에 쓸 수 없는 이름입니다. 기본 계정 이름이나 사용할 수 없는 기호를 확인하세요.");if(!pass)return WHY("암호 입력칸을 확인하세요.");User1 u={0};u.name=(W*)id;u.password=pass;u.priv=1;u.comment=(W*)MARK;u.flags=0x201|2;U32 arg=0,e=NetUserAdd(0,1,(U8*)&u,&arg);if(e)return account_error(out,cap,e,L"Windows에서 NAS 계정을 만들지 못했습니다. 기존 계정은 변경하지 않았습니다.");
U8 sid[128];H policy=0;LsaObj a={0};a.len=sizeof(a);T denied[]={L"SeDenyInteractiveLogonRight",L"SeDenyRemoteInteractiveLogonRight",L"SeDenyBatchLogonRight",L"SeDenyServiceLogonRight"};LsaStr l[4];for(int i=0;i<4;i++){l[i].len=2*sz(denied[i]);l[i].max=l[i].len+2;l[i].data=(W*)denied[i];}e=namedSID(id,sid)?LsaNtStatusToWinError(LsaOpenPolicy(0,&a,0x810,&policy)):1332;if(!e){e=LsaNtStatusToWinError(LsaAddAccountRights(policy,sid,l,4));LsaClose(policy);}if(!e){U32 fl=0x201;e=NetUserSetInfo(0,id,1008,(U8*)&fl,&arg);}if(e){U32 rollback=NetUserDel(0,id);if(rollback)return errnum(out,cap,L"계정 설정 실패. 비활성 계정 정리가 필요합니다.",rollback);return errnum(out,cap,L"계정 설정을 되돌렸습니다.",e);}return cp(out,cap,L"NAS 아이디를 만들었습니다. ‘권한’에서 폴더를 허용하기 전까지는 접근할 수 없습니다.");}
static int groupOwn(T name,T tag){Group1*g=0;if(NetLocalGroupGetInfo(0,name,1,(U8**)&g))return 0;int ok=eq(g->comment,tag);NetApiBufferFree(g);return ok;}
static U32 gmake(T name,T tag){Group1 g={(W*)name,(W*)tag};return NetLocalGroupAdd(0,1,(U8*)&g,0);}
/* Only local fixed data volumes. Enumeration reads metadata; it creates no shares. */
extern U32 QueryDosDeviceW(T,W*,U32);
extern I32 GetVolumeNameForVolumeMountPointW(T,W*,U32);
static int candidate(T root,VolumePick *out) {
 W fs[32],label[261],device[512],dos[3],win[260],osVol[72],selected[72];
 if(!rootpath(root)||GetDriveTypeW(root)!=3)return 0;
 dos[0]=root[0];dos[1]=':';dos[2]=0;
 if(!QueryDosDeviceW(dos,device,CNT(device)))return 0;
 T prefix=L"\\Device\\HarddiskVolume";int k=0;
 while(prefix[k]){if(device[k]!=prefix[k])return 0;k++;}
 if(!device[k])return 0;
 for(;device[k];k++)if(device[k]<'0'||device[k]>'9')return 0;
 U32 n=GetWindowsDirectoryW(win,CNT(win));
 if(!n||n>=CNT(win)||win[1]!=':'||win[2]!='\\')return 0;
 W systemRoot[]={win[0],':','\\',0};
 if(!GetVolumeNameForVolumeMountPointW(systemRoot,osVol,CNT(osVol))||
    !GetVolumeNameForVolumeMountPointW(root,selected,CNT(selected))||eq(osVol,selected)||
    !GetVolumeInformationW(root,label,CNT(label),0,0,0,fs,CNT(fs))||!eq(fs,L"NTFS"))return 0;
 if(!cp(out->root,CNT(out->root),root)||!cp(out->id,CNT(out->id),selected))return 0;
 if(!label[0]||!cp(out->label,CNT(out->label),label))cp(out->label,CNT(out->label),L"로컬 디스크");
 /* A disk label is display text, not a path or an instruction. */
 for(W*q=out->label;*q;q++)if(*q<32||*q==127||(*q>=0x202a&&*q<=0x202e)||(*q>=0x2066&&*q<=0x2069))*q=' ';
 return 1;
}
int datavolumes(VolumePick*rows,int cap) {
 U32 mask=GetLogicalDrives();int count=0;
 for(int i=0;i<26&&count<cap;i++){if(!(mask&(1u<<i)))continue;
  W root[]={(W)('A'+i),':','\\',0};if(candidate(root,&rows[count]))count++;
 }return count;
}
static int samevolume(T expected,H handle){
 W result[100],bound[100];U32 n=GetFinalPathNameByHandleW(handle,result,CNT(result),1); /* VOLUME_NAME_GUID */
 if(!n||n>=CNT(result)||!expected||!cp(bound,CNT(bound),expected))return 0;
 int m=sz(bound);if(m&&bound[m-1]=='\\')bound[--m]=0;
 if(n&&result[n-1]=='\\')result[n-1]=0;
 return m>0&&eq(result,bound);
}
static H folder(T path){int n=sz(path);if(n<3||n>245||path[1]!=':'||path[2]!='\\'||(n>3&&path[n-1]=='\\'))return 0;W clean[260],os[260],fs[30];if(!cp(clean,CNT(clean),path))return 0;W root[]={path[0],':','\\',0};if(GetDriveTypeW(root)!=3||!GetVolumeInformationW(root,0,0,0,0,0,fs,CNT(fs))||!eq(fs,L"NTFS"))return 0;
if(!GetWindowsDirectoryW(os,CNT(os)))return 0;W device[512],aVol[70],oVol[70],oRoot[]={os[0],':','\\',0},dos[]={root[0],':',0};extern U32 QueryDosDeviceW(T,W*,U32);extern I32 GetVolumeNameForVolumeMountPointW(T,W*,U32);if(!QueryDosDeviceW(dos,device,CNT(device))||device[0]!='\\'||device[1]!='D'||!GetVolumeNameForVolumeMountPointW(root,aVol,CNT(aVol))||!GetVolumeNameForVolumeMountPointW(oRoot,oVol,CNT(oVol)))return 0;if(eq(aVol,oVol)){if(n==3)return 0;T blocked[]={L"Windows",L"Program Files",L"Program Files (x86)",L"ProgramData",L"Recovery",L"Users"};for(int i=0;i<CNT(blocked);i++){int m=sz(blocked[i]);if(n>=3+m){W save=clean[3+m];clean[3+m]=0;int same=eq(clean+3,blocked[i]);clean[3+m]=save;if(same&&(save==0||save=='\\')){if(i!=5||save==0)return 0;int more=0;for(int j=4+m;j<n;j++)if(clean[j]=='\\')more=1;if(!more)return 0;}}}}
for(int i=3;i<=n;i++){W v=clean[i];if(v==0||v=='\\'){W ch=clean[i-1];if(ch=='.'||ch==' '||(i>3&&ch=='\\'))return 0;clean[i]=0;U32 a=GetFileAttributesW(clean);clean[i]=v;if(a==0xffffffff||!(a&16)||(a&0x400))return 0;}else if(v<32||v==':'||v=='/'||v=='*'||v=='?'||v=='"'||v=='<'||v=='>'||v=='|')return 0;}
T denied[]={L"System Volume Information",L"$Recycle.Bin"};for(int j=0;j<CNT(denied);j++){int m=sz(denied[j]);if(n>=m+3){W end=clean[m+3];clean[m+3]=0;int bad=eq(clean+3,denied[j]);clean[m+3]=end;if(bad&&(end==0||end=='\\'))return 0;}}
H h=CreateFileW(path,0x00060080,3,0,3,0x02200000,0);if(h==BAD)return 0;W final[300],want[300];cp(want,CNT(want),L"\\\\?\\");cat(want,CNT(want),path);FInfo f={0};U32 out=GetFinalPathNameByHandleW(h,final,CNT(final),0);if(!out||out>=CNT(final)||(!eq(want,final)&&!(rootpath(path)&&sz(final)+1==sz(want)&&cat(final,CNT(final),L"\\")&&eq(want,final)))||!GetFileInformationByHandle(h,&f)||!(f.attr&16)||(f.attr&0x400)){CloseHandle(h);return 0;}return h;}
static U32 fsrights(H h,U8*r,U8*w,int remove){H old=0,sd=0,acl=0;U32 e=GetSecurityInfo(h,1,4,0,0,&old,0,&sd);if(e)return e;if(!old){LocalFree(sd);return 1336;}Access a[2]={0};for(int i=0;i<2;i++){a[i].rights=i?0x1301bf:0x1200a9;a[i].mode=remove?4:1;a[i].inherit=3;a[i].who.form=0;a[i].who.type=2;a[i].who.name=(W*)(i?w:r);}e=SetEntriesInAclW(2,a,old,&acl);if(!e)e=SetSecurityInfo(h,1,4,0,0,acl,0);if(acl)LocalFree(acl);LocalFree(sd);return e;}
extern I32 CompareStringOrdinal(T,I32,T,I32,I32);
static int sameOrParent(T a,T b){
 int n=sz(a),m=sz(b);if(!n||!m)return 0;
 while(n>1&&a[n-1]=='\\')n--;while(m>1&&b[m-1]=='\\')m--;
 if(n>m||CompareStringOrdinal(a,n,b,n,1)!=2)return 0;
 return n==m||a[n-1]=='\\'||b[n]=='\\';
}
static int pathOverlap(T a,T b){
 if(!sz(a)||!sz(b))return 0;
 if(sameOrParent(a,b)||sameOrParent(b,a))return 1;
 if(sz(a)<3||sz(b)<3||a[1]!=':'||b[1]!=':'||a[2]!='\\'||b[2]!='\\')return 0;
 W r1[]={a[0],':','\\',0},r2[]={b[0],':','\\',0},g1[72],g2[72];
 if(eq(r1,r2)||!(sameOrParent(a+2,b+2)||sameOrParent(b+2,a+2)))return 0;
 /* Different letters can mount the same physical volume. If either cannot be
    checked and the relative paths overlap, no new share is created. */
 if(!GetVolumeNameForVolumeMountPointW(r1,g1,CNT(g1))||!GetVolumeNameForVolumeMountPointW(r2,g2,CNT(g2)))return -1;
 return eq(g1,g2);
}
/* Enumeration at level 2 returns paths without reading each share's security descriptor. */
typedef struct {W*name;U32 type;W*comment;U32 permission,max,use;W*path,*password;} Share2;
_Static_assert(sizeof(Share2)==56&&offsetof(Share2,path)==40,"SHARE_INFO_2 ABI");
static void showshare(W*out,int cap,T name,T path){cat(out,cap,L"\r\n  ");cat(out,cap,name?name:L"(이름 미상)");cat(out,cap,L"  →  ");cat(out,cap,path?path:L"(경로 미상)");}
static int available(T path,W*out,int cap){
 U32 resume=0;int seen=0;out[0]=0;
 do{Share2*p=0;U32 n=0,t=0,old=resume;
  U32 rc=NetShareEnum(0,2,(U8**)&p,0xffffffff,&n,&t,&resume);
  if(rc&&rc!=234){if(p)NetApiBufferFree(p);return errnum(out,cap,L"Windows 공유 목록을 읽지 못했습니다. 기존 공유와 겹치는지는 확인되지 않았습니다. 컴퓨터 관리 → 공유 폴더 → 공유에서 확인하세요.",rc);}
  for(U32 i=0;i<n;i++){
   if((p[i].type&0x80000000u)||(p[i].type&255)||!p[i].path)continue;
   int relation=pathOverlap(path,p[i].path);if(!relation)continue;
   if(!seen)cp(out,cap,L"이번 공유는 추가하지 않았습니다. 아래 기존 공유를 확인하세요.");seen++;
   if(seen<=8){cat(out,cap,relation<0?L"\r\n두 경로가 같은 디스크인지 확인할 수 없음:":validtag(p[i].comment)?L"\r\n차즘 NAS에 이미 등록된 위치:":L"\r\n같거나 상위·하위에 있는 기존 Windows 공유:");showshare(out,cap,p[i].name,p[i].path);}
  }
  if(p)NetApiBufferFree(p);
  if(!rc)break;if(old==resume)return errnum(out,cap,L"Windows 공유 목록을 끝까지 읽지 못했습니다. 변경하지 않았습니다.",234);
 }while(1);
 if(seen){if(seen>8)cat(out,cap,L"\r\n나머지는 ‘공유 목록 보기’에서 확인하세요.");
 cat(out,cap,L"\r\n같은 위치가 이미 차즘에 있으면 그 공유 이름으로 ‘3. 아이디·권한’을 여세요. 다른 Windows 공유는 변경하지 않았습니다.");return 0;}return 1;
}
/* Distinguish a wrong path belonging to our transaction from an unrelated
   share that acquired the same name. Never delete the latter during rollback. */
static int readback(T name,T tag,T path){
 Share502 *actual=0;U32 e=NetShareGetInfo(0,name,502,(U8**)&actual);
 if(e||!actual){if(actual)NetApiBufferFree(actual);return 0;}
 int ours=eq(actual->comment,tag)&&eq(actual->name,name)&&actual->type==0;
 int exact=ours&&(!path||eq(actual->path,path));NetApiBufferFree(actual);
 return exact;
}
int shareaction(T name,T path,int remove,T expected,W*out,int cap){if(!shareok(name))return WHY("공유 이름을 입력하세요. 한글·영문·숫자·공백·밑줄·붙임표를 쓸 수 있습니다.");if(!remove){
 if(rootpath(path)){VolumePick check={0};
  if(!expected||!*expected||!candidate(path,&check)||!eq(check.id,expected))return WHY("선택한 데이터 드라이브를 다시 확인하세요. 드라이브 목록을 새로 고친 뒤 다시 선택해 주세요.");}
 else if(expected&&*expected)return WHY("드라이브와 선택한 위치가 다릅니다. 위치를 다시 선택하세요.");
 }
 W tag[64],r[30],w[30];Share502*old=0;U32 rc=NetShareGetInfo(0,name,502,(U8**)&old);W real[260];if(remove){if(rc||!old)return WHY("이름과 일치하는 공유를 찾지 못했습니다.");int owned=validtag(old->comment)&&cp(tag,CNT(tag),old->comment)&&cp(real,CNT(real),old->path);NetApiBufferFree(old);if(!owned)return WHY("5.0이 만든 공유만 해제할 수 있습니다.");path=real;}else{if(!rc){cp(out,cap,old&&validtag(old->comment)&&old->path&&eq(old->path,path)?L"이 위치는 같은 이름으로 이미 등록되어 있습니다. ‘3. 아이디·권한’으로 가세요.":L"이 이름을 기존 Windows 공유가 사용 중입니다. 기존 설정은 바꾸지 않았습니다.");if(old){showshare(out,cap,name,old->path);NetApiBufferFree(old);}return 0;}if(rc!=2310)return errnum(out,cap,L"공유 상태 확인 실패",rc);if(!available(path,out,cap))return 0;cp(tag,CNT(tag),head);if(!guidtext(tag+12,40))return 0;}
if(!names(tag,r,w))return WHY("공유 식별자를 만들 수 없습니다.");H h=folder(path);if(!h&&!remove)return WHY("로컬 NTFS 데이터 드라이브나 일반 폴더를 선택하세요. 시스템 영역·연결 지점은 제외됩니다.");if(!remove&&rootpath(path)&&!samevolume(expected,h)){CloseHandle(h);return WHY("선택한 드라이브와 현재 열린 디스크가 다릅니다. 공유하지 않았습니다.");}
U8 rs[128],ws[128];int gr=0,gw=0,fs=0,added=0,invalid=0;H sd=0;W *rsid=0,*wsid=0;
if(remove){if(!groupOwn(r,tag)||!groupOwn(w,tag)||!namedSID(r,rs)||!namedSID(w,ws)){if(h)CloseHandle(h);return WHY("권한 그룹 소유권을 확인하지 못했습니다.");}rc=NetShareDel(0,name,0);if(!rc){int dirty=!h||fsrights(h,rs,ws,1)!=0;U32 a=NetLocalGroupDel(0,r),b=NetLocalGroupDel(0,w);if(h)CloseHandle(h);if(dirty||a||b)return WHY("공유는 해제했습니다. 폴더 또는 그룹의 권한 정리는 확인이 필요합니다.");return cp(out,cap,L"공유를 해제했습니다. 폴더와 파일은 그대로 있습니다.");}if(h)CloseHandle(h);return errnum(out,cap,L"공유 해제 실패",rc);}
rc=gmake(r,tag);if(rc)goto done;gr=1;rc=gmake(w,tag);if(rc)goto done;gw=1;
if(!namedSID(r,rs)||!namedSID(w,ws)||!ConvertSidToStringSidW(rs,&rsid)||!ConvertSidToStringSidW(ws,&wsid)){rc=GetLastError();if(!rc)rc=1332;goto done;}
W sddl[550];_snwprintf(sddl,CNT(sddl),L"D:P(A;;FA;;;BA)(A;;0x1200a9;;;%ls)(A;;0x1301bf;;;%ls)",rsid,wsid);if(!ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl,1,&sd,0)){rc=GetLastError();goto done;}rc=fsrights(h,rs,ws,0);if(rc)goto done;fs=1;
Share502 sh={0};sh.name=(W*)name;sh.comment=tag;sh.path=(W*)path;sh.max=0xffffffff;sh.sd=sd;rc=NetShareAdd(0,502,(U8*)&sh,0);if(rc)goto done;added=1;
/* 1005 is SHARE_INFO_1005: enforce encryption and access-based enumeration. */
Flags fl={0x8000|0x0800|0x30};rc=NetShareSetInfo(0,name,1005,(U8*)&fl,0);if(!rc){Flags*live=0;rc=NetShareGetInfo(0,name,1005,(U8**)&live);if(!rc){if((live->flags&0x8800)!=0x8800)rc=5;NetApiBufferFree(live);}}
if(!rc&&!readback(name,tag,path))rc=13;
if(!rc&&rootpath(path)){VolumePick now={0};if(!candidate(path,&now)||!eq(expected,now.id))rc=15;}
done:if(rc||!added){if(added&&(!readback(name,tag,0)||NetShareDel(0,name,0)))invalid=1;if(fs&&fsrights(h,rs,ws,1))invalid=1;if(gw&&NetLocalGroupDel(0,w))invalid=1;if(gr&&NetLocalGroupDel(0,r))invalid=1;}if(sd)LocalFree(sd);if(rsid)LocalFree(rsid);if(wsid)LocalFree(wsid);CloseHandle(h);if(invalid)return WHY("설정 실패 후 정리가 모두 끝나지 않았습니다. 서버 목록과 Windows 권한을 확인하세요.");if(rc||!added)return errnum(out,cap,L"공유를 만들지 못했습니다. 이번 변경을 되돌렸습니다.",rc);cp(out,cap,rootpath(path)?L"드라이브 전체를 공유했습니다: ":L"폴더를 공유했습니다: ");cat(out,cap,path);cat(out,cap,L"\r\n공유 이름: ");cat(out,cap,name);cat(out,cap,L"\r\n다음: ‘3. 아이디·권한’에서 누가 읽거나 쓸 수 있는지 정하세요.");return 1;}
typedef struct {W*client,*user;U32 seconds,idle;} Session10;
extern U32 NetSessionEnum(T,T,T,U32,U8**,U32,U32*,U32*,U32*),NetSessionDel(T,T,T);
static int closeuser(T user){Session10*ss=0;U32 count=0,total=0,res=0,ret=NetSessionEnum(0,0,0,10,(U8**)&ss,0xffffffff,&count,&total,&res);if(ret)return 0;int all=1;W pc[16],full[64];U32 len=16;full[0]=0;if(GetComputerNameW(pc,&len))_snwprintf(full,CNT(full),L"%ls\\%ls",pc,user);for(U32 i=0;i<count;i++){if(!ss[i].user||(!eq(user,ss[i].user)&&!eq(full,ss[i].user)))continue;U32 rv=NetSessionDel(0,ss[i].client,ss[i].user);if(rv&&rv!=2312)all=0;}if(ss)NetApiBufferFree(ss);return all;}
int rights(T share,T user,int mode,W*out,int cap){if(!shareok(share)||!managedUser(user)||mode<0||mode>2)return WHY("등록한 공유 이름과 NAS 아이디, 권한을 선택하세요.");Share502*s=0;if(NetShareGetInfo(0,share,502,(U8**)&s))return WHY("공유를 찾지 못했습니다.");W r[30],w[30];int own=names(s->comment,r,w)&&groupOwn(r,s->comment)&&groupOwn(w,s->comment);NetApiBufferFree(s);U8 sid[128];if(!own||!namedSID(user,sid))return WHY("5.0이 관리하는 공유와 NAS 아이디만 변경할 수 있습니다.");H p=sid;T list[]={r,w};for(int i=0;i<2;i++)if(mode!=i+1){U32 e=NetLocalGroupDelMembers(0,list[i],0,(U8*)&p,1);if(e&&e!=1377&&e!=1387)return errnum(out,cap,L"기존 그룹 권한 제거 실패",e);}if(mode){U32 e=NetLocalGroupAddMembers(0,list[mode-1],0,(U8*)&p,1);if(e&&e!=1378){closeuser(user);return errnum(out,cap,L"새 권한 적용 실패. 권한 목록을 확인하세요.",e);}}if(!closeuser(user))return WHY("권한을 변경했습니다. 이전 접속을 종료하지 못해 해당 아이디의 연결을 끊고 다시 접속해야 합니다.");return cp(out,cap,L"권한을 저장했습니다. 이 아이디로 다시 연결하면 새 권한이 적용됩니다.");}
static void members(W*out,int cap,T group,T prefix){U8*buf=0;U32 n=0,t=0;uintptr_t h=0;cat(out,cap,prefix);U32 e=NetLocalGroupGetMembers(0,group,3,&buf,0xffffffff,&n,&t,&h);if(e)cat(out,cap,L"(조회 실패)");else if(!n)cat(out,cap,L"없음");else{W**v=(W**)buf;for(U32 i=0;i<n;i++){if(i)cat(out,cap,L", ");cat(out,cap,v[i]);}}if(buf)NetApiBufferFree(buf);cat(out,cap,L"\r\n");}
int inventory(W*out,int cap){cp(out,cap,L"공유 중인 위치와 권한\r\n");Share502*s=0;U32 n=0,t=0,u=0;U32 e=NetShareEnum(0,502,(U8**)&s,0xffffffff,&n,&t,&u);if(e)return errnum(out,cap,L"공유 목록 조회 실패",e);int seen=0;for(U32 i=0;i<n;i++){W r[30],w[30];if(!names(s[i].comment,r,w))continue;seen++;cat(out,cap,rootpath(s[i].path)?L"\r\n[드라이브 전체] ":L"\r\n[폴더] ");cat(out,cap,s[i].name);cat(out,cap,L"  →  ");cat(out,cap,s[i].path);cat(out,cap,L"\r\n");members(out,cap,r,L"   읽기: ");members(out,cap,w,L"   읽기·쓰기: ");}if(!seen)cat(out,cap,L"아직 이 앱으로 등록한 공유가 없습니다.\r\n");
int other=0;for(U32 i=0;i<n;i++)if(!(s[i].type&0x80000000u)&&!(s[i].type&255)&&!validtag(s[i].comment)&&s[i].path){
 if(!other++)cat(out,cap,L"\r\n다른 Windows 공유 (보기 전용):");showshare(out,cap,s[i].name,s[i].path);}
if(other)cat(out,cap,L"\r\n");if(s)NetApiBufferFree(s);cat(out,cap,L"\r\nNAS 아이디: ");User1*usr=0;n=t=u=0;e=NetUserEnum(0,1,2,(U8**)&usr,0xffffffff,&n,&t,&u);if(e)return errnum(out,cap,L"계정 목록 조회 실패",e);seen=0;for(U32 i=0;i<n;i++)if(eq(usr[i].comment,MARK)){if(seen++)cat(out,cap,L", ");cat(out,cap,usr[i].name);}if(!seen)cat(out,cap,L"없음");if(usr)NetApiBufferFree(usr);return 1;}
int subnet(T s){W ip[16];int slash=0;while(s&&s[slash]&&s[slash]!='/')slash++;if(slash<7||slash>15||s[slash]!='/')return 0;for(int i=0;i<slash;i++)ip[i]=s[i];ip[slash]=0;if(!ipprivate(ip))return 0;T p=s+slash+1;int prefix=0;if(sz(p)<1||sz(p)>2||p[0]=='0')return 0;while(*p){if(*p<'0'||*p>'9')return 0;prefix=prefix*10+*p++-'0';}if(prefix<16||prefix>32)return 0;U32 address=0,oct=0;for(T v=ip;;v++){if(*v=='.'||!*v){address=(address<<8)|oct;oct=0;if(!*v)break;}else oct=oct*10+*v-'0';}U32 mask=prefix==32?0:((1u<<(32-prefix))-1);return (address&mask)==0;}
int firewall(T local,T remote,W*out,int cap){if(!islocal(local)||!subnet(remote))return WHY("서버에 실제로 할당된 집 안 IP와 허용할 사설 대역을 입력하세요. 예: 192.168.0.0/24");W sample[2000],log[400],guid[33];if(!guidtext(guid,CNT(guid)))return WHY("방화벽 규칙 이름을 만들 수 없습니다.");_snwprintf(sample,CNT(sample),L"New-NetFirewallRule -Name 'ChaZeumNAS5-%ls' -DisplayName 'ChaZeum NAS 5 - SMB private' -Group 'ChaZeum NAS 5' -Direction Inbound -Action Allow -Protocol TCP -LocalPort 445 -LocalAddress '%ls' -RemoteAddress '%ls' -Profile Private -EdgeTraversalPolicy Block | Out-Null",guid,local,remote);if(!cmdlet(sample,log,CNT(log)))return WHY("Windows 방화벽 규칙을 추가하지 못했습니다. 관리자 권한과 네트워크 프로필을 확인하세요.");return cp(out,cap,L"개인 네트워크에 한해 입력한 대역의 SMB 접근을 허용했습니다. 공유기의 인터넷 포트는 변경하지 않았습니다.");}
int choices(int people,W*out,int cap){U32 n=0,t=0,u=0;out[0]=0;if(people){User1*v=0;U32 e=NetUserEnum(0,1,2,(U8**)&v,0xffffffff,&n,&t,&u);if(e)return 0;for(U32 i=0;i<n;i++)if(eq(v[i].comment,MARK)&&idok(v[i].name)){cat(out,cap,v[i].name);cat(out,cap,L"\n");}if(v)NetApiBufferFree(v);}else{Share1*v=0;U32 e=NetShareEnum(0,1,(U8**)&v,0xffffffff,&n,&t,&u);if(e)return 0;for(U32 i=0;i<n;i++)if(validtag(v[i].remark)&&shareok(v[i].name)){cat(out,cap,v[i].name);cat(out,cap,L"\n");}if(v)NetApiBufferFree(v);}return 1;}
static int hasMember(T group,H sid){U8 *data=0;U32 n=0,t=0;uintptr_t off=0;U32 rc=NetLocalGroupGetMembers(0,group,0,&data,0xffffffff,&n,&t,&off);if(rc)return -1;int yes=0;H*p=(H*)data;for(U32 i=0;i<n;i++)if(EqualSid(sid,p[i]))yes=1;if(data)NetApiBufferFree(data);return yes;}
int byuser(T user,W*out,int cap){U8 sid[128];if(!managedUser(user)||!namedSID(user,sid))return WHY("목록에서 등록한 NAS 아이디를 선택하세요.");W list[4096];if(!choices(0,list,CNT(list)))return WHY("공유 목록 조회 실패");cp(out,cap,user);cat(out,cap,L" 아이디의 폴더별 권한\r\n");for(W*p=list;*p;){W*q=p;while(*q&&*q!='\n')q++;int end=!*q;*q=0;Share1*s=0;W r[30],w[30];if(!NetShareGetInfo(0,p,1,(U8**)&s)){if(names(s->remark,r,w)){int a=hasMember(r,sid),b=hasMember(w,sid);cat(out,cap,p);cat(out,cap,b<0||a<0?L" : 조회 못함\r\n":b?L" : 읽기·쓰기\r\n":a?L" : 읽기\r\n":L" : 접근 안 됨\r\n");}NetApiBufferFree(s);}if(end)break;p=q+1;}return 1;}
int changepw(T id,W*pass,W*out,int cap){if(!managedUser(id))return WHY("이 앱에서 만든 NAS 아이디를 선택하세요.");if(!pass)return WHY("암호 입력칸을 확인하세요.");U32 e=NetUserSetInfo(0,id,1003,(U8*)&pass,0);if(e)return account_error(out,cap,e,L"Windows에서 NAS 암호를 바꾸지 못했습니다.");if(!closeuser(id))return WHY("암호를 바꿨습니다. 기존 접속은 직접 끊고 새 암호로 다시 연결하세요.");return cp(out,cap,L"NAS 암호를 바꿨습니다. 해당 기기에서 새 암호로 드라이브를 다시 연결하세요.");}
