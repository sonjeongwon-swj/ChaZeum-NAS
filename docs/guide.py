"""Build a compact, accessible Korean vector guide. Illustrations mirror the UI, not real accounts."""
from pathlib import Path
from io import BytesIO
import fitz
from fontTools import ttLib,subset
P=Path(__file__).parent
pages=[
("서버에 쓰던 디스크 연결하기","3080 PC · 서버용","D: 드라이브 전체", "또는 H:의 폴더 하나", "공유 이름 직접 지정",[
("1   서버 설치","3080 PC에서 Chazeum-NAS_SERVER 설치 파일을 엽니다. 다음부터는 시작 메뉴에서 엽니다. Windows에서 게시자를 확인할 수 없어 차단하면 진행을 멈추세요."),
("2   공유 추가","서버 상태 → + 드라이브·폴더. 드라이브 전체를 선택하거나 폴더 찾아보기를 누릅니다. 공유 이름은 직접 지어 주세요. 공유 추가 전에 표시된 경로를 확인하세요."),
("3   두 개 이상도 가능","같은 방법으로 다른 드라이브나 폴더를 추가합니다. 로컬 고정 NTFS 데이터 드라이브 전체를 지원합니다. Windows 디스크 전체와 이미 등록한 위치에 겹치는 공유는 제외합니다.")]),
("아이디와 권한을 한 화면에서","공유 권한표","공유 이름 / 위치","아이디 A","아이디 B",[
("아이디 만들기","서버 상태 → + 아이디·권한에서 NAS 아이디와 암호를 정합니다. Microsoft 계정은 필요 없습니다. 암호를 다른 사람이나 채팅에 보내지 마세요."),
("권한 주기","서버 상태에서 왼쪽은 공유, 위쪽은 아이디입니다. 만나는 칸을 눌러 접근 안 됨, 읽기, 읽기·쓰기·삭제 중 하나를 고른 뒤 권한 저장을 누릅니다."),
("창을 닫으면","이미 등록한 파일은 Windows가 계속 공유합니다. PC를 끄거나 절전 상태로 두면 접속되지 않습니다. 부팅 때 공유 시작을 누르면 Windows 서비스를 자동 시작하도록 설정합니다.")]),
("노트북에서 탐색기로 열기","Windows · 유저용","서버 등록 (처음 한 번)","아이디 · 암호","내 드라이브 문자",[
("1   서버 연결 정보","서버 상태 → 연결 정보 복사. 노트북에서 Chazeum-NAS_USER를 설치한 뒤 그 정보를 붙여넣고 이 서버 등록을 누릅니다. 이때 집으로 연결되는 기존 WireGuard 또는 Tailscale이 켜져 있어야 합니다."),
("2   내 PC에서 쓸 문자 선택","NAS 아이디, 암호, 빈 드라이브 문자(예: X:)를 고르고 드라이브 연결을 누릅니다. 접근할 공유가 한 개면 자동 연결됩니다. 여러 개면 목록에서 고른 뒤 다시 누릅니다."),
("3   매번 다시 입력하지 않기","탐색기 → 내 PC → 선택한 드라이브를 여세요. 로그인 때 자동 연결을 켤 수 있습니다. 보안망이 준비되어야 재연결됩니다. 현황 숨기기는 연결을 끊지 않습니다.")]),
("아이폰·아이패드는 기본 파일 앱","Apple · 외부 연결","집으로 보안 연결","파일 → 둘러보기 → …","서버에 연결",[
("집 밖에서 먼저 할 일","집 안에서는 집 Wi-Fi에 연결합니다. 밖에서는 그 기기에 등록한 집 보안 연결을 먼저 켜세요. WireGuard를 쓰면 기기마다 자기 프로필을 등록합니다. 파일 관리 전용 앱은 필요 없습니다."),
("파일 열고 저장하기","파일 → 둘러보기 → 오른쪽 위 … → 서버에 연결. 서버용 연결 정보에 표시된 smb:// 주소를 넣고 등록 사용자를 선택해 NAS 아이디와 암호로 로그인합니다. 다음부터는 최근 서버를 누르세요."),
("와이파이를 끄고 확인","아이폰은 Wi-Fi를 끄고, 아이패드는 집 밖 네트워크에서 쓰기 권한이 있는 폴더에 작은 파일을 저장하세요. 앱을 닫았다가 다시 열어 내용이 같은지 확인합니다.")]),
("설치와 외부 접속은 따로 확인","상태와 업데이트","파일 공유 서비스","집 밖에서 저장·재열기","실제 복사 속도",[
("집에서 쓰던 파일","공유를 추가하는 것은 포맷이 아닙니다. 기존 파일을 그대로 둔 채 권한을 설정합니다. 한 디스크의 고장에 대비해 중요한 원본은 다른 디스크에도 보관하세요."),
("속도 확인","노트북을 집 밖 네트워크에 연결합니다. 확인·자동 연결 → 시험할 드라이브 → 256 MiB 시험. 쓰기·읽기 MB/s와 표시된 보안 경로를 기록하세요. 탐색기에 문자가 뜬 것만으로 외부 시험이 끝난 것은 아닙니다."),
("새 버전","같은 역할의 새 설치 파일을 실행하면 같은 자리에 설치됩니다. 앱을 제거해도 선택한 데이터 파일은 지우지 않습니다. 인터넷에 파일 공유 포트를 직접 열 필요는 없습니다.")])]
common='차즘 NAS 5.0.2.0 · 시작 안내 / 1234567890  예시 화면   D: 영상자료  H: 강의원고 읽기·쓰기 읽기 접근 안 됨'
chars=str(pages)+common
def face(bold):
 f=ttLib.TTFont('/usr/share/fonts/opentype/noto/NotoSansCJK-'+('Bold' if bold else 'Regular')+'.ttc',fontNumber=1,recalcTimestamp=False);q=subset.Subsetter();q.populate(text=chars);q.subset(f);o=BytesIO();f.save(o);return o.getvalue()
