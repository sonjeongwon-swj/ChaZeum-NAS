$ErrorActionPreference='Stop'
if($env:GITHUB_ACTIONS -ne 'true' -or $env:RUNNER_ENVIRONMENT -ne 'github-hosted'){throw 'Isolated GitHub VM only'}
$cc=Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6/ISCC.exe'
$v=[version]((Get-Content release/VERSION -Raw).Trim())
$in=(Resolve-Path unsigned).Path
$out=New-Item -ItemType Directory -Path setup-test -Force
function Run-Setup($exe){$p=Start-Process $exe -ArgumentList '/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART','/NORESTARTAPPLICATIONS' -Wait -PassThru;if($p.ExitCode -ne 0){throw "Unexpected installer result $($p.ExitCode)"}}
foreach($role in @('SERVER','USER')){
 $file='Chazeum-NAS_'+$role+'.exe';$base='Chazeum-NAS_'+$role
 $ms=($v.Major -shl 16) -bor $v.Minor;$ls=($v.Build -shl 16) -bor $v.Revision
 & $cc /Qp ("/DRole="+$role) ("/DVer="+$v.ToString()) ("/DVerMS="+$ms) ("/DVerLS="+$ls) ("/DBinPath="+$in) ("/DOutPath="+$out.FullName) 'installer/desktop.iss'
 if($LASTEXITCODE -ne 0){throw 'Could not build installer'}
 $setup=Join-Path $out.FullName $file
 $dest=if($role -eq 'SERVER'){Join-Path $env:ProgramFiles 'ChazeumNAS/Server'}else{Join-Path $env:LOCALAPPDATA 'Programs/ChazeumNAS/User'}
 $links=if($role -eq 'SERVER'){[Environment]::GetFolderPath('CommonPrograms')}else{[Environment]::GetFolderPath('Programs')}
 $hive=if($role -eq 'SERVER'){'HKLM:\Software\ChazeumNAS\Five\Server'}else{'HKCU:\Software\ChazeumNAS\Five\Client'}
 $link=Join-Path $links ($base+'.lnk');$app=Join-Path $dest $file
 if((Test-Path $dest) -or (Test-Path $link) -or (Test-Path $hive)){throw 'Unexpected prior test state'}
 try{
   Run-Setup $setup
   if((Get-FileHash $app).Hash -cne (Get-FileHash (Join-Path $in $file)).Hash -or (New-Object -ComObject WScript.Shell).CreateShortcut($link).TargetPath -ne $app){throw 'Missing app or Start menu link'}
   $null=New-Item $hive -Force
   $pair='CZ5|nas.example.invalid|192.168.7.24|TEST-NAS'
   $null=New-ItemProperty -LiteralPath $hive -Name Pair -Value $pair
   Run-Setup $setup
   if((Get-FileHash $app).Hash -cne (Get-FileHash (Join-Path $in $file)).Hash -or (Get-ItemPropertyValue $hive Pair) -cne $pair){throw 'Reinstall lost executable or settings'}
   Run-Setup (Join-Path $dest 'unins000.exe')
   if((Test-Path $app) -or (Test-Path $link) -or (Get-ItemPropertyValue $hive Pair) -cne $pair){throw 'Uninstall lost settings or left app'}
   "PASS $role: install, signed-payload-ready byte identity, Start menu, reinstall retaining settings, uninstall retaining settings"
 } finally {if(Test-Path $hive){Remove-Item -LiteralPath $hive -Recurse}}
}
