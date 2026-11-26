# Only for the disposable hosted Windows runner. No household addresses, keys,
# accounts, port forwarding, SMB traffic or Internet-facing test endpoint.
$ErrorActionPreference = 'Stop'
if ($env:GITHUB_ACTIONS -ne 'true' -or $env:RUNNER_ENVIRONMENT -ne 'github-hosted' -or
    $env:RUNNER_OS -ne 'Windows' -or !$env:RUNNER_TEMP) { throw 'Disposable hosted Windows runner only' }
Add-Type -Path "$PSScriptRoot/windows-ui.cs"
$exe = (Resolve-Path 'unsigned/Chazeum-NAS_USER.exe').Path
$pins = Get-Content -Raw "$PSScriptRoot/../release/unsigned-sha256.json" | ConvertFrom-Json
if ((Get-FileHash $exe).Hash -ne $pins.Client) { throw 'Unexpected NAS client bytes' }
$store = 'HKCU:\Software\ChazeumNAS\Five\Client'
if (Test-Path $store) { throw 'Refusing to change an existing client profile' }
$suffix = [Guid]::NewGuid().ToString('N').Substring(0,8)
$left = "CzA$suffix"; $right = "CzB$suffix"
$dir = Join-Path $env:RUNNER_TEMP "cz-wg-$suffix"
$msi = Join-Path $dir 'wg.msi'
$homeIP = '192.168.246.99'
$leftIP = '10.241.197.1'; $rightIP = '10.241.197.2'
$wg = Join-Path $env:ProgramFiles 'WireGuard\wg.exe'
$wireguard = Join-Path $env:ProgramFiles 'WireGuard\wireguard.exe'
$created = [Collections.Generic.List[string]]::new()
$app = $null
function Wait-Fixture([scriptblock]$probe,[string]$description) {
    $t=[Diagnostics.Stopwatch]::StartNew()
    do {
        $v = & $probe
        if ($v) { return $v }
        Start-Sleep -Milliseconds 220
    } while ($t.Elapsed.TotalSeconds -lt 42)
    throw "Timed out: $description"
}
function Port-Pair {
    $a=[Net.Sockets.UdpClient]::new([Net.IPEndPoint]::new([Net.IPAddress]::Loopback,0))
    try {
        $b=[Net.Sockets.UdpClient]::new([Net.IPEndPoint]::new([Net.IPAddress]::Loopback,0))
        try { return @($a.Client.LocalEndPoint.Port,$b.Client.LocalEndPoint.Port) } finally { $b.Dispose() }
    } finally { $a.Dispose() }
}
function Add-Fixture([string]$name,[string]$localIP,[int]$port,[int]$peerPort,[string]$private,[string]$peer,[string]$allow) {
    if (Get-Service -Name ('WireGuardTunnel$'+$name) -ErrorAction SilentlyContinue) { throw 'Existing service' }
    $conf = Join-Path $dir "$name.conf"
    # The temporary keys are generated on this runner; never print or upload them.
    [IO.File]::WriteAllLines($conf,@('[Interface]',"PrivateKey = $private","Address = $localIP/32","ListenPort = $port",
        '[Peer]',"PublicKey = $peer","AllowedIPs = $allow","Endpoint = 127.0.0.1:$peerPort",'PersistentKeepalive = 2'),[Text.Encoding]::ASCII)
    $created.Add($name)
    $p=Start-Process $wireguard -ArgumentList @('/installtunnelservice',('"' + $conf + '"')) -Wait -PassThru
    if ($p.ExitCode -ne 0) { throw "Fixture install failed ($($p.ExitCode))" }
    $null = Wait-Fixture { (Get-Service -Name ('WireGuardTunnel$'+$name) -ErrorAction SilentlyContinue).Status -eq 'Running' } 'fixture service startup'
}
function Has-Handshake([string]$name,[string]$expected,[long]$since) {
    $answer = @(& $wg show $name latest-handshakes)
    if ($LASTEXITCODE -ne 0 -or $answer.Count -ne 1) {return $false}
    $parts=$answer[0].Trim() -split '\s+'
    return ($parts.Count -eq 2 -and $parts[0] -ceq $expected -and [long]$parts[1] -ge $since -and [long]$parts[1] -gt 0)
}
try {
    $null=New-Item -ItemType Directory -Path $dir
    $sid=[Security.Principal.WindowsIdentity]::GetCurrent().User.Value
    & icacls.exe $dir '/inheritance:r' '/grant:r' ('*'+$sid+':(OI)(CI)F') '*S-1-5-18:(OI)(CI)F' | Out-Null
    if ($LASTEXITCODE -ne 0) { throw 'Could not protect temporary fixture directory' }
    $uri='https://download.wireguard.com/windows-client/wireguard-amd64-1.1.1.msi'
    Invoke-WebRequest -Uri $uri -OutFile $msi -TimeoutSec 60
    # x64 digest from Microsoft's winget-pkgs WireGuard.WireGuard 1.1.1 manifest.
    if ((Get-FileHash $msi).Hash -ne '7BFED60AD61B785C914B38B61555A975488E1D3EC472DBFB2FCDF498FCA75242') {throw 'MSI digest mismatch'}
    $sig=Get-AuthenticodeSignature $msi
    if ($sig.Status -ne 'Valid' -or $sig.SignerCertificate.GetNameInfo('SimpleName',$false) -ne 'WireGuard LLC') {throw 'MSI publisher mismatch'}
    $p=Start-Process msiexec.exe -ArgumentList @('/i',('"' + $msi + '"'),'/qn','/norestart','DO_NOT_LAUNCH=1') -Wait -PassThru
    if ($p.ExitCode -ne 0) {throw "WireGuard MSI returned $($p.ExitCode)"}
    $ports=Port-Pair
    $a=(& $wg genkey); $b=(& $wg genkey)
    $pubA=($a | & $wg pubkey); $pubB=($b | & $wg pubkey)
    if ($LASTEXITCODE -ne 0 -or !$pubA -or !$pubB -or $pubA -ceq $pubB) {throw 'Failed generating temporary key pairs'}
    $since=[DateTimeOffset]::UtcNow.ToUnixTimeSeconds()-2
    Add-Fixture $right $rightIP $ports[1] $ports[0] $b $pubA "$leftIP/32"
    Add-Fixture $left $leftIP $ports[0] $ports[1] $a $pubB "$homeIP/32"
    $a=$null;$b=$null
    $null=Wait-Fixture { (Has-Handshake $left $pubB $since) -and (Has-Handshake $right $pubA $since) } 'authenticated loopback handshake at both peers'
    'PASS actual WireGuardNT: both isolated peers completed fresh authenticated handshakes over loopback.'
    $adapter=Get-NetAdapter -Name $left -IncludeHidden
    $guid=([Guid]$adapter.InterfaceGuid).ToString('N')
    $route=@(Find-NetRoute -RemoteIPAddress $homeIP | Where-Object { $_.DestinationPrefix })
    if ($route.Count -ne 1 -or $route[0].InterfaceIndex -ne $adapter.ifIndex -or $route[0].DestinationPrefix -ne "$homeIP/32") {throw 'Fixture host route not selected'}
    $app=Start-Process $exe -PassThru
    $win=Wait-Fixture { $h=[CzWindow]::FindOwned($app.Id,'ChazeumNAS5Client');if($h -ne [IntPtr]::Zero){$h} } 'client main window'
    [CzWindow]::Set($win,401,$homeIP);[CzWindow]::Set($win,402,'TEST-NAS')
    [CzWindow]::Click($win,232)
    $dialog=Wait-Fixture { $h=[CzWindow]::GetLastActivePopup($win);if($h -ne $win -and $h -ne [IntPtr]::Zero -and [CzWindow]::GetDlgItem($h,6) -ne [IntPtr]::Zero){$h} } 'confirmation'
    [CzWindow]::Click($dialog,6)
    $null=Wait-Fixture { [CzWindow]::GetDlgItem($win,216) -ne [IntPtr]::Zero } 'automatic navigation to drive selection'
    $saved=Get-ItemPropertyValue $store Pair
    if($saved -cne ('CZ5T|@WG:'+$guid+'|'+$homeIP+'|TEST-NAS')){throw 'The GUI did not save the actual tunnel identity'}
    'PASS unmodified user EXE: saved the actual WireGuard adapter and opened drive selection without a domain.'
    [CzWindow]::Click($win,103)
    $null=Wait-Fixture { [CzWindow]::GetDlgItem($win,233) -ne [IntPtr]::Zero } 'connection check page'
    [CzWindow]::Click($win,233)
    $null=Wait-Fixture { [CzWindow]::Read($win,501).StartsWith('WireGuard · 등록한 터널 경로') } 'active route detection'
    Stop-Service -Name ('WireGuardTunnel$'+$left) -ErrorAction Stop
    [CzWindow]::Click($win,233)
    $null=Wait-Fixture { [CzWindow]::Read($win,501).StartsWith('집 PC로 가는 WireGuard 터널을 확인하지 못했습니다') } 'refuse missing tunnel'
    if ((Get-ItemPropertyValue $store Pair) -cne $saved) {throw 'Refused connection overwrote saved endpoint'}
    'PASS unmodified user EXE: the active path was confirmed; stopping the tunnel was detected and the saved settings were preserved.'
    'Scope: disposable Windows VM and loopback WireGuard; no household devices, physical WAN, SMB bytes through this tunnel or throughput are represented.'
} finally {
    if($app){if(!$app.HasExited){$null=$app.CloseMainWindow();if(!$app.WaitForExit(5000)){$app.Kill();$app.WaitForExit()}};$app.Dispose()}
    foreach($n in $created){if(Get-Service -Name ('WireGuardTunnel$'+$n) -ErrorAction SilentlyContinue){$null=Start-Process $wireguard -ArgumentList @('/uninstalltunnelservice',$n) -Wait -PassThru}}
    if(Test-Path $store){Remove-Item -LiteralPath $store -Recurse -Force}
    if(Test-Path $dir){Remove-Item -LiteralPath $dir -Recurse -Force}
}
