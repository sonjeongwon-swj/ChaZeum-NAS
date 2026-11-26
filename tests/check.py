"""Local portable policy/integration tests and direct inspection of the two produced PEs."""
import subprocess,sys,hashlib,struct,tempfile,os,pathlib
from xml.etree import ElementTree as ET
from pypdf import PdfReader
R=pathlib.Path(__file__).resolve().parents[1]; O=R/'out'
def run(a):
 p=subprocess.run([str(x) for x in a],capture_output=True,text=True)
 if p.returncode:
  raise RuntimeError(f'{pathlib.Path(str(a[0])).name} exited {p.returncode}: {p.stderr} {p.stdout}')
 return p.stdout.strip()
mode=os.getenv('CZNAS_TEST_MODE','default')
flags={'default':['-O1'],'optimized':['-O2'],'checked':['-O0','-fsanitize=undefined','-fno-sanitize-recover=all']}[mode]
print('Test mode:',mode)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
with tempfile.TemporaryDirectory() as td:
 t=pathlib.Path(td)
 def ctest(name,units,args=()):
  exe=t/name
  opts=['gcc','-D_POSIX_C_SOURCE=200809L','-std=c11',*flags,'-fshort-wchar','-ffunction-sections','-fdata-sections','-I'+str(R/'src'),'-Wl,--gc-sections']
  run([*opts,R/'tests'/f'{name}.c',*[R/'src'/(u+'.c') for u in units],'-o',exe])
  print(run([exe,*args]))
 ctest('scenarios',['model','admin','map']);ctest('auth_flow',['map','model']);ctest('share_flow',['admin','model']);ctest('tunnel_flow',['model']);ctest('io',['meter','model'],[t/'new-file'])
 ctest('secure_flow',['model','secureroute']);ctest('abi',[]);ctest('startup_flow',['startup','model'])
 for role in [1,0]:
  exe=t/f'ui-{role}'
  run(['gcc','-std=c11',*flags,'-fshort-wchar','-ffunction-sections','-fdata-sections',f'-DSERVER={role}','-I'+str(R/'src'),'-Wl,--gc-sections',R/'tests/ui_approval.c',R/'src/model.c','-o',exe])
  print(run([exe]))
pdf=(R/'docs/Guide.pdf').read_bytes();reader=PdfReader(R/'docs/Guide.pdf');assert len(reader.pages)==9 and '아이패드' in reader.pages[4].extract_text()
assert all('드라이브' in reader.pages[i].extract_text() or '아이디' in reader.pages[i].extract_text() for i in [6,7,8])
for s in ['Server','Client']:
 role='SERVER' if s=='Server' else 'USER'; p=O/f'Chazeum-NAS_{role}.exe';b=p.read_bytes();pe=struct.unpack_from('<I',b,60)[0];assert b[:2]==b'MZ' and b[pe:pe+4]==b'PE\0\0'
 machine,sc=struct.unpack_from('<HH',b,pe+4);opt=pe+24;assert machine==0x8664 and struct.unpack_from('<H',b,opt)[0]==0x20b
 def pair(at):return struct.unpack_from('<II',b,at)
 assert pair(opt+112+4*8)==(0,0), 'dev artifact should be clearly unsigned';assert struct.unpack_from('<H',b,opt+68)[0]==2
 flags=struct.unpack_from('<H',b,opt+70)[0];assert flags&0x160==0x160
 sections=opt+struct.unpack_from('<H',b,pe+20)[0];maps=[]
 for j in range(sc):
  k=sections+j*40;vs,va,rs,raw=struct.unpack_from('<IIII',b,k+8);maps.append((va,raw,max(vs,rs)))
 def off(x):return next(raw+x-va for va,raw,size in maps if va<=x<va+size)
 root=off(pair(opt+112+2*8)[0])
 def child(a,id):
  z=root+a;n=sum(struct.unpack_from('<HH',b,z+12))
  tab=[pair(z+16+i*8) for i in range(n)];return next(v&0x7fffffff for k,v in tab if k==id)
 def resource(typ,lang,ident=1):
  pos=child(child(child(0,typ),ident),lang);r,l=pair(root+pos);return b[off(r):off(r)+l]
 m=ET.fromstring(resource(24,0));level=m.find('.//{urn:schemas-microsoft-com:asm.v3}requestedExecutionLevel');assert level.get('level')==('requireAdministrator' if s=='Server' else 'asInvoker')
 v=resource(16,0x412);idx=v.index(bytes.fromhex('bd04effe'));assert struct.unpack_from('<I',v,idx+8)[0]==0x50000 and struct.unpack_from('<I',v,idx+12)[0]==(1<<16) and '5.0.1.0'.encode('utf-16le') in v
 assert p.name.encode('utf-16le') in v
 ico=(R/'brand'/f'{role}.ico').read_bytes();group=resource(14,0,101);assert group[:6]==ico[:6]
 num=struct.unpack_from('<H',ico,4)[0];assert num==9 and len(group)==6+14*num
 for i in range(num):
  row=ico[6+16*i:22+16*i];g=group[6+i*14:20+i*14];length,offset=struct.unpack_from('<II',row,8)
  assert row[:12]==g[:12] and resource(3,0,struct.unpack_from('<H',g,12)[0])==ico[offset:offset+length]
 assert b.count(pdf)==1
 info=run(['objdump','-p',p]);assert 'WinVerifyTrust' in info and 'LoadImageW' in info
 if s=='Client':assert 'WNetAddConnection2W' in info and 'CredWriteW' in info and 'RasDialW' in info and 'NetUserAdd' not in info and 'NetShareAdd' not in info and all(api in info for api in ['LogonUserW','ImpersonateLoggedOnUser','RevertToSelf'])
 else:assert all(api in info for api in ['Shell_NotifyIconW','QueryServiceConfigW','ChangeServiceConfigW','StartServiceW']) and 'NetUserAdd' in info and 'NetShareAdd' in info and 'LsaAddAccountRights' in info and 'RasDialW' not in info and 'CredWriteW' not in info
 print(s,'x64, resource version, privilege level, unsigned state, separate role imports and embedded PDF passed;',sha(p))
print('No target Windows launch, publisher signature, phone, iPad, outside laptop or external throughput is represented by these results.')
