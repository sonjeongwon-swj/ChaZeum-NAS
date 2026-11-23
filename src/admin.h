#include "base.h"
typedef struct {W*name,*comment;} Group1;
typedef struct {W *name;U32 type;W *comment;U32 perm,max,use;W *path,*pass;U32 zero;H sd;} Share502;
typedef struct {H multiple;U32 op,form,type;W*name;} Trustee;
typedef struct {U32 rights,mode,inherit;Trustee who;} Access;
typedef struct {U32 attr;U32 times[6],volume,sizehi,sizelo,links,idxhi,idxlo;} FInfo;
typedef struct {U32 flags;} Flags;
extern U32 NetLocalGroupAdd(T,U32,U8*,U32*),NetLocalGroupDel(T,T),NetLocalGroupGetInfo(T,T,U32,U8**),NetLocalGroupAddMembers(T,T,U32,U8*,U32),NetLocalGroupDelMembers(T,T,U32,U8*,U32),NetLocalGroupGetMembers(T,T,U32,U8**,U32,U32*,U32*,uintptr_t*);
extern U32 NetUserGetInfo(T,T,U32,U8**),NetUserEnum(T,U32,U32,U8**,U32,U32*,U32*,U32*),NetShareAdd(T,U32,U8*,U32*),NetShareGetInfo(T,T,U32,U8**),NetShareDel(T,T,U32),NetShareSetInfo(T,T,U32,U8*,U32*);
extern I32 GetFileInformationByHandle(H,FInfo*),GetVolumeInformationW(T,W*,U32,U32*,U32*,U32*,W*,U32),EqualSid(H,H),ConvertSidToStringSidW(H,W**),ConvertStringSecurityDescriptorToSecurityDescriptorW(T,U32,H*,U32*);
extern U32 GetFinalPathNameByHandleW(H,W*,U32,U32),GetWindowsDirectoryW(W*,U32),GetDriveTypeW(T),GetSecurityInfo(H,U32,U32,H*,H*,H*,H*,H*),SetSecurityInfo(H,U32,U32,H,H,H,H),SetEntriesInAclW(U32,Access*,H,H*);
int cmdlet(T,W*,int);int listip(W*,int);int islocal(T);
typedef struct { W root[4],id[72],label[64]; } VolumePick;
int datavolumes(VolumePick*,int);
void watchshare(void (*fn)(unsigned));
int shareaction(T,T,int,T,W*,int);int rights(T,T,int,W*,int);int inventory(W*,int);int firewall(T,T,W*,int);int dropsessions(T,W*,int);
int subnet(T);int choices(int,W*,int);int byuser(T,W*,int);
