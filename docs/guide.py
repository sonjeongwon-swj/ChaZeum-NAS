"""Illustrated, searchable offline guide, embedded in each executable."""
from pathlib import Path
import fitz
from fontTools import ttLib,subset
from io import BytesIO
import tempfile
from menus import create as render_menus, pages as menu_pages
root=Path(__file__).resolve().parents[1]
pages=[
("01","어디에 무엇이 있나요?","집 밖에서도 내 파일을 여는 데에는 두 가지가 필요합니다.",[
("접속 기기","노트북은 탐색기, 아이폰·아이패드는 기본 파일 앱에서 엽니다.\n외부에서 접속할 때 VPN은 집까지 암호화한 통로를 만듭니다."),
("집의 TP-Link 공유기","도메인 또는 공인 IP가 가리키는 곳입니다. VPN 접속을 받아 줍니다.\n공유기 USB에 있던 예전 드라이브와 3080 PC의 폴더는 서로 다릅니다."),
("3080 PC와 실제 폴더","파일을 보관하고 아이디별로 읽기·쓰기를 허용하는 곳입니다.\n외부에서 사용할 동안 PC와 공유기가 켜져 있어야 합니다.")], "서버용은 집 PC에, 유저용은 접속할 Windows에 둡니다."),
("02","서버용  |  드라이브 전체 또는 폴더만","서버용은 ‘공유할 위치’부터 열립니다. 로컬 공유는 도메인 입력 전에도 만들 수 있습니다.",[
("1   서버 주소","‘내 PC 주소 보기’에서 집 네트워크에 연결된 3080 PC의 IPv4를 확인합니다.\n공유기 도메인과 그 IPv4를 입력하고 ‘주소 저장 → 접속 정보 복사’를 누릅니다."),
("2   공유할 위치","‘드라이브 전체’ 목록에서 하나를 고르거나, ‘폴더 찾아보기’로 폴더만 고릅니다.\n예: D:\\ 전체 → ‘자료창고’. 공유 이름 입력 → ‘공유 추가’ → 실제 위치 확인."),
("3   아이디·권한","NAS 아이디와 새 암호로 ‘이 아이디 새로 만들기’를 한 번 누릅니다.\n같은 화면에 공유 이름과 읽기 / 읽기·쓰기·삭제 권한을 정해 ‘권한 저장’. 공유마다 다르게 줍니다.")], "전체 공유 대상: 로컬 고정 NTFS 데이터 드라이브. Windows 시스템 드라이브 전체는 제외합니다."),
("03","공유기  |  외부 통로 한 번 준비","Archer BE230 기준입니다. 메뉴 이름은 펌웨어·언어에 따라 다를 수 있습니다.",[
("고급  ›  네트워크  ›  동적 DNS","집 인터넷 주소가 바뀌면 VPN도 찾지 못합니다. 이미 쓰는 DDNS가 있으면 그대로\n사용하세요. 3080 PC에는 주소 예약으로 같은 집 안 IP가 유지되게 합니다."),
("고급  ›  VPN 서버  ›  L2TP/IPSec","Windows 기본 기능을 쓰려면 이 서버를 켜고 IPsec 암호화를 사용합니다.\n긴 공유 키와 VPN 계정·암호를 직접 정해 저장합니다. PPTP는 사용하지 않습니다."),
("공유기와 Windows 방화벽","서버용 앱은 공유기 설정을 바꾸지 않습니다. TCP 445나 FTP를 포트포워딩하지 마세요.\n필요할 때만 서버용 ‘연결 확인’에서 개인망·실제 사설 대역만 허용합니다.")], "이중 공유기나 통신사 공유 주소면 VPN이 외부에서 도착하지 않을 수 있습니다."),
("04","Windows 유저용  |  드라이브 연결","서버에서 복사한 접속 정보에는 비밀번호가 들어 있지 않습니다.",[
("1   서버 주소","복사한 정보를 붙여넣고 ‘입력칸에 채우기 → 서버 주소 저장’을 누릅니다.\n또는 공유기 도메인, 3080 PC의 집 안 IP, PC 이름을 직접 입력합니다."),
("2   외부 VPN","공유기 VPN 아이디·암호와 IPsec 공유 키로 ‘이 PC에 VPN 등록’을 누릅니다.\nWindows 기본 VPN을 사용합니다. Microsoft 계정이나 다른 Windows 앱은 필요 없습니다."),
("3   드라이브","NAS 아이디·암호를 입력합니다. ‘공유 목록 보기’에서 보고 공유 이름을 씁니다.\n서버 D:도 여기서 빈 문자 X:로 고를 수 있습니다. ‘드라이브 연결·저장’ → 탐색기 ‘내 PC’.")], "원하면 ‘확인·자동 연결 → 로그인 때 자동 연결’을 켭니다. 드라이브마다 연결을 반복하세요."),
("05","아이폰·아이패드  |  기본 파일 앱","VPN은 집에 가는 통로, 파일 앱은 폴더를 열고 저장하는 도구입니다.",[
("1   기기 설정  ›  VPN","설정 검색창에 ‘VPN’을 입력 → VPN 구성 추가 → L2TP. 서버=공유기 도메인,\n계정·암호=공유기 VPN 계정, 비밀=IPsec 공유 키. 저장하고 VPN을 연결합니다."),
("2   파일  ›  둘러보기  ›  …  ›  서버에 연결","주소는 smb:// 뒤에 3080 PC의 집 안 IPv4를 붙입니다. 공유기 주소가 아닙니다.\n‘등록 사용자’로 서버용에서 만든 NAS 아이디와 암호를 입력합니다."),
("3   쓰기 시험","쓰기 권한이 있는 폴더에 ‘나의 iPhone/iPad’의 작은 파일을 복사합니다.\n닫았다가 다시 열어 확인하세요. 다음에는 ‘최근 서버’ 또는 ‘공유됨’에서 엽니다.")], "이미 WireGuard를 쓴다면 기존에 노출된 QR의 피어는 삭제하고 기기별 새 피어를 등록하세요."),
("06","완료 확인  |  집 밖에서 실제 시험","집 안에서 열렸다는 사실만으로 외부 접속 성공이라고 할 수 없습니다.",[
("아이폰은 Wi-Fi 끄기","셀룰러 표시를 확인한 다음 VPN → 파일 앱. 새 파일을 저장하고 다시 엽니다.\n아이패드는 집 밖 Wi-Fi나 집 Wi-Fi를 끈 폰의 핫스팟으로 같은 시험을 합니다."),
("노트북은 외부망에서 탐색기","집 밖 Wi-Fi로 연결하고 유저용의 저장된 드라이브를 엽니다.\n‘확인·자동 연결’에서 문자를 고르고 ‘256 MiB 시험’을 누릅니다."),
("기록할 것","새 파일의 저장·재열기·내용 일치, 쓰기·읽기 MB/s, 접속한 망을 기록합니다.\n직접 VPN 경로는 앱이 확인합니다. 인터넷 회선 내부의 중간 홉까지 뜻하지 않습니다.")], "외부망 접속은 아직 미검증입니다. Windows에서 실행을 차단하면 중단하세요.")
]
srcs=[("TP-Link · Archer BE230/BE3600 사용 설명서","https://static.tp-link.com/upload/manual/2025/202508/20250815/1910013637_Archer%20BE230%26BE3600_UG_REV1.0.0.pdf"),("Apple · 파일 앱 서버 연결","https://support.apple.com/ko-kr/guide/iphone/iphe9aff429a/ios"),("Apple · VPN 지원 방식","https://support.apple.com/guide/security/secure-network-access-sec802e8ab55/web")]
def face(path,chars):
 f=ttLib.TTFont(path,fontNumber=1,recalcTimestamp=False);opt=subset.Options(retain_gids=True);sub=subset.Subsetter(options=opt);sub.populate(text=chars);sub.subset(f);b=BytesIO();f.save(b);return b.getvalue()
