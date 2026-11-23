/* Local read/write experiment using the production benchmark and actual bytes/files.
   Only Win32 I/O bindings & routing are simulated. NOT a network benchmark. */
#include "client.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/random.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
static const char *local;static int wr=-1,rd=-1,corrupt,deny,routeok=1;static U64 written,readbytes;
int getmap(T letter,W*b,int n){return eq(letter,L"X:")&&cp(b,n,L"CZ5|home.example.net|192.168.1.90|NASBOX|자료|alpha");}U32 WNetGetConnectionW(T l,W*b,U32*n){(void)l;cp(b,*n,L"\\\\192.168.1.90\\자료");return 0;}int route5(const Endpoint*ep,W*b,int n){(void)ep;cp(b,n,L"simulated");return routeok;}
int guidtext(W*b,int n){return cp(b,n,L"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");}U32 GetLastError(void){return errno;}
H VirtualAlloc(H h,size_t n,U32 x,U32 y){(void)h;(void)x;(void)y;return malloc(n);}I32 VirtualFree(H h,size_t n,U32 x){(void)n;(void)x;free(h);return 1;}
int BCryptGenRandom(H h,U8*b,U32 n,U32 f){(void)h;(void)f;while(n){ssize_t r=getrandom(b,n>1048576?1048576:n,0);if(r<=0)return -1;b+=r;n-=r;}return 0;}
U64 GetTickCount64(void){struct timespec a;clock_gettime(CLOCK_MONOTONIC,&a);return 1000*(U64)a.tv_sec+(U64)a.tv_nsec/1000000;}
H CreateFileW(T p,U32 a,U32 share,H sa,U32 create,U32 flags,H tmpl){(void)p;(void)a;(void)share;(void)sa;(void)tmpl;if(deny)return BAD;if(create==1){if((flags&0x04000000)==0)return BAD;wr=open(local,O_RDWR|O_EXCL|O_CREAT,0600);return wr<0?BAD:(H)(N)(wr+1);}rd=open(local,O_RDONLY);return rd<0?BAD:(H)(N)(rd+1);}
I32 WriteFile(H h,const void*b,U32 n,U32*c,H x){(void)x;ssize_t s=write((int)(N)h-1,b,n);if(s<0)return 0;*c=s;written+=s;return 1;}
I32 ReadFile(H h,H b,U32 n,U32*c,H x){(void)x;ssize_t s=read((int)(N)h-1,b,n);if(s<0)return 0;*c=s;readbytes+=s;if(corrupt)((unsigned char*)b)[111]^=0x04;return 1;}
I32 FlushFileBuffers(H h){return !fsync((int)(N)h-1);}I32 GetFileSizeEx(H h,int64_t*n){struct stat st;if(fstat((int)(N)h-1,&st))return 0;*n=st.st_size;return 1;}I32 CloseHandle(H h){int fd=(int)(N)h-1;close(fd);if(fd==wr){unlink(local);wr=-1;}return 1;}
int errnum(W*out,int n,T s,U32 e){(void)e;cp(out,n,s);return 0;}int _snwprintf(W*out,size_t n,T fmt,...){if(eq(fmt,L"\\\\%ls\\"))cp(out,n,L"\\\\192.168.1.90\\");else cp(out,n,L"completed");return sz(out);}
int main(int argc,char**argv){if(argc!=2)return 3;local=argv[1];W msg[1000];Endpoint ep={L"home.example.net",L"192.168.1.90",L"NASBOX"};routeok=0;if(driveprobe(&ep,L"X:",msg,CNT(msg))||written)return 1;routeok=1;deny=1;if(driveprobe(&ep,L"X:",msg,CNT(msg))||written||access(local,F_OK)==0)return 5;deny=0;if(!driveprobe(&ep,L"X:",msg,CNT(msg))||written!=268435456||readbytes!=268435456||access(local,F_OK)==0)return 2;corrupt=1;if(driveprobe(&ep,L"X:",msg,CNT(msg))||access(local,F_OK)==0)return 4;printf("256 MiB write, fresh read handle, byte compare, cleanup, corrupt-read rejection, simulated read-only denial and absent-route rejection passed on LOCAL filesystem.\n");return 0;}
