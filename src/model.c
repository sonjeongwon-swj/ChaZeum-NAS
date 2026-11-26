#include "base.h"
int sz(T s){int n=0;if(s)while(s[n])n++;return n;}
int cp(W*b,int cap,T s){int n=sz(s);if(!b||cap<1||!s||n>=cap){if(b&&cap)b[0]=0;return 0;}for(int i=0;i<=n;i++)b[i]=s[i];return 1;}
int cat(W*b,int cap,T s){int n=sz(b);return n<cap&&cp(b+n,cap-n,s);}
void erase(void*p,size_t n){volatile U8*b=p;while(n--)*b++=0;}
static int fold(W a){return a>=L'A'&&a<=L'Z'?a+32:a;}
int eq(T a,T b){if(!a||!b)return 0;while(*a&&fold(*a)==fold(*b)){a++;b++;}return fold(*a)==fold(*b);}
static int al(W c){return (c>='0'&&c<='9')||(fold(c)>='a'&&fold(c)<='z');}
static int octets(T s,int o[4]){if(!s)return 0;for(int i=0;i<4;i++){int n=0,d=0;if(*s=='0'&&s[1]>='0'&&s[1]<='9')return 0;while(*s>='0'&&*s<='9'){n=n*10+*s++-'0';if(++d>3||n>255)return 0;}if(!d||(*s!=(i<3?'.':0)))return 0;o[i]=n;if(i<3)s++;}return 1;}
int ipprivate(T s){int o[4];return octets(s,o)&&((o[0]==10)||(o[0]==172&&o[1]>=16&&o[1]<=31)||(o[0]==192&&o[1]==168));}
int tailip(T s){int o[4];return octets(s,o)&&o[0]==100&&o[1]>=64&&o[1]<=127;}
/* A machine-local adapter GUID. Tailscale also binds the stable remote node ID.
   These are identities, never command arguments, server names or credentials. */
int securepin(T s){
 int n=sz(s);if(n<36||n>101||s[0]!='@'||s[3]!=':')return 0;
 int wg=s[1]=='W'&&s[2]=='G',ts=s[1]=='T'&&s[2]=='S';if(!wg&&!ts)return 0;
 int nonzero=0;for(int i=4;i<36;i++){W c=s[i];if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')))return 0;nonzero|=c!='0';}
 if(!nonzero)return 0;if(wg)return n==36?1:0;
 if(n<38||s[36]!=':')return 0;
 for(int i=37;i<n;i++)if(!al(s[i])&&s[i]!='-'&&s[i]!='_')return 0;
 return 2;
}
int samehost(T a,T b){
 if(!a||!b)return 0;if(a[0]!='@'&&b[0]!='@')return eq(a,b);
 while(*a&&*a==*b){a++;b++;}return *a==*b;
}
int securetarget(T pin,T ip){int k=securepin(pin);return k==1?ipprivate(ip):k==2?tailip(ip):0;}
int hostok(T s){int n=sz(s),o[4];if(n<3||n>253)return 0;if(octets(s,o))return !ipprivate(s)&&o[0]>0&&o[0]<224&&o[0]!=127&&!(o[0]==169&&o[1]==254)&&!(o[0]==100&&o[1]>=64&&o[1]<=127);int dots=0,len=0,hasLetter=0;for(int i=0;i<=n;i++){W c=s[i];if(c=='.'||c==0){if(!len||len>63||s[i-1]=='-')return 0;len=0;dots+=c=='.';}else{if((!al(c)&&c!='-')||(!len&&c=='-'))return 0;len++;if(fold(c)>='a'&&fold(c)<='z')hasLetter=1;}}return dots&&hasLetter;}
int machineok(T s){int n=sz(s);if(n<1||n>15)return 0;for(int i=0;i<n;i++)if(!al(s[i])&&s[i]!='-'&&!(s[i]>=0xac00&&s[i]<=0xd7a3))return 0;return s[0]!='-'&&s[n-1]!='-';}
/* A Windows local name, not an ASCII slug. Retain Windows' delimiters,
   the local account length boundary, and separation from its built-in accounts. */
int idok(T s){
 int n=sz(s);if(!n||n>20||s[n-1]=='.')return 0;
 T invalid=L"\",/\\[]:|<>+=;?*";for(int i=0;i<n;i++){if(s[i]<32)return 0;for(T p=invalid;*p;p++)if(s[i]==*p)return 0;}
 return !eq(s,L"administrator")&&!eq(s,L"guest")&&!eq(s,L"defaultaccount")&&!eq(s,L"wdagutilityaccount");
}
int shareok(T s){int n=sz(s);if(n<1||n>60||s[0]==' '||s[n-1]==' '||s[n-1]=='.')return 0;for(int i=0;i<n;i++){W c=s[i];if(!(al(c)||(c>=0xac00&&c<=0xd7a3)||c==' '||c=='-'||c=='_'))return 0;}return !eq(s,L"IPC")&&!eq(s,L"ADMIN")&&!eq(s,L"PRINT");}
int letterok(T s){return s&&sz(s)==2&&fold(s[0])>='a'&&fold(s[0])<='z'&&s[1]==':';}
int validend(const Endpoint*e){return machineok(e->machine)&&((hostok(e->host)&&ipprivate(e->ip))||securetarget(e->host,e->ip));}
int invite(W*out,int cap,T h,T ip,T pc){int sc=securetarget(h,ip);if((!sc&&(!hostok(h)||!ipprivate(ip)))||!machineok(pc))return 0;return cp(out,cap,sc?L"CZ5T|":L"CZ5|")&&cat(out,cap,h)&&cat(out,cap,L"|")&&cat(out,cap,ip)&&cat(out,cap,L"|")&&cat(out,cap,pc);}
int pair(T s,W*h,W*ip,W*pc){W temp[310];if(!cp(temp,CNT(temp),s))return 0;W*a[4]={temp};int j=1;for(W*p=temp;*p;p++)if(*p=='|'){if(j==4)return 0;*p=0;a[j++]=p+1;}if(j!=4||!machineok(a[3]))return 0;if(eq(a[0],L"CZ5T")){if(!securetarget(a[1],a[2]))return 0;}else if(!eq(a[0],L"CZ5")||!hostok(a[1])||!ipprivate(a[2]))return 0;return cp(h,254,a[1])&&cp(ip,16,a[2])&&cp(pc,16,a[3]);}

int rootpath(T path){
 return sz(path)==3&&((path[0]>='A'&&path[0]<='Z')||(path[0]>='a'&&path[0]<='z'))&&path[1]==':'&&path[2]=='\\';
}
