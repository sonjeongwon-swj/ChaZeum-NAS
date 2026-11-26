#ifndef CzRole
#error CzRole must be SERVER or USER
#endif
#if CzRole == "SERVER"
#define CzCaption "서버용"
#define CzInstallPath "{autopf}\ChazeumNAS\Server"
#else
#if CzRole != "USER"
#error Incorrect role
#endif
#define CzCaption "유저용"
#define CzInstallPath "{localappdata}\Programs\ChazeumNAS\User"
#endif
#define CzFile "Chazeum-NAS_" + CzRole
[Setup]
AppId=ChazeumNAS5-{#CzRole}
AppName={#CzFile}
AppVersion={#CzVersionText}
AppVerName=차즘 NAS {#CzCaption} {#CzVersionText}
AppPublisher=ChaZeum NAS
DefaultDirName={#CzInstallPath}
DefaultGroupName=차즘 NAS
UsePreviousAppDir=no
DisableProgramGroupPage=yes
DisableDirPage=yes
DisableWelcomePage=no
#if CzRole == "SERVER"
PrivilegesRequired=admin
#else
PrivilegesRequired=lowest
#endif
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir={#CzOutput}
OutputBaseFilename={#CzFile}
SetupIconFile=..\brand\{#CzRole}.ico
UninstallDisplayIcon={app}\{#CzFile}.exe
VersionInfoVersion={#CzVersionText}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
[Languages]
Name: "korean"; MessagesFile: "compiler:Languages\Korean.isl"
[Messages]
WelcomeLabel2=이 PC에 차즘 NAS {#CzCaption}을 설치합니다. 시작 메뉴에서 다시 열 수 있습니다.%n%n쓰던 디스크와 파일은 그대로 두며, 설치만으로 공유기나 방화벽은 바뀌지 않습니다.
[Files]
Source: "{#CzBin}\{#CzFile}.exe"; DestDir: "{app}"; Flags: ignoreversion
[Icons]
Name: "{autoprograms}\{#CzFile}"; Filename: "{app}\{#CzFile}.exe"
[Run]
Filename: "{app}\{#CzFile}.exe"; Description: "차즘 NAS {#CzCaption} 열기"; Flags: postinstall nowait skipifsilent
[Code]
function PrepareToInstall(var NeedsRestart: Boolean): String;
var ms,ls: Cardinal;
begin
  Result := '';
  if GetVersionNumbers(ExpandConstant('{app}\{#CzFile}.exe'),ms,ls) then
    if (ms > {#CzMajorMinor}) or ((ms = {#CzMajorMinor}) and (ls > {#CzBuildRev})) then Result := '더 최신 버전이 설치되어 있습니다.';
end;
