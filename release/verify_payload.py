"""Verify source-built PE payloads; a structural check never proves a trusted signature."""
import argparse
import hashlib
import json
import stat
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ROLES = {'Server':'ChaZeumServer.exe','Client':'ChaZeumClient.exe'}

def check(ok,msg):
    if not ok: raise ValueError(msg)

def read_pair(folder):
    entries = list(folder.iterdir())
    check({e.name for e in entries} == set(ROLES.values()), 'expected exactly the two product files')
    for e in entries:
        check(stat.S_ISREG(e.lstat().st_mode) and not e.is_symlink(), 'unexpected file type')
    return {k:(folder / n).read_bytes() for k,n in ROLES.items()}

def fields(b):
    check(len(b)>=88 and b[:2]==b'MZ','invalid DOS header')
    pe = struct.unpack_from('<I',b,60)[0]; opt = pe+24
    check(opt+152 <=len(b) and b[pe:pe+6]==b'PE\0\0\x64\x86' and struct.unpack_from('<H',b,opt)[0]==0x20b,'invalid x64 PE')
    return opt+64,opt+112+4*8

def match_payload(original, delivery):
    # Restrict this product to append-only certificates and canonical zero padding.
    # Windows Authenticode verification is a separate mandatory step in verify-release.ps1.
    ch,sec=fields(original);check(fields(delivery)==(ch,sec),'changed PE headers')
    check(original[sec:sec+8]==bytes(8),'unexpected signature on build input')
    off,size=struct.unpack_from('<II',delivery,sec)
    check(size>=8 and off==(len(original)+7)//8*8 and off+size==len(delivery),'invalid certificate table or trailing bytes')
    check(not any(delivery[len(original):off]),'unexpected alignment bytes')
    body=bytearray(delivery[:len(original)]);body[ch:ch+4]=original[ch:ch+4];body[sec:sec+8]=original[sec:sec+8]
    check(body==original,'program bytes changed after build')

def run(unsigned,signed=None):
    pins=json.loads((ROOT/'release/unsigned-sha256.json').read_text(encoding='utf8')); raw=read_pair(unsigned)
    out=read_pair(signed) if signed else None
    for role,b in raw.items():
        check(hashlib.sha256(b).hexdigest()==pins[role], f'{role}: reviewed unsigned hash mismatch')
        _,sec=fields(b);check(b[sec:sec+8]==bytes(8),'unsigned input expected')
        if out: match_payload(b,out[role])
    print('Both program payloads match reviewed source. This test does not establish publisher trust.')

if __name__=='__main__':
    arg=argparse.ArgumentParser();arg.add_argument('unsigned',type=Path);arg.add_argument('signed',type=Path,nargs='?');a=arg.parse_args();run(a.unsigned,a.signed)
