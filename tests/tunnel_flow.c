#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(v) do{if(!(v)){fprintf(stderr,"routing case failed: %d\n",__LINE__);exit(1);}count++;}while(0)
int count,have=1,addr=1,up,forward,faildial,called,passreads,closecount;W name[]=L"CZ5-aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";Endpoint ep={L"home.example.net",L"192.168.1.22",L"NASBOX"};
#include "../src/tunnel.c"
int getstr(H h,T k,T n,W*out,int cap){(void)h;(void)k;(void)n;return have&&cp(out,cap,name);}H GetProcessHeap(void){return (H)1;}H HeapAlloc(H h,U32 f,size_t n){(void)h;(void)f;return calloc(1,n);}I32 HeapFree(H h,U32 f,H v){(void)h;(void)f;free(v);return 1;}
U32 RasGetEntryPropertiesW(T b,T p,Entry*e,U32*n,U8*x,U32*y){(void)b;(void)p;(void)n;(void)x;(void)y;e->kind=2;e->strategy=3;e->opts2=16;e->enc=1;e->opts=0x20000000;cp(e->phone,CNT(e->phone),addr?ep.host:L"other.example.net");return 0;}
U32 RasEnumConnectionsW(Connection*v,U32*n,U32*c){(void)n;*c=up;v->h=(H)7;cp(v->name,CNT(v->name),name);return 0;}
U32 RasGetCredentialsW(T b,T p,RasCred*c){(void)b;(void)p;passreads++;c->mask=3;cp(c->user,CNT(c->user),L"test");cp(c->pass,CNT(c->pass),L"os-opaque-handle");return 0;}
U32 RasDialW(H a,T b,RasParams*r,U32 d,H e,H*h){(void)a;(void)b;(void)e;(void)d;CHECK(r->size==2128&&eq(r->entry,name)&&eq(r->pass,L"os-opaque-handle"));called++;*h=(H)7;if(faildial)return 691;up=1;return 0;}
U32 RasHangUpW(H h){(void)h;closecount++;return 0;}void Sleep(U32 n){(void)n;}
int cmdlet(T c,W*out,int n){(void)c;return cp(out,n,forward?L"CZ5-ROUTE":L"no matching route");}int errnum(W*out,int n,T s,U32 e){(void)e;cp(out,n,s);return 0;}
int _snwprintf(W*b,size_t n,T fmt,...){return cp(b,n,fmt);}
int main(void){W out[450];CHECK(!guidprofile(L"CZ5-incomplete"));CHECK(!guidprofile(L"CZ5-aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa;"));CHECK(guidprofile(name));have=0;CHECK(!connectnas(&ep,0,out,CNT(out))&&!called&&!passreads);have=1;addr=0;CHECK(!connectnas(&ep,0,out,CNT(out))&&!called&&!passreads);addr=1;faildial=1;CHECK(!connectnas(&ep,0,out,CNT(out))&&called==1&&closecount==1);faildial=0;forward=0;CHECK(!connectnas(&ep,0,out,CNT(out))&&called==2&&up);forward=1;CHECK(connectnas(&ep,0,out,CNT(out))&&called==2&&passreads==2);EpEnd:;printf("%d VPN profile, authentication, route, reuse checks passed (simulated).\n",count);return 0;}
