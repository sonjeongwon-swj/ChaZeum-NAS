"""Assemble GCC's Windows x64 ABI output with LLVM's COFF assembler (no network dependencies)."""
import ctypes as C, re, sys, pathlib
def emit(assembly, dest):
 lib=C.CDLL('libLLVM-19.so')
 def fn(name,ret,*args):
  f=getattr(lib,name); f.restype=ret; f.argtypes=list(args); return f
 P=C.c_void_p; S=C.c_char_p; I=C.c_int; E=C.POINTER(S)
 for n in ['TargetInfo','Target','TargetMC','AsmPrinter','AsmParser']:fn('LLVMInitializeX86'+n,None)()
 lines=[]; commons=[]
 for line in assembly.splitlines():
  s=line.strip()
  if s.startswith('.local'):continue
  common=re.match(r'\.comm\s+([\w.$]+),(\d+)(?:,(\d+))?',s)
  if common:
   name,num,align=common.groups();commons.extend(['.bss','.balign '+(align or '8'),name+':','.zero '+num]);continue
  if any(s.startswith(x) for x in ('.file ','.file\t','.type','.size','.ident','.section\t.note.GNU-stack','.section .note.GNU-stack','.hidden')):continue
  if '.section' in s and '.rodata' in s:line='.section .rdata,"dr"'
  if '.section' in s and '.data.rel.ro' in s:line='.section .rdata,"dr"'
  line=line.replace('@PLT','')
  lines.append(line)
 asm='\n'.join(lines+commons)+'\n'
 triple=b'x86_64-pc-windows-msvc'
 ir=('target triple = "'+triple.decode()+'"\nmodule asm "'+''.join('\\%02X'%x for x in asm.encode())+'"\n').encode()
 ctx=fn('LLVMContextCreate',P)()
 buf=fn('LLVMCreateMemoryBufferWithMemoryRangeCopy',P,S,C.c_size_t,S)(ir,len(ir),b'module')
 mod=P();msg=S()
 def ok(err):
  if err:raise RuntimeError(msg.value)
 ok(fn('LLVMParseIRInContext',I,P,P,C.POINTER(P),E)(ctx,buf,C.byref(mod),C.byref(msg)))
 tar=P();ok(fn('LLVMGetTargetFromTriple',I,S,C.POINTER(P),E)(triple,C.byref(tar),C.byref(msg)))
 mach=fn('LLVMCreateTargetMachine',P,P,S,S,S,I,I,I)(tar,triple,b'generic',b'',2,0,0)
 ok(fn('LLVMTargetMachineEmitToFile',I,P,P,S,I,E)(mach,mod,str(dest).encode(),1,C.byref(msg)))
if __name__=='__main__':emit(pathlib.Path(sys.argv[1]).read_text(),sys.argv[2])