alts=['7. 서버: 데이터 드라이브 전체 또는 폴더를 골라 공유 이름을 입력합니다.', '8. 서버: NAS 아이디별로 접근 안 됨, 읽기, 읽기·쓰기·삭제를 선택합니다.', '9. 유저: 공유 이름과 이 PC에서 쓸 빈 드라이브 문자를 선택합니다.']
alltext=str(pages)+str(srcs)+str(alts)+str(menu_pages)+'메뉴 설계 미리보기. 예시 값이며 Windows 실기기 캡처가 아닙니다. 드라이브를 교체하거나 문자를 바꾸기 전에는 공유를 먼저 해제하세요.'+'차즘 NAS 5.0.0.3  |  시작 안내    설정 흐름도 • 1234567890 /  서버용 유저용 공식 도움말'
fs=[face('/usr/share/fonts/opentype/noto/NotoSansCJK-'+v+'.ttc',alltext) for v in ['Regular','Bold']]
def glyph(page,x,y,key,col):
 R=lambda a,b,c,d: page.draw_rect((x+a,y+b,x+c,y+d),color=col,width=1.6)
 L=lambda a,b,c,d: page.draw_line((x+a,y+b),(x+c,y+d),color=col,width=1.7)
 C=lambda a,b,r: page.draw_circle((x+a,y+b),r,color=col,width=1.5)
 if key=='phone':
  R(6,1,24,30);L(9,6,21,6);C(15,26,1)
 elif key=='router':
  R(0,16,31,27);L(4,16,2,4);L(27,16,28,4);C(7,22,1);L(12,22,17,22)
 elif key=='pc':
  R(1,2,22,24);L(4,8,18,8);L(4,12,15,12);C(15,19,1.5);L(4,28,20,28)
 elif key=='folder':
  L(1,24,1,5);L(1,5,11,5);L(11,5,15,10);L(15,10,30,10);L(30,10,30,25);L(30,25,1,25);L(3,14,26,14)
 elif key=='person':
  C(15,8,5);page.draw_bezier((x+4,y+26),(x+4,y+12),(x+26,y+12),(x+26,y+26),color=col,width=1.6);L(4,26,26,26)
 elif key=='lock':
  R(4,13,27,28);page.draw_bezier((x+8,y+13),(x+8,y-2),(x+22,y-2),(x+22,y+13),color=col,width=1.7);C(15,20,1.8)
 elif key=='drive':
  R(1,8,30,25);L(4,17,27,17);C(23,21,1);L(5,21,9,21)
 elif key=='network':
  R(10,1,20,8);L(15,8,15,16);L(4,16,27,16);L(4,16,4,21);L(27,16,27,21);R(0,21,9,27);R(22,21,31,27)
 elif key=='check':
  C(15,15,13);L(7,15,12,20);L(12,20,23,9)
 elif key=='key':
  C(9,10,6);L(14,14,27,27);L(19,19,22,16);L(23,23,26,20)
 else:
  R(2,3,29,22);L(15,22,15,27);L(8,27,23,27);L(5,9,26,9)
