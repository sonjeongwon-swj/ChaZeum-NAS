#include "base.h"
int cmdlet(T,W*,int);int route5(const Endpoint*,W*,int);int guideCopy(W*,int);int vpnsavecred(T,T,W*,W*,W*,int);
int getprofile(W*,int);int scanonly(const Endpoint*,T,T,W*,int);int guidprofile(T);int smbcred(T,T,T,W*,int);int getmap(T,W*,int);
extern I32 LogonUserW(T,T,T,U32,U32,H*),ImpersonateLoggedOnUser(H),RevertToSelf(void);
extern U32 WNetGetUserW(T,W*,U32*);
