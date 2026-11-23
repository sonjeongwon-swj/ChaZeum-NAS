# Windows-only final gate. Never issues a signature, installs a certificate, or contacts Google Drive.
[CmdletBinding()] param(
    [Parameter(Mandatory)][string]$Unsigned,
    [Parameter(Mandatory)][string]$Signed,
    [Parameter(Mandatory)][ValidatePattern('^[0-9a-fA-F]{64}$')][string]$ExpectedCertSha256
)
$ErrorActionPreference = 'Stop'
& python "$PSScriptRoot/verify_payload.py" $Unsigned $Signed
if ($LASTEXITCODE -ne 0) { throw 'Payload verification failed.' }
$bin = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits/10/bin'
$tools = @(Get-ChildItem -LiteralPath $bin -Directory | Where-Object { $_.Name -match '^\d+\.\d+\.\d+\.\d+$' } | Sort-Object { [version]$_.Name } -Descending | ForEach-Object { Join-Path $_.FullName 'x64/signtool.exe' } | Where-Object { Test-Path -LiteralPath $_ })
if (!$tools.Count) { throw 'Microsoft Windows SDK SignTool missing.' }
$tool=$tools[0];$tsig=Get-AuthenticodeSignature -LiteralPath $tool
if ($tsig.Status -ne 'Valid' -or $tsig.SignerCertificate.Subject -notmatch '(^|,\s*)O=Microsoft Corporation(,|$)') {throw 'SDK tool trust check failed.'}
$hash=[System.Security.Cryptography.SHA256]::Create()
try {
 foreach ($name in @('ChaZeumServer.exe','ChaZeumClient.exe')) {
     $p=Join-Path $Signed $name; $s=Get-AuthenticodeSignature -LiteralPath $p
     if ($s.Status -ne 'Valid' -or $null -eq $s.TimeStamperCertificate -or $null -eq $s.SignerCertificate) {throw "$name : no valid timestamped publisher signature"}
     $cert=([BitConverter]::ToString($hash.ComputeHash($s.SignerCertificate.RawData))).Replace('-','')
     if ($cert -ne $ExpectedCertSha256) {throw "$name : wrong signing certificate"}
     & $tool verify /pa /all /v /tw $p
     if ($LASTEXITCODE -ne 0) { throw "$name : SDK verification did not pass cleanly" }
     Write-Host ('Verified {0}  SHA256={1}' -f $name,(Get-FileHash -Algorithm SHA256 -LiteralPath $p).Hash)
 }
} finally { $hash.Dispose() }
