#include "client.h"

/* Only the validated numeric destination reaches a command. No path entered by
   the user, alias, password, config file, public IP or private key is read.
   Bind the OS-selected interface; with Tailscale bind the peer identity too. */
extern int timedprobe(T,W*,int);
static T routeProbe =
 L"$r=@(Find-NetRoute -RemoteIPAddress $ip | Where-Object { $_.DestinationPrefix });"
 L"if($r.Count -ne 1){return};"
 L"$a=Get-NetAdapter -IncludeHidden -InterfaceIndex $r[0].InterfaceIndex;"
 L"if($a.Status -ne 'Up'){return};$id=([guid]$a.InterfaceGuid).ToString('N');"
 L"if($tail){"
 L"if($a.InterfaceDescription -notmatch '^(Tailscale( Tunnel)?|WireGuard Tunnel|Wintun Userspace Tunnel)( #[0-9]+)?$'){return};"
 L"if((Get-Service -Name Tailscale -ErrorAction SilentlyContinue).Status -ne 'Running'){return};"
 L"$cli=Join-Path ([Environment]::GetFolderPath('ProgramFiles')) 'Tailscale\\tailscale.exe';"
 L"if(!(Test-Path -LiteralPath $cli -PathType Leaf)){return};"
 L"$j=(& $cli status --json | Out-String | ConvertFrom-Json);if($LASTEXITCODE -ne 0 -or $j.BackendState -ne 'Running'){return};"
 L"$src=@(Get-NetIPAddress -InterfaceIndex $a.ifIndex -AddressFamily IPv4 | Where-Object {$j.Self.TailscaleIPs -contains $_.IPAddress});"
 L"if($src.Count -ne 1){return};$p=@($j.Peer.PSObject.Properties | ForEach-Object {$_.Value} | Where-Object {$_.TailscaleIPs -contains $ip});"
 L"if($p.Count -ne 1 -or !$p[0].Online -or !$p[0].ID){return};$p=$p[0];"
 L"$path='UNKNOWN';if($p.Active){if($p.CurAddr){$path='DIRECT'}elseif($p.PeerRelay){$path='PEERRELAY'}elseif($p.Relay){$path='RELAY'}};"
 L"'@TS:'+$id+':'+$p.ID+'|'+$path"
 L"}else{"
 L"if($a.InterfaceDescription -notmatch '^(WireGuard Tunnel|Wintun Userspace Tunnel)( #[0-9]+)?$'){return};"
 L"$svc=@(Get-Service | Where-Object { $_.Name -eq ('WireGuardTunnel$'+$a.Name) -and $_.Status -eq 'Running' });"
 L"if($svc.Count -eq 1){'@WG:'+$id+'|UNKNOWN'}}";

static int observed(T ip,W *identity,int identcap,W *out,int cap){
 int tail=tailip(ip); if(!tail&&!ipprivate(ip)){cp(out,cap,L"집 PC의 내부 IPv4 또는 Tailscale 전용 IPv4를 입력하세요.");return 0;}
 W cmd[4100]=L"$ip='",answer[160]={0};
 if(!cat(cmd,CNT(cmd),ip)||!cat(cmd,CNT(cmd),tail?L"';$tail=$true;":L"';$tail=$false;")||!cat(cmd,CNT(cmd),routeProbe)||
    !timedprobe(cmd,answer,CNT(answer)))goto missing;
 W*bar=answer;while(*bar&&*bar!='|')bar++;if(!*bar)goto missing;*bar++=0;
 if(!securetarget(answer,ip)||!cp(identity,identcap,answer))goto missing;
 if(!tail&&eq(bar,L"UNKNOWN"))return cp(out,cap,L"WireGuard · 등록한 터널 경로 (직결·중계 미측정)");
 if(tail&&eq(bar,L"DIRECT"))return cp(out,cap,L"Tailscale · 직접 연결");
 if(tail&&eq(bar,L"RELAY"))return cp(out,cap,L"Tailscale · 중계 서버 경유");
 if(tail&&eq(bar,L"PEERRELAY"))return cp(out,cap,L"Tailscale · 다른 기기 중계");
 if(tail&&eq(bar,L"UNKNOWN"))return cp(out,cap,L"Tailscale · 직결·중계 아직 확인 안 됨");
missing:
 cp(out,cap,tail?L"이 주소로 연결된 Tailscale NAS를 확인하지 못했습니다. 두 PC가 같은 개인망에 연결됐는지 확인하세요.":L"집 PC로 가는 WireGuard 터널을 확인하지 못했습니다. 노트북에서 집 터널을 연결하고 다시 누르세요.");return 0;
}
int pinroute(Endpoint *e,W*out,int cap){
 W pin[108];if(!machineok(e->machine)){cp(out,cap,L"집에 있는 NAS PC의 Windows 컴퓨터 이름을 입력하세요.");return 0;}
 return observed(e->ip,pin,CNT(pin),out,cap)&&cp(e->host,CNT(e->host),pin);
}
int checkpin(const Endpoint*e,W*out,int cap){
 W actual[108];if(!validend(e)||!securepin(e->host)||!observed(e->ip,actual,CNT(actual),out,cap))return 0;
 if(sz(actual)!=sz(e->host)||memcmp(actual,e->host,(size_t)sz(actual)*sizeof(W))!=0){cp(out,cap,L"처음 등록한 보안망 또는 NAS 기기와 달라 접속을 멈췄습니다. NAS 로그인 정보를 보내지 않았습니다.");return 0;}return 1;
}