icons=[['phone','router','pc'],['network','folder','person'],['network','lock','router'],['pc','lock','drive'],['key','folder','phone'],['phone','drive','check']]
doc=fitz.open();ink=(.075,.14,.22);mut=(.24,.32,.40);teal=(.04,.36,.36)
for no,t,sub,cards,bottom in pages:
 p=doc.new_page(width=640,height=670)
 for n,b in zip(('CZReg','CZBold'),fs):p.insert_font(fontname=n,fontbuffer=b)
 def txt(x,y,s,size=13,b=False,color=ink):
  for k,line in enumerate(s.split('\n')):p.insert_text((x,y+k*(size+8)),line,fontname='CZBold' if b else 'CZReg',fontsize=size,color=color)
 p.draw_rect((0,0,640,13),color=teal,fill=teal)
 txt(32,51,'차즘 NAS 5.0.0.3  /  시작 안내',13,True,teal);txt(565,52,no+' / 09',12,False,mut)
 txt(32,108,t,26,True);txt(33,144,sub,13,False,mut)
 for i,(title,body) in enumerate(cards):
  top=175+i*121
  p.draw_rect((32,top,608,top+104),color=(.80,.87,.89),fill=(.965,.98,.98))
  p.draw_rect((32,top,37,top+104),color=teal,fill=teal)
  glyph(p,52,top+18,icons[int(no)-1][i],teal);txt(96,top+34,title,16,True);txt(52,top+65,body,12)
 txt(33,563,bottom,12,True,teal)
 if no in ('03','05'):
  for j,(s,url) in enumerate(srcs[:1] if no=='03' else srcs[1:]):
   txt(33,601+20*j,s,10,False,mut);p.insert_link({'kind':fitz.LINK_URI,'from':fitz.Rect(31,587+20*j,321,606+20*j),'uri':url})
 p.draw_line((32,634),(608,634),color=(.84,.88,.90));txt(32,653,'차즘 NAS 5.0.0.3  |  서버용 · 유저용',10,False,mut)
with tempfile.TemporaryDirectory() as td:
 data=render_menus(td)
 for j,(role,idx,title,caption) in enumerate(menu_pages):
  p=doc.new_page(width=640,height=670);p.insert_font(fontname='CZReg',fontbuffer=fs[0]);p.insert_font(fontname='CZBold',fontbuffer=fs[1])
  p.insert_image(fitz.Rect(13,15,627,596),filename=str(Path(td)/(role+'-'+str(idx)+'.png')))
  txt(32,615,alts[j],12,True)
  txt(32,636,'드라이브를 교체하거나 문자를 바꾸기 전에는 공유를 먼저 해제하세요.' if j==0 else '메뉴 설계 미리보기. 예시 값이며 Windows 실기기 캡처가 아닙니다.',10,False,mut)
  txt(565,652,str(7+j)+' / 09',10,False,mut)
doc.set_metadata({'title':'차즘 NAS 5.0.0.3 시작 안내','author':'차즘 NAS','subject':'집 PC의 파일을 집 밖에서 여는 방법'});doc.save(root/'docs'/'Guide.pdf',garbage=4,deflate=True,no_new_id=True)
