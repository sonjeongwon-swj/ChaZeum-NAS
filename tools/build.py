"""Build clean x64 PE files. Only compiler backend helper is shared with older projects."""
import os,sys,re,struct,subprocess,tempfile,shutil,hashlib
from pathlib import Path
from coff import emit
R=Path(__file__).resolve().parents[1]
apis={
'kernel32':'CompareStringOrdinal GetModuleHandleW GetProcessHeap HeapAlloc HeapFree CreateThread CloseHandle ReadFile WriteFile CreatePipe SetHandleInformation GetModuleFileNameW GetSystemDirectoryW GetLastError WaitForSingleObject GetLogicalDrives GetFileAttributesW CreateProcessW GetExitCodeProcess GetComputerNameW DeleteFileW FlushFileBuffers GetFileSizeEx CreateFileW ExitProcess Sleep GetTickCount64 VirtualAlloc VirtualFree GetCommandLineW LocalFree GlobalAlloc GlobalLock GlobalFree GlobalUnlock CreateMutexW ReleaseMutex GetTempPathW GetFileInformationByHandle GetVolumeInformationW GetFinalPathNameByHandleW GetWindowsDirectoryW GetDriveTypeW QueryDosDeviceW GetVolumeNameForVolumeMountPointW CreateDirectoryW CopyFileW',
'advapi32':'LogonUserW ImpersonateLoggedOnUser RevertToSelf RegOpenKeyExW RegCreateKeyExW RegCloseKey RegGetValueW RegSetValueExW RegDeleteValueW LookupAccountNameW LsaOpenPolicy LsaAddAccountRights LsaNtStatusToWinError LsaClose CredWriteW CredReadW CredDeleteW CredFree EqualSid ConvertSidToStringSidW ConvertStringSecurityDescriptorToSecurityDescriptorW GetSecurityInfo SetSecurityInfo SetEntriesInAclW',
'user32':'LoadCursorW CreateWindowExW RegisterClassExW DestroyWindow GetMessageW TranslateMessage ShowWindow UpdateWindow IsDialogMessageW SetWindowTextW GetWindowTextW GetWindowTextLengthW MessageBoxW PostMessageW EnableWindow SetFocus DefWindowProcW DispatchMessageW SendMessageW PostQuitMessage SetClipboardData OpenClipboard EmptyClipboard CloseClipboard',
'gdi32':'CreateFontW GetStockObject','shell32':'CommandLineToArgvW SHBrowseForFolderW SHGetPathFromIDListW ShellExecuteW SHGetFolderPathW','ole32':'CoInitializeEx CoCreateGuid CoTaskMemFree','wintrust':'WinVerifyTrust',
'netapi32':'NetUserAdd NetUserDel NetUserSetInfo NetUserGetInfo NetUserEnum NetShareEnum NetApiBufferFree NetLocalGroupAdd NetLocalGroupDel NetLocalGroupGetInfo NetLocalGroupAddMembers NetLocalGroupDelMembers NetLocalGroupGetMembers NetShareAdd NetShareGetInfo NetShareDel NetShareSetInfo NetSessionEnum NetSessionDel',
'mpr':'WNetAddConnection2W WNetCancelConnection2W WNetGetConnectionW WNetGetUserW','rasapi32':'RasSetCredentialsW RasGetCredentialsW RasDialW RasHangUpW RasGetEntryPropertiesW RasSetEntryPropertiesW RasValidateEntryNameW RasEnumDevicesW RasEnumConnectionsW RasDeleteEntryW','bcrypt':'BCryptGenRandom','msvcrt':'memset memcpy memcmp _wcsicmp _snwprintf'}
def pad(b):return b+b'\0'*(-len(b)%4)
def u(s):return (s+'\0').encode('utf-16le')
def block(k,v=b'',ty=1,ch=()):
 head=pad(b'\0'*6+u(k)); b=head+v
 for child in ch:b=pad(b)+child
 return struct.pack('<HHH',len(b),len(v)//(2 if ty else 1),ty)+b[6:]
def version(role,exe):
 fixed=struct.pack('<13I',0xfeef04bd,0x10000,5<<16,3,5<<16,3,0x3f,0,0x40004,1,0,0,0)
 table=block('041204b0',ch=[block(a,u(b)) for a,b in {'CompanyName':'ChaZeum NAS','FileDescription':'차즘 NAS 5.0 '+role,'FileVersion':'5.0.0.3','OriginalFilename':exe,'ProductName':'차즘 NAS','ProductVersion':'5.0.0.3'}.items()])
 return block('VS_VERSION_INFO',fixed,0,[block('StringFileInfo',ch=[table]),block('VarFileInfo',ch=[block('Translation',struct.pack('<HH',0x412,1200),0)])])
def resource(d,server):
 name='Server' if server else 'Client';(d/'ver').write_bytes(version('서버용' if server else '유저용','ChaZeum'+name+'.exe'))
 level="requireAdministrator" if server else "asInvoker";(d/'manifest').write_text(f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?><assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0"><assemblyIdentity type="win32" name="ChaZeum.NAS.{name}" version="5.0.0.3" processorArchitecture="amd64"/><trustInfo xmlns="urn:schemas-microsoft-com:asm.v3"><security><requestedPrivileges><requestedExecutionLevel level="{level}" uiAccess="false"/></requestedPrivileges></security></trustInfo></assembly>""")
 def directory(rows):return '.long 0,0\n.short 0,0,0,'+str(len(rows))+'\n'+'\n'.join('.long '+str(k)+','+('0x80000000+' if branch else '')+label+'-root' for k,label,branch in rows)+'\n'
 s='.section .rsrc,"dr"\nroot:\n'+directory([(16,'v',1),(24,'m',1)])
 for label,path in [('v','ver'),('m','manifest')]:
  s+=label+':\n'+directory([(1,label+'name',1)])+label+'name:\n'+directory([(0x412 if label=='v' else 0,label+'entry',0)])+label+'entry:\n.rva '+label+'bytes\n.long '+str((d/path).stat().st_size)+',0,0\n'
 for label,path in [('v','ver'),('m','manifest')]:s+='.balign 4\n'+label+'bytes:\n.incbin "'+str(d/path)+'"\n'
 emit(s,d/'resource.obj')
def run(a,wd):subprocess.run([str(x) for x in a],cwd=wd,check=True)
def main():
 (R/'out').mkdir(exist_ok=True)
 with tempfile.TemporaryDirectory() as t:
  d=Path(t)
  for lib,words in apis.items():
   names=words.split();emit('.text\n'+'\n'.join('.globl '+x+'\n'+x+':\nret' for x in names),d/'st.obj');(d/'st.def').write_text('LIBRARY '+lib+'.dll\nEXPORTS\n'+'\n'.join(names))
   run(['ld','-mi386pep','--dll','--no-insert-timestamp','-e','0','st.def','st.obj','--out-implib','lib'+lib+'.a','-o','stub.dll'],d)
  def cc(unit,opts=()):
   run(['gcc','-std=c11','-S','-Os','-mabi=ms','-fshort-wchar','-fno-asynchronous-unwind-tables','-fno-unwind-tables','-fno-stack-protector','-fno-ident','-ffreestanding','-fno-builtin','-fno-strict-aliasing','-mno-red-zone','-fno-jump-tables','-Werror=implicit-function-declaration','-Werror=incompatible-pointer-types',*opts,R/'src'/(unit+'.c'),'-o','unit.s'],d)
   emit((d/'unit.s').read_text(),d/(unit+'.obj'))
  for c in ['model','os','admin','tunnel','map','meter','guide']:cc(c)
  emit('.section .rdata,"dr"\n.globl CZGuide,CZGuideEnd\nCZGuide:\n.incbin "'+str(R/'docs/Guide.pdf')+'"\nCZGuideEnd:\n',d/'pdf.obj')
  for srv in [1,0]:
   cc('ui',['-DSERVER='+str(srv)]);resource(d,srv);role=['Client','Server'][srv];obj=['model','os','guide','pdf','resource','ui']+(['admin'] if srv else ['tunnel','map','meter'])
   out=R/'out'/('ChaZeum'+role+'.exe');run(['ld','-mi386pep','--no-insert-timestamp','--dynamicbase','--high-entropy-va','--nxcompat','--subsystem','windows:6.2','--stack','0x800000,0x100000','-s','-e','start',*(f'{x}.obj' for x in obj),'-L.',*('-l'+x for x in apis),'-o',out],d)
   print(role,out.stat().st_size,hashlib.sha256(out.read_bytes()).hexdigest())
if __name__=='__main__':main()
