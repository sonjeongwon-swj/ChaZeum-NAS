/* OS route/CLI responses are fixtures; no external NAS timing is implied. */
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
static int assertions,queries,ok=1;
static T response=L"@WG:00112233445566778899aabbccddeeff|UNKNOWN";
static T wg=L"@WG:00112233445566778899aabbccddeeff";
static T ts=L"@TS:00112233445566778899aabbccddeeff:nABC23";
#define C(x) do{assertions++;if(!(x)){fprintf(stderr,"secure route check failed at %d\n",__LINE__);exit(1);}}while(0)
static int contains(T a,T b){int n=sz(b);for(;*a;a++){int j=0;while(j<n&&a[j]==b[j])j++;if(j==n)return 1;}return 0;}
int timedprobe(T code,W*out,int n){queries++;C(contains(code,L"Find-NetRoute -RemoteIPAddress $ip"));C(!contains(code,L"test-password"));return ok&&cp(out,n,response);}
int main(void){
 Endpoint e={L"",L"192.168.1.50",L"NASPC"},restore={0};W out[512],saved[330];
 C(samehost(L"Home.Example",L"home.example"));C(!samehost(ts,L"@TS:00112233445566778899aabbccddeeff:nabc23"));C(samehost(ts,ts));C(securepin(wg)==1);C(securepin(ts)==2);C(!securepin(L"@WG:00000000000000000000000000000000"));
 C(!securepin(L"@TS:00112233445566778899aabbccddeeff:bad|peer"));C(!securepin(L"@WG:00112233445566778899aabbccddeeffextra"));
 C(pinroute(&e,out,512)&&eq(e.host,wg));C(contains(out,L"미측정"));C(checkpin(&e,out,512));
 C(invite(saved,CNT(saved),e.host,e.ip,e.machine));C(pair(saved,restore.host,restore.ip,restore.machine)&&eq(restore.host,wg));
 C(!pair(L"CZ5|@WG:00112233445566778899aabbccddeeff|192.168.1.50|NASPC",restore.host,restore.ip,restore.machine));
 C(!pair(L"CZ5T|home.example.org|192.168.1.50|NASPC",restore.host,restore.ip,restore.machine));
 response=L"@WG:11112233445566778899aabbccddeeff|UNKNOWN";C(!checkpin(&e,out,512));
 response=L"@WG:00112233445566778899aabbccddeeff|DIRECT";C(!checkpin(&e,out,512));
 response=L"junk";C(!checkpin(&e,out,512));
 C(!tailip(L"100.63.255.1"));C(tailip(L"100.64.0.1"));C(tailip(L"100.127.255.254"));C(!tailip(L"100.128.0.1"));
 cp(e.ip,CNT(e.ip),L"100.80.2.3");response=L"@TS:00112233445566778899aabbccddeeff:nABC23|UNKNOWN";
 C(pinroute(&e,out,512)&&eq(e.host,ts));C(contains(out,L"확인 안 됨"));
 response=L"@TS:00112233445566778899aabbccddeeff:nABC23|DIRECT";C(checkpin(&e,out,512)&&contains(out,L"직접"));
 response=L"@TS:00112233445566778899aabbccddeeff:nABC23|PEERRELAY";C(checkpin(&e,out,512)&&contains(out,L"다른 기기 중계"));
 response=L"@TS:00112233445566778899aabbccddeeff:nABC23|RELAY";C(checkpin(&e,out,512)&&contains(out,L"서버 경유"));
 response=L"@TS:00112233445566778899aabbccddeeff:nabc23|DIRECT";C(!checkpin(&e,out,512));
 response=L"@TS:00112233445566778899aabbccddeeff:OTHER|DIRECT";C(!checkpin(&e,out,512));
 response=L"@TS:00112233445566778899aabbccddeeff:nABC23|FAKE";C(!checkpin(&e,out,512));
 ok=0;C(!checkpin(&e,out,512));int prev=queries;
 cp(e.ip,CNT(e.ip),L"8.8.8.8");C(!pinroute(&e,out,512)&&queries==prev);
 cp(e.ip,CNT(e.ip),L"10.0.1.1';x");C(!pinroute(&e,out,512)&&queries==prev);
 cp(e.ip,CNT(e.ip),L"127.0.0.1");C(!pinroute(&e,out,512)&&queries==prev);
 C(!securetarget(wg,L"100.80.2.3"));C(!securetarget(ts,L"192.168.1.50"));
 printf("%d stored VPN binding, peer identity, route type, IP bounds and rejection checks passed (simulated).\n",assertions);return 0;
}
