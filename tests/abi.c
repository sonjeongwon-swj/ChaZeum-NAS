#include "base.h"
#include "admin.h"
#include <stdio.h>
int main(){printf("RasParams %zu cb %zu ifIndex %zu enc %zu  RasCred %zu  ACL %zu Share502 %zu LSA Obj %zu\n",sizeof(RasParams),offsetof(RasParams,cb),offsetof(RasParams,ifIndex),offsetof(RasParams,enc),sizeof(RasCred),sizeof(Access),sizeof(Share502),sizeof(LsaObj));}
