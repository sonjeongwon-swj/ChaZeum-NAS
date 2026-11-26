#ifndef Role
#error Role must be SERVER or USER
#endif
#if Role == "SERVER"
#define AppRole "서버용"
#define AppDir "{autopf}\ChazeumNAS\Server"
#else
#if Role != "USER"
#error Incorrect role
#endif
#define AppRole "유저용"
#define AppDir "{localappdata}\Programs\ChazeumNAS\User"
#endif
#define AppFile "Chazeum-NAS_" + Role
[Setup]
AppId=ChazeumNAS5-{#Role}
AppName={#AppFile}
AppVersion={#Ver}
AppVerName=차즘 NAS {#AppRole} {#Ver}
AppPublisher=ChaZeum NAS
DefaultDirName={#AppDir}
DefaultGroupName=차즘 NAS
UsePreviousAppDir=no
DisableProgramGroupPage=yes
DisableDirPage=yes
DisableWelcomePage=no
#if Role == "SERVER"
PrivilegesRequired=admin
#else
PrivilegesRequired=lowest
#endif
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir={#OutPath}
OutputBaseFilename={#AppFile}
SetupIconFile=..\brand\{#Role}.ico
UninstallDisplayIcon={app}\{#AppFile}.exe
VersionInfoVersion={#Ver}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
[Languages]
Name: "korean"; MessagesFile: "compiler:Languages\Korean.isl"
[Messages]
WelcomeLabel2=이 PC에 차즘 NAS {#AppRole}을 설치합니다. 시작 메뉴에서 다시 열 수 있습니다.%n%n쓰던 디스크와 파일은 그대로 두며, 설치만으로 공유기나 방화벽은 바뀌지 않습니다.
[Files]
Source: "{#BinPath}\{#AppFile}.exe"; DestDir: "{app}"; Flags: ignoreversion
[Icons]
Name: "{autoprograms}\{#AppFile}"; Filename: "{app}\{#AppFile}.exe"
[Run]
Filename: "{app}\{#AppFile}.exe"; Description: "차즘 NAS {#AppRole} 열기"; Flags: postinstall nowait skipifsilent
[Code]
function PrepareToInstall(var NeedsRestart: Boolean): String;
var ms,ls: Cardinal;
begin
  Result := '';
  if GetVersionNumbers(ExpandConstant('{app}\{#AppFile}.exe'),ms,ls) then
    if (ms > {#VerMS}) or ((ms = {#VerMS}) and (ls > {#VerLS})) then Result := '더 최신 버전이 설치되어 있습니다.';
end;
