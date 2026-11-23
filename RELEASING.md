# 업데이트와 외부 배포

**받는 사람:** 서명 검증이 끝난 릴리스에서는 기존처럼 Chazeum-NAS_SERVER.exe와 Chazeum-NAS_USER.exe만 받아 같은 용도로 사용합니다. 받는 사람이 인증서를 구매하거나 개발자 계정을 만들 필요는 없습니다. 앱을 ×로 종료하고 원래 로컬 폴더에 같은 이름의 새 버전을 덮어씁니다. 5.0.0.3에서 넘어올 때만 새 파일 이름을 사용합니다. 구버전의 자동 연결이 켜져 있으면 유저용을 처음 열 때 경로 이전을 한 번 확인합니다. 이후에는 같은 이름의 파일을 쓰므로 매번 자동 연결을 다시 설정하지 않습니다. 서명본을 미서명 개발본으로 바꾸지 마세요. 자동 업데이트를 위해 파일을 내려받거나 Windows의 보안 설정을 바꾸지 않습니다.

**최초 준비 (관리자):**

1. 이 소스만 유지관리자가 관리하는 공개 저장소에 올립니다. 개인 드라이브·공유 데이터는 올리지 않습니다. 홈페이지에 프로그램 설명, 실제 역할과 담당자, Code signing policy, 개인정보 정책과 릴리스 다운로드 링크를 연결합니다. 재단은 심사 신청 당시 실행 파일 형식으로 이미 배포 중인 프로젝트와 확인 가능한 활동·평판을 요구합니다. 미서명 평가판을 서명본으로 소개하지 않습니다.
2. GitHub에서 main 및 버전 태그의 변경을 보호하고 배포 환경 signpath-release를 제한합니다. GitHub 호스팅 빌드 결과, 기존 릴리스, 실제 사용·유지관리 근거를 준비한 후 [재단 신청서](https://signpath.org/apply)를 제출합니다. 명칭·설명만으로 승인이 보장되지 않습니다.
3. 승인 후 SignPath에서 이 저장소의 검증된 빌드만 받도록 연결하고, release/artifact-configuration.xml과 사람 승인이 필요한 배포 서명 정책을 등록합니다. GitHub 환경에 SIGNPATH_API_TOKEN은 비밀로, SIGNPATH_ORGANIZATION_ID, SIGNPATH_PROJECT, SIGNPATH_POLICY, SIGNPATH_ARTIFACT_CONFIG, SIGNPATH_CERT_SHA256은 확인한 사항만 등록합니다. 지문은 재단의 신뢰된 경로에서 별도로 확인합니다. 비밀번호나 토큰을 이 저장소·문서·채팅에 넣지 않습니다. 준비가 끝날 때까지 저장소 변수 SIGNPATH_ENABLED는 활성화하지 않습니다.

**새 버전마다 (관리자):**

1. 코드를 검토하고 두 EXE의 버전 리소스, 빌드 스크립트 안의 버전, 테스트, release/VERSION을 함께 맞춥니다. 독립적으로 다시 빌드한 두 결과를 대조한 후 release/unsigned-sha256.json을 검토·갱신합니다. 해시 비교가 실패하면 원인을 먼저 확인합니다.
2. 보호된 버전 태그에서 Build and verify Windows pair를 수동 실행합니다. 저장소 변수 SIGNPATH_ENABLED=true가 아니면 서명 단계는 시작하지 않습니다. 허용된 두 바이너리에 대해서만 유지관리자가 SignPath 승인 화면에서 서명을 승인합니다.
3. Windows에서 두 파일의 신뢰 체인·게시자 지문·타임스탬프와 원래 빌드와의 일치를 확인해야 배포 결과물이 생깁니다. 인증서 갱신 시 새 지문도 재단 경로로 검증한 뒤 설정합니다. 실패하면 배포하지 않습니다.
4. 검증된 결과물의 Chazeum-NAS_SERVER.exe·Chazeum-NAS_USER.exe만 다운로드 폴더에 배포합니다. 공개 배포 페이지에 소스 태그, 라이선스 고지, Code signing policy, 두 서명본의 SHA-256을 연결합니다. 배포처에서 두 파일을 다시 받아 해시를 비교합니다. 정식 배포 전에 대상 Windows의 다운로드·실행, 이전 버전에서의 교체, 실제 외부망에서 허가된 계정의 저장·재열기, 읽기 전용 계정의 쓰기 거절을 시험합니다.

서명은 앱의 게시자와 파일 변조 여부를 검증하는 수단입니다. 계정 권한·접속 성공·전송 속도를 보증하지 않으며, 새로운 다운로드에는 Windows SmartScreen 평판 경고가 별도로 표시될 수 있습니다.

[SignPath 조건](https://signpath.org/terms) · [GitHub 빌드 연결 문서](https://docs.signpath.io/trusted-build-systems/github) · [Microsoft SmartScreen](https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/smartscreen-reputation)
