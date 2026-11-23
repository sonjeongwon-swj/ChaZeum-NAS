/* Fault injection for the production share transaction. No Windows changes. */
#include "admin.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
static int checks,groups,groupAdds,groupDels,aclGrants,aclRevokes,shareAdds,shareDels,opened,closed;
static int failGroup,failAcl,failShare,failFlags,omitEncrypt,failDelete,overlap,exists,owned=1,reparse,ntfs=1;
static T wanted=L"D:\\Data", existingPath=L"D:\\Data";static int aliasedLetter,liveShare,wrongPath,wrongOwner,missingABE,letterSwap,handleSwap,shortRoot,diskKind=3,dosAlias,systemAlias;static U32 driveMask;
static T volumeD=L"\\\\?\\Volume{00000000-0000-0000-0000-000000000001}\\",volumeE=L"\\\\?\\Volume{00000000-0000-0000-0000-000000000002}\\",volumeOS=L"\\\\?\\Volume{00000000-0000-0000-0000-000000000003}\\";
I32 CompareStringOrdinal(T a,I32 x,T b,I32 y,I32 ignore){(void)ignore;if(x!=y)return 1;for(int i=0;i<x;i++){W left=a[i],right=b[i];if(left>='a'&&left<='z')left-=32;if(right>='a'&&right<='z')right-=32;if(left!=right)return 1;}return 2;}
U32 GetLogicalDrives(void){return driveMask;}
static U32 recordedError,liveFlags;static W openedPath[260],out[2048];
static W appTag[]=L"ChaZeumNAS5:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
#define CHECK(x) do { checks++; if(!(x)){fprintf(stderr,"share regression line %d: %s\n",__LINE__,#x);exit(1);} } while(0)
int _snwprintf(W*b,size_t cap,T fmt,...){va_list ap;va_start(ap,fmt);size_t n=0;for(;*fmt&&n+1<cap;fmt++){if(*fmt!='%'){b[n++]=*fmt;continue;}fmt++;int prec=0,limit=0;while(*fmt=='.'||(*fmt>='0'&&*fmt<='9')){if(*fmt=='.')prec=1;else limit=limit*10+*fmt-'0';fmt++;}if(*fmt=='l'&&fmt[1]=='s'){fmt++;T v=va_arg(ap,T);int count=0;while(*v&&n+1<cap&&(!prec||count++<limit))b[n++]=*v++;}}b[n]=0;va_end(ap);return n;}
int errnum(W*b,int cap,T why,U32 e){recordedError=e;cp(b,cap,why);return 0;}
U32 GetLastError(void){return 0;}int guidtext(W*b,int cap){return cp(b,cap,L"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");}
I32 GetComputerNameW(W*b,U32*n){return cp(b,*n,L"NASBOX");}
I32 LookupAccountNameW(T a,T name,H sid,U32*n,W*domain,U32*dn,U32*type){(void)a;(void)name;(void)n;(void)domain;(void)dn;(void)type;memset(sid,1,12);return 1;}
U32 GetDriveTypeW(T a){return a[0]=='Z'?4:diskKind;}U32 GetWindowsDirectoryW(W*b,U32 n){cp(b,n,L"C:\\Windows");return 10;}
I32 GetVolumeInformationW(T a,W*b,U32 c,U32*d,U32*e,U32*f,W*fs,U32 n){if(b&&c)cp(b,c,a[0]=='D'?L"수업자료":L"");(void)d;(void)e;(void)f;return cp(fs,n,ntfs?L"NTFS":L"exFAT");}
U32 QueryDosDeviceW(T a,W*b,U32 n){(void)a;cp(b,n,dosAlias?L"\\??\\D:\\mount":L"\\Device\\HarddiskVolume9");return 25;}
I32 GetVolumeNameForVolumeMountPointW(T root,W*b,U32 n){return cp(b,n,root[0]=='C'||systemAlias?volumeOS:((root[0]=='E'&&!aliasedLetter)||letterSwap)?volumeE:volumeD);}
U32 GetFileAttributesW(T a){(void)a;return reparse?0x410:16;}
H CreateFileW(T path,U32 access,U32 sharing,H sa,U32 creation,U32 flags,H template){(void)sa;(void)template;CHECK(access==0x60080&&sharing==3&&creation==3&&flags==0x02200000);opened++;cp(openedPath,CNT(openedPath),path);return (H)10;}
U32 GetFinalPathNameByHandleW(H h,W*b,U32 n,U32 f){CHECK(h==(H)10&&f<=1);if(f){cp(b,n,handleSwap||openedPath[0]=='E'?volumeE:volumeD);}else {cp(b,n,L"\\\\?\\");cat(b,n,openedPath);}if(shortRoot&&rootpath(openedPath))b[sz(b)-1]=0;return sz(b);}
I32 GetFileInformationByHandle(H h,FInfo*f){CHECK(h==(H)10);f->attr=16;return 1;}
I32 CloseHandle(H h){CHECK(h==(H)10);closed++;return 1;}H LocalFree(H p){(void)p;return 0;}
U32 NetApiBufferFree(H p){(void)p;return 0;}
U32 NetShareGetInfo(T server,T name,U32 level,U8**p){(void)server;(void)name;static Share502 share;static Flags flags;
if(level==502){if(!exists&&!liveShare)return 2310;share.name=L"자료";share.type=0;share.comment=(owned&&(!liveShare||!wrongOwner))?appTag:L"somebody else";share.path=(W*)(liveShare?(wrongPath?L"F:\\Unexpected":wanted):existingPath);*p=(U8*)&share;return 0;}
CHECK(level==1005);flags.flags=omitEncrypt?0:missingABE?(liveFlags&~0x800):liveFlags;*p=(U8*)&flags;return 0;}
U32 NetShareEnum(T server,U32 level,U8**p,U32 size,U32*n,U32*total,U32*resume){(void)server;(void)size;(void)resume;static Share502 share;CHECK(level==502);share.type=0;share.path=(W*)existingPath;*p=overlap?(U8*)&share:0;*n=*total=overlap?1:0;return 0;}
U32 NetLocalGroupAdd(T server,U32 level,U8*p,U32*err){(void)server;(void)err;Group1*g=(Group1*)p;CHECK(level==1&&eq(g->comment,appTag));groupAdds++;if(failGroup==groupAdds)return 2223;groups++;return 0;}
U32 NetLocalGroupDel(T server,T group){(void)server;(void)group;groups--;groupDels++;return 0;}
U32 NetLocalGroupGetInfo(T server,T name,U32 level,U8**p){(void)server;(void)name;(void)level;static Group1 g;g.comment=appTag;*p=(U8*)&g;return 0;}
I32 ConvertSidToStringSidW(H sid,W**str){(void)sid;*str=L"S-1-5-21-100-1001";return 1;}
I32 ConvertStringSecurityDescriptorToSecurityDescriptorW(T s,U32 revision,H*out,U32*size){(void)s;(void)size;CHECK(revision==1);*out=(H)20;return 1;}
U32 GetSecurityInfo(H h,U32 type,U32 info,H*a,H*b,H*acl,H*c,H*sd){(void)a;(void)b;(void)c;CHECK(h==(H)10&&type==1&&info==4);*acl=(H)30;*sd=(H)31;return 0;}
U32 SetEntriesInAclW(U32 n,Access*a,H previous,H*out){CHECK(n==2&&previous==(H)30&&a[0].inherit==3&&a[1].inherit==3&&a[0].rights==0x1200a9&&a[1].rights==0x1301bf);if(a[0].mode==4){CHECK(a[1].mode==4);aclRevokes++;}else{CHECK(a[0].mode==1&&a[1].mode==1);aclGrants++;if(failAcl)return 5;}*out=(H)32;return 0;}
U32 SetSecurityInfo(H h,U32 type,U32 info,H a,H b,H acl,H c){(void)a;(void)b;(void)c;CHECK(h==(H)10&&type==1&&info==4&&acl==(H)32);return 0;}
U32 NetShareAdd(T server,U32 level,U8*p,U32*err){(void)server;(void)err;Share502*s=(Share502*)p;CHECK(level==502&&eq(s->name,L"자료")&&eq(s->path,wanted)&&s->sd==(H)20&&eq(s->comment,appTag));shareAdds++;if(!failShare)liveShare=1;return failShare?5:0;}
U32 NetShareSetInfo(T server,T name,U32 level,U8*p,U32*err){(void)server;(void)name;(void)err;CHECK(level==1005);liveFlags=((Flags*)p)->flags;CHECK((liveFlags&0x8800)==0x8800);return failFlags?50:0;}
U32 NetShareDel(T server,T name,U32 zero){(void)server;(void)name;CHECK(!zero);shareDels++;if(!failDelete)liveShare=0;return failDelete?5:0;}
static void reset(void){groups=groupAdds=groupDels=aclGrants=aclRevokes=shareAdds=shareDels=opened=closed=0;failGroup=failAcl=failShare=failFlags=omitEncrypt=failDelete=overlap=exists=reparse=0;owned=ntfs=1;aliasedLetter=liveShare=wrongPath=wrongOwner=missingABE=0;letterSwap=handleSwap=shortRoot=dosAlias=systemAlias=0;diskKind=3;wanted=existingPath=L"D:\\Data";driveMask=(1u<<2)|(1u<<3)|(1u<<4)|(1u<<25);recordedError=liveFlags=0;}
static int create(void){return shareaction(L"자료",L"D:\\Data",0,0,out,CNT(out));}
int main(void){
reset();CHECK(create());CHECK(groups==2&&aclGrants==1&&!aclRevokes&&shareAdds==1&&!shareDels&&opened==closed);
reset();failGroup=1;CHECK(!create());CHECK(recordedError==2223&&!groups&&!groupDels&&!shareAdds&&opened==closed);
reset();failGroup=2;CHECK(!create());CHECK(recordedError==2223&&!groups&&groupDels==1&&!shareAdds&&opened==closed);
reset();failAcl=1;CHECK(!create());CHECK(!groups&&groupDels==2&&!shareAdds&&!aclRevokes&&opened==closed);
reset();failShare=1;CHECK(!create());CHECK(!groups&&aclRevokes==1&&!shareDels&&opened==closed);
reset();failFlags=1;CHECK(!create());CHECK(!groups&&shareDels==1&&aclRevokes==1&&opened==closed);
reset();omitEncrypt=1;CHECK(!create());CHECK(!groups&&shareDels==1&&aclRevokes==1&&opened==closed);
reset();failFlags=failDelete=1;CHECK(!create());CHECK(shareDels==1&&groupDels==2&&opened==closed);
reset();overlap=1;CHECK(!create());CHECK(!groupAdds&&!opened&&!shareAdds);
reset();exists=1;CHECK(!create());CHECK(!groupAdds&&!opened&&!shareAdds);
reset();ntfs=0;CHECK(!create());CHECK(!groupAdds&&!opened&&!shareAdds);
reset();reparse=1;CHECK(!create());CHECK(!groupAdds&&!opened&&!shareAdds);
reset();CHECK(!shareaction(L"자료",L"C:\\",0,0,out,CNT(out)));CHECK(!groupAdds&&!opened&&!shareAdds);
reset();CHECK(!shareaction(L"자료",L"C:\\Windows\\Data",0,0,out,CNT(out)));CHECK(!groupAdds&&!opened&&!shareAdds);
reset();CHECK(!shareaction(L"자료",L"D:\\Data\\",0,0,out,CNT(out)));CHECK(!groupAdds&&!opened&&!shareAdds);
reset();exists=1;groups=2;CHECK(shareaction(L"자료",L"ignored",1,0,out,CNT(out)));CHECK(!groups&&groupDels==2&&shareDels==1&&aclRevokes==1&&!aclGrants&&opened==closed);
reset();exists=1;owned=0;CHECK(!shareaction(L"자료",L"ignored",1,0,out,CNT(out)));CHECK(!shareDels&&!opened&&!groupDels);
reset();exists=1;groups=2;failDelete=1;CHECK(!shareaction(L"자료",L"ignored",1,0,out,CNT(out)));CHECK(groups==2&&!aclRevokes&&!groupDels&&opened==closed);
/* Whole-volume selection is distinct from a client-side letter or folder. */
VolumePick v[26]={0};reset();CHECK(datavolumes(v,26)==2);CHECK(eq(v[0].root,L"D:\\")&&eq(v[0].label,L"수업자료")&&eq(v[0].id,volumeD));CHECK(eq(v[1].root,L"E:\\")&&eq(v[1].id,volumeE));CHECK(!groupAdds&&!shareAdds&&!opened);
reset();wanted=L"D:\\";CHECK(shareaction(L"자료",wanted,0,volumeD,out,CNT(out)));CHECK(shareAdds==1&&aclGrants==1&&opened==closed);
reset();wanted=L"E:\\";shortRoot=1;CHECK(shareaction(L"자료",wanted,0,volumeE,out,CNT(out)));CHECK(shareAdds==1&&opened==closed);
reset();wanted=L"D:\\";CHECK(!shareaction(L"자료",wanted,0,0,out,CNT(out)));CHECK(!opened&&!groupAdds&&!shareAdds);
reset();wanted=L"D:\\";letterSwap=1;CHECK(!shareaction(L"자료",wanted,0,volumeD,out,CNT(out)));CHECK(!opened&&!groupAdds&&!shareAdds);
reset();wanted=L"D:\\";handleSwap=1;CHECK(!shareaction(L"자료",wanted,0,volumeD,out,CNT(out)));CHECK(opened==1&&closed==1&&!groupAdds&&!shareAdds);
reset();wanted=L"C:\\";CHECK(!shareaction(L"자료",wanted,0,volumeOS,out,CNT(out)));CHECK(!opened&&!groupAdds);
reset();systemAlias=1;CHECK(datavolumes(v,26)==0);CHECK(!shareaction(L"자료",L"D:\\",0,volumeOS,out,CNT(out)));CHECK(!shareAdds);
reset();diskKind=2;CHECK(!datavolumes(v,26));CHECK(!shareaction(L"자료",L"D:\\",0,volumeD,out,CNT(out)));CHECK(!groupAdds);
reset();ntfs=0;CHECK(!datavolumes(v,26));CHECK(!shareaction(L"자료",L"D:\\",0,volumeD,out,CNT(out)));CHECK(!groupAdds);
reset();dosAlias=1;CHECK(!datavolumes(v,26));CHECK(!shareaction(L"자료",L"D:\\",0,volumeD,out,CNT(out)));CHECK(!groupAdds);
reset();overlap=1;CHECK(!shareaction(L"자료",L"D:\\",0,volumeD,out,CNT(out)));CHECK(!groupAdds);
reset();overlap=1;existingPath=L"D:\\";CHECK(!create());CHECK(!groupAdds);
reset();wanted=L"D:\\";existingPath=L"E:\\Else";overlap=1;CHECK(shareaction(L"자료",wanted,0,volumeD,out,CNT(out)));CHECK(shareAdds==1);
reset();wanted=existingPath=L"D:\\";exists=1;groups=2;CHECK(shareaction(L"자료",L"",1,0,out,CNT(out)));CHECK(groups==0&&aclRevokes==1&&shareDels==1&&opened==closed);
reset();CHECK(!shareaction(L"자료",L"D:\\Data",0,volumeD,out,CNT(out)));CHECK(!groupAdds&&!opened);
reset();wrongPath=1;CHECK(!create());CHECK(!groups&&shareDels==1&&aclRevokes==1);
reset();wrongOwner=1;CHECK(!create());CHECK(!groups&&shareDels==0&&aclRevokes==1); /* keep unrelated share */
reset();missingABE=1;CHECK(!create());CHECK(!groups&&shareDels==1&&aclRevokes==1);
reset();aliasedLetter=1;overlap=1;existingPath=L"E:\\Else";CHECK(!shareaction(L"자료",L"D:\\",0,volumeD,out,CNT(out)));CHECK(!groupAdds&&!opened);
printf("%d drive selection, share creation, rollback, encrypted-share enforcement and folder protection checks passed (Windows APIs simulated).\n",checks);return 0;}
