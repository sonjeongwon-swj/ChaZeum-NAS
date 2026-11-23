# Integration test on a disposable GitHub-hosted Windows VM, never the user's PC.
$ErrorActionPreference='Stop'
if ($env:GITHUB_ACTIONS -ne 'true' -or $env:RUNNER_ENVIRONMENT -ne 'github-hosted') { throw 'Hosted test VM only.' }
Add-Type -Path "$PSScriptRoot/windows-ui.cs"
$tag=[Guid]::NewGuid().ToString('N').Substring(0,9)
$tempDir=Join-Path $env:RUNNER_TEMP "cz-$tag"
$folder=Join-Path $tempDir 'files'
$share="CZtest$tag"
$pcName=$env:COMPUTERNAME
$nasId="강사$tag";$testPassword=[Guid]::NewGuid().ToString('N')+'aZ!'
$madeUser=$false
$pairSpec='CZ5|nas.example.invalid|192.168.7.24|TEST-NAS'
$mapSpec="$pairSpec|자료|demo"
$clientKey='HKCU:\Software\ChazeumNAS\Five\Client'
$runKey='HKCU:\Software\Microsoft\Windows\CurrentVersion\Run'
$oldPair=Join-Path $tempDir 'Chazeum-NAS_USER.exe'
if (Test-Path $clientKey) {throw 'Unexpected existing client settings; stopping.'}
New-Item -ItemType Directory $folder -Force | Out-Null
function Wait-Cz([scriptblock]$probe,[string]$message){
 $watch=[Diagnostics.Stopwatch]::StartNew()
 do { $v=& $probe; if ($v) {return $v};Start-Sleep -Milliseconds 130 } while ($watch.Elapsed.TotalSeconds -lt 22)
 throw "Timed out: $message"
}
function Open-Cz([string]$file,[string]$cls){
 $script:app=Start-Process -FilePath $file -PassThru
 $script:win=Wait-Cz { if($app.HasExited){throw 'App exited early'}; $h=[CzWindow]::FindOwned($app.Id,$cls);if($h -ne [IntPtr]::Zero){$h} } 'main window'
}
function Stop-Cz(){
 if($null -eq $app){return}
 if(-not $app.CloseMainWindow() -or -not $app.WaitForExit(10000) -or $app.ExitCode -ne 0){throw 'Window did not close normally'}
 $app.Dispose();$script:app=$null
}
function LoginCommand-Cz {
 $k=[Microsoft.Win32.Registry]::CurrentUser.OpenSubKey('Software\Microsoft\Windows\CurrentVersion\Run')
 if($null -eq $k){return $null}
 try{return $k.GetValue('ChaZeumNAS5',$null)}finally{$k.Close()}
}
function Control-Cz([int]$id){ $null=Wait-Cz { [CzWindow]::GetDlgItem($win,$id) -ne [IntPtr]::Zero } "control $id" }
function Tab-Cz([int]$id,[int]$child){[CzWindow]::Click($win,$id);Control-Cz $child}
function Allow-Cz(){
 $dialog=Wait-Cz { $v=[CzWindow]::GetLastActivePopup($win);if($v -ne $win -and $v -ne [IntPtr]::Zero -and [CzWindow]::GetDlgItem($v,6) -ne [IntPtr]::Zero){$v} } 'the app confirmation'
 [CzWindow]::Click($dialog,6)
}
function Result-Cz([string]$prefix){
 try { $null=Wait-Cz { [CzWindow]::Read($win,501).StartsWith($prefix) } $prefix }
 catch { throw ("Expected: {0}; app reported: {1}" -f $prefix,[CzWindow]::Read($win,501)) }
}
$app=$null;$registered=$false
try {
 # The old reviewed client saves its real endpoint and opts into its old startup copy.
 Copy-Item 'ci-old/ChaZeumClient.exe' $oldPair
 Open-Cz $oldPair 'ChazeumNAS5Client';Control-Cz 203
 [CzWindow]::Set($win,400,'nas.example.invalid');[CzWindow]::Set($win,401,'192.168.7.24');[CzWindow]::Set($win,402,'TEST-NAS')
 [CzWindow]::Click($win,203);Result-Cz '서버 주소를 저장했습니다'
 Tab-Cz 103 219;[CzWindow]::Click($win,219);Allow-Cz
 Result-Cz '다음 Windows 로그인 때'
 $null=Wait-Cz { (LoginCommand-Cz) -like '*Client-*.exe*' } 'old startup copy'
 Stop-Cz
 # Synthetic saved letter, no credential or outside connection is created for it.
 New-ItemProperty $clientKey -Name 'Drive-X' -Value $mapSpec -Force | Out-Null
 Copy-Item 'unsigned/Chazeum-NAS_USER.exe' $oldPair -Force
 Open-Cz $oldPair 'ChazeumNAS5Client';Allow-Cz;Control-Cz 216
 $null=Wait-Cz { (LoginCommand-Cz) -eq ('"'+$oldPair+'" /reconnect') } 'migration to stable path'
 if((Get-ItemPropertyValue $clientKey -Name Pair) -cne $pairSpec -or (Get-ItemPropertyValue $clientKey -Name 'Drive-X') -cne $mapSpec){throw 'Saved settings changed'}
 Tab-Cz 100 203
 if([CzWindow]::Read($win,400) -cne 'nas.example.invalid' -or [CzWindow]::Read($win,401) -cne '192.168.7.24'){throw 'New client failed to load old address'}
 Stop-Cz
 'PASS old client to new file: actual stored address/opt-in migrated; saved letter record retained.'
 # Actual new server GUI creates the share; no firewall, public endpoint or VPN change.
 $server=(Resolve-Path 'unsigned/Chazeum-NAS_SERVER.exe').Path
 if((Get-Service LanmanServer).Status -ne 'Running'){Start-Service LanmanServer}
 Open-Cz $server 'ChazeumNAS5Server';Control-Cz 507
 if([CzWindow]::Read($win,507) -notmatch '실행 중' -or [CzWindow]::ClassLong($win,-14) -eq [IntPtr]::Zero -or [CzWindow]::ClassLong($win,-34) -eq [IntPtr]::Zero){throw 'Service state or icons missing'}
 Tab-Cz 101 206;[CzWindow]::Set($win,400,$folder);[CzWindow]::Set($win,401,$share)
 [CzWindow]::Click($win,206);Allow-Cz;Result-Cz '폴더를 공유했습니다';$registered=$true
 $s=Get-SmbShare -Name $share
 if($s.Path -ne $folder -or $s.Description -notmatch '^ChaZeumNAS5:[a-f0-9]{32}$'){throw 'Wrong share metadata'}
 # A nested folder must produce a precise conflict, without changing either share.
 $nested=Join-Path $folder 'nested';New-Item -ItemType Directory $nested | Out-Null
 [CzWindow]::Set($win,400,$nested);[CzWindow]::Set($win,401,"nested$tag");[CzWindow]::Click($win,206);Allow-Cz
 Result-Cz '이번 공유는 추가하지 않았습니다'
 $explain=[CzWindow]::Read($win,501)
 if(!$explain.Contains($share) -or !$explain.Contains($folder)){throw 'Missing conflict name/path'}
 Tab-Cz 102 208;[CzWindow]::Set($win,400,$nasId);[CzWindow]::Set($win,401,$testPassword);[CzWindow]::Click($win,208);Allow-Cz;Result-Cz 'NAS 아이디를 만들었습니다';$madeUser=$true
 [CzWindow]::Set($win,400,$nasId);[CzWindow]::Set($win,402,$share);[CzWindow]::Index($win,403,1)
 [CzWindow]::Click($win,209);Allow-Cz;Result-Cz '권한을 저장했습니다'
 $acl=(Get-Acl $folder).Sddl
 Tab-Cz 105 507
 if([CzWindow]::Read($win,507) -notmatch '등록한 공유: [1-9]'){throw 'New share not counted'}
 Stop-Cz
 $unc="\\$pcName\$share\check.txt"
 $marker=[Guid]::NewGuid().ToString()
 [IO.File]::WriteAllText($unc,$marker)
 if([IO.File]::ReadAllText($unc) -cne $marker){throw 'SMB saved contents changed after closing UI'}
 $identityCheck=[Action[string]]{
   param($expectedName)
   $client=Wait-Cz {
    $names=@(Get-SmbOpenFile | Where-Object { $_.Path -eq (Join-Path $folder 'check.txt') } | ForEach-Object ClientUserName)
    if($names -contains $expectedName){$expectedName}
   } 'server did not report the NAS test identity for the open SMB file'
   Write-Host "PASS authenticated SMB identity: $client"
 }
 $aclReport=Get-SmbShareAccess -Name $share | Select-Object AccountName,AccessControlType,AccessRight
 $aclReport | Format-Table | Out-String | Write-Host
 $groups=Get-LocalGroup | Where-Object { $_.Description -eq $s.Description }
 foreach($g in $groups){$members=@(Get-LocalGroupMember $g.Name | ForEach-Object Name);Write-Host $g.Name ($members -join ',')}
 [CzWindow]::TestShare($nasId,$testPassword,$share,$marker,$false,$identityCheck)
 Open-Cz $server 'ChazeumNAS5Server';Control-Cz 507;Tab-Cz 102 209
 [CzWindow]::Set($win,400,$nasId);[CzWindow]::Set($win,402,$share);[CzWindow]::Index($win,403,2)
 [CzWindow]::Click($win,209);Allow-Cz;Result-Cz '권한을 저장했습니다';Stop-Cz
 [CzWindow]::TestShare($nasId,$testPassword,$share,$marker,$true,$identityCheck)
 'PASS Korean NAS ID: allowed read/denied write and then allowed write/reopen with both management windows closed; overlap diagnosis named the existing share.'
 # Simulate a subsequent same-location overwrite; ensure existing ACL and content survive.
 $fixed=Join-Path $tempDir 'Chazeum-NAS_SERVER.exe'
 Copy-Item $server $fixed;Open-Cz $fixed 'ChazeumNAS5Server';Control-Cz 507;Stop-Cz
 Copy-Item $server $fixed -Force;Open-Cz $fixed 'ChazeumNAS5Server';Control-Cz 507
 if((Get-Acl $folder).Sddl -cne $acl -or [IO.File]::ReadAllText($unc) -cne $marker){throw 'Changed after replacement'}
 'PASS actual GUI-created local SMB share: write/reopen after closing the server and after same-location copy; settings/ACL retained.'
 # Let this app remove its own temporary share and groups.
 Tab-Cz 101 207;[CzWindow]::Set($win,401,$share);[CzWindow]::Click($win,207);Allow-Cz
 $null=Wait-Cz { -not (Get-SmbShare -Name $share -ErrorAction SilentlyContinue) } 'remove temporary share'
 Stop-Cz;$registered=$false
} finally {
 if($null -ne $app){if(-not $app.HasExited){$app.Kill();$app.WaitForExit()};$app.Dispose()}
 if($registered -and (Get-SmbShare -Name $share -ErrorAction SilentlyContinue)){Remove-SmbShare -Name $share -Force}
 if($madeUser){Remove-LocalUser -Name $nasId -ErrorAction SilentlyContinue}
 $k=[Microsoft.Win32.Registry]::CurrentUser.OpenSubKey('Software\Microsoft\Windows\CurrentVersion\Run',$true)
 if($k){try{$k.DeleteValue('ChaZeumNAS5',$false)}finally{$k.Close()}}
 if(Test-Path $clientKey){Remove-Item -LiteralPath $clientKey -Recurse}
 if(Test-Path $tempDir){Remove-Item -LiteralPath $tempDir -Force -Recurse}
}
