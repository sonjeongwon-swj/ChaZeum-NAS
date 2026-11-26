"""Illustrate the *production* page builder: run record-only Win32 adapters and
render their control geometry, labels, and synthetic example values. NOT a
Windows screenshot. Only the explanatory frame is added in this renderer."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import subprocess,json,tempfile
R=Path(__file__).resolve().parents[1]
BASE=Path('/usr/share/fonts/opentype/noto')
pages=[('server',5,'서버용  |  내 NAS 현황','가로는 아이디, 세로는 공유입니다. 칸을 눌러 그 조합의 권한을 바꿀 수 있어요.'),
       ('user',5,'유저용  |  내 NAS 드라이브','내 노트북의 드라이브 문자를 고르고 연결합니다. 현황표는 숨길 수 있어요.'),
       ('server',1,'서버용  |  무엇을 공유할까요?','D: 전체를 고른 예입니다. 같은 화면에서 폴더 하나만 고를 수도 있습니다.'),
       ('server',2,'서버용  |  누가 읽고 쓸 수 있나요?','예시 아이디 demo에 자료창고의 읽기 권한을 선택한 상태입니다.'),
       ('user',2,'유저용  |  어느 문자로 열까요?','서버의 자료창고를 이 노트북에서 X:로 열도록 선택한 예입니다.')]
def capture():
 with tempfile.TemporaryDirectory() as td:
  data={}
  for role,server in [('server',1),('user',0)]:
   exe=Path(td)/role
   subprocess.run(['gcc','-std=c11','-ffunction-sections','-fdata-sections','-fshort-wchar','-I'+str(R/'src'),f'-DSERVER={server}',str(R/'tests/menu_layout.c'),str(R/'src/model.c'),'-Wl,--gc-sections','-o',str(exe)],check=True)
   data[role]=[json.loads(subprocess.check_output([str(exe),str(p)])) for p in range(6)]
  return data
def render(controls,role,idx,title,caption,output):
 S=2;W=850;H=802;im=Image.new('RGB',(W*S,H*S),'#f6f6fb');dr=ImageDraw.Draw(im);cache={}
 def font(n,b=False):
  if (n,b) not in cache:cache[n,b]=ImageFont.truetype(str(BASE/('NotoSansCJK-'+('Bold' if b else 'Regular')+'.ttc')),round(n*S),index=1)
  return cache[n,b]
 def xy(k):return tuple(round(v*S) for v in k)
 def rect(a,b,c,d,fill,outline=None,rad=0):
  dr.rounded_rectangle(xy((a,b,c,d)),radius=round(rad*S),fill=fill,outline=outline,width=S)
 def text(x,y,t,n=16,col='#20242e',bold=False,anchor='lm'):
  dr.text(xy((x,y)),t,font=font(n,bold),fill=col,anchor=anchor)
 def drawlines(x,y,value,width,n=16,spacing=23,maxlines=4):
  lines=[]
  for para in value.replace('\r','').split('\n'):
   row=''
   for ch in para:
    if row and dr.textlength(row+ch,font=font(n))/S>width:lines.append(row.rstrip());row=ch.lstrip()
    else:row+=ch
   if row or not para:lines.append(row)
  for j,l in enumerate(lines[:maxlines]):text(x,y+j*spacing,l,n)
 rect(0,0,W,7,'#14263D');text(28,28,'차즘 NAS 5.0.2.0     /     메뉴 설계 미리보기',14,'#14263D',True)
 text(28,68,title,28,'#22233b',True);text(29,106,caption,16,'#41475a')
 ox=29;oy=174
 rect(ox-1,oy-39,ox+789,oy+598,'#f0f0f0','#aeb2bf',6);rect(ox,oy-38,ox+788,oy-1,'#fafafa',None,5)
 text(ox+14,oy-19,('Chazeum-NAS_SERVER' if role=='server' else 'Chazeum-NAS_USER')+' 5.0.2.0',15)
 text(ox+770,oy-19,'×',18,'#555b67',False,'mm')
 for c in controls:
  kind=c['kind'];x=ox+c['x'];y=oy+c['y'];w,h=c['w'],c['h'];t=c['text'];ident=c['id'];fl=c['flags']
  if kind=='STATIC':
   if c['title']:text(x,y+17,t,25,'#202331',True)
   else:drawlines(x,y+12,t,w,15,22,max(1,h//20+1))
  elif kind=='BUTTON' and fl&15==3:
   rect(x+1,y+7,x+16,y+22,'white','#838791',2);text(x+24,y+14,t,15)
  elif kind=='BUTTON':
   rect(x,y,x+w,y+h,'#e9e9eb' if not c['enabled'] else '#fafafa','#aaaeb6',3)
   text(x+w/2,y+h/2,t,15,'#565a64' if not c['enabled'] else '#20242e',anchor='mm')
  elif kind in ('EDIT','COMBOBOX'):
   comp=kind=='COMBOBOX';hh=29 if comp else h
   rect(x,y,x+w,y+hh,'#f4f4f5' if c['ro'] or ident==501 else '#ffffff','#9499a4',2)
   if comp:dr.polygon([xy((x+w-18,y+12)),xy((x+w-8,y+12)),xy((x+w-13,y+17))],fill='#555965')
   drawlines(x+7,y+14,t,w-(30 if comp else 15),15,22,max(1,min(5,hh//20)))
 text(30,788,'배치·문구는 앱 코드에서 추출했습니다. 예시 값이며 실제 Windows 캡처가 아닙니다.',13,'#534b73')
 im.save(output,optimize=True)
def create(dest):
 dest=Path(dest);dest.mkdir(parents=True,exist_ok=True);data=capture()
 for role,idx,title,cap in pages:render(data[role][idx],role,idx,title,cap,dest/(role+'-'+str(idx)+'.png'))
 (dest/'menus.json').write_text(json.dumps(data,ensure_ascii=False),encoding='utf8')
 return data
if __name__=='__main__':
 import sys
 create(Path(sys.argv[1]) if len(sys.argv)>1 else R/'out/preview')