fonts=[face(False),face(True)];fm=[fitz.Font(fontbuffer=b) for b in fonts]
doc=fitz.open();ink=(.08,.15,.23);teal=(.03,.35,.35);bg=(.94,.97,.97)
for j,(headline,tag,a,b,c,data) in enumerate(pages):
 p=doc.new_page(width=570,height=592)
 for i,n in enumerate(['FReg','FBold']):p.insert_font(fontname=n,fontbuffer=fonts[i])
 def txt(x,y,t,h=12,bold=False,col=ink):p.insert_text((x,y),t,fontname=('FBold' if bold else 'FReg'),fontsize=h,color=col)
 def body(x,y,t):
  line=''
  for word in t.split():
   v=(line+' '+word).strip()
   if fm[0].text_length(v,fontsize=12)>478 and line:txt(x,y,line);y+=19;line=word
   else:line=v
  if line:txt(x,y,line)
 p.draw_rect((0,0,570,10),fill=teal,color=teal);txt(27,39,common.split(' / ')[0],11,True,teal);txt(495,39,f'{j+1} / 5',11)
 txt(27,85,headline,23,True);txt(27,113,tag,13,False,teal)
 for i,v in enumerate([a,b,c]):
  x=27+i*174;p.draw_rect((x,137,x+164,176),fill=bg,color=(.70,.82,.82));txt(x+8,161,v,11,True)
 if j==1:
  for i,row in enumerate([('D: 영상자료','읽기·쓰기','읽기'),('H: 강의원고','읽기','접근 안 됨')]):
   for k,v in enumerate(row):txt(35+k*174,192+i*19,v,10)
 for i,(k,t) in enumerate(data):
  y=243+i*108;txt(27,y,k,16,True,teal);body(27,y+25,t)
 p.draw_line((27,563),(543,563),color=(.77,.83,.84));txt(27,581,'차즘 NAS   /   '+tag,10)
doc.set_metadata({'title':'차즘 NAS 5.0.2.0 시작 안내','author':'ChaZeum NAS'});doc.save(P/'Guide.pdf',garbage=4,deflate=True,no_new_id=True)
