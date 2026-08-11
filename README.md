# Yahboom ROS Robot Expansion Board (STM32F103RCT6) 2WD Platform

이 프로젝트는 **야붐 ROS 로봇 확장 보드 (V3.0)**와 **JGA25-371 감속 모터**를 사용하여 2륜 차동 제어 모바일 로봇 하위 제어기(LLC)를 직접 개발하기 위한 개발 환경 및 소스 코드 저장소입니다.

하드웨어 핀맵과 큐브MX 클럭/타이머 상세 가이드는 [PinMap.md](PinMap.md) 문서를 참고해 주세요.

---

## 1. 개발 및 빌드 환경
* **하드웨어:** STM32F103RCT6 LQFP64 Core Board
* **개발 IDE:** [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) (STM32CubeMX 내장)
* **연결 케이블:** 보드의 Micro-USB 포트와 PC 연결 (CH340 USB-to-Serial 드라이버 설치 필수)

---

## 2. 빌드 및 업로드 방법 (디버거 장비 없을 때)

별도의 디버깅 장비(ST-Link 등) 없이, 보드에 내장된 USB 포트(`COM` 포트)를 통해 코드를 직접 다운로드할 수 있는 간편 스크립트를 제공합니다.

### A. 빌드 진행 (STM32CubeIDE)
1. 프로젝트를 STM32CubeIDE에서 불러옵니다.
2. 상단 망치 아이콘을 클릭하거나 **`Ctrl + B`**를 눌러 프로젝트를 빌드합니다.
3. 빌드가 정상적으로 완료되면 `Debug/yahboom_jga.elf` 파일이 생성됩니다.

### B. 펌웨어 업로드 (Terminal)
윈도우 보안 정책에 구애받지 않고 업로드 스크립트를 즉시 실행하려면 터미널(PowerShell)에서 아래 명령어를 실행해 주세요.

```powershell
powershell -ExecutionPolicy Bypass -File .\upload.ps1
```

### C. 부트로더 모드 진입 및 실행 순서
`upload.ps1`을 실행하면 포트 및 타겟 파일을 자동으로 감지한 뒤, 부트로더 진입을 대기하는 안내 문구가 출력됩니다.

1. **보드 전원이 켜진 상태(12V 배터리 전원 스위치 ON)**인지 확인합니다.
2. 보드 위의 버튼을 아래 순서대로 조작합니다:
   * **`BOOT0`** 버튼을 누른 채로 유지합니다.
   * 그 상태에서 **`RESET` (RST)** 버튼을 짧게 눌렀다 뗍니다.
   * 누르고 있던 **`BOOT0`** 버튼을 뗍니다.
3. 터미널 창으로 돌아와 **아무 키나 한 번 누르면** 업로드가 시작됩니다.
4. **`SUCCESS: Upload complete and verified!`** 메시지가 나타나면 성공입니다.
5. 업로드 완료 후, 새 코드를 실행하기 위해 보드의 **`RESET`** 버튼을 한 번 더 눌러줍니다.

---

## 3. 업로드 헬퍼 스크립트 (`upload.ps1`) 특징
* **상대 경로 지원:** 수강생마다 프로젝트가 위치한 폴더 경로가 달라도 문제없이 `.elf` 파일을 자동 추적합니다.
* **프로그래머 자동 탐색:** `C:\ST` 하위의 모든 STM32CubeIDE/STM32CubeCLT 설치 경로 및 `C:\Program Files`를 자동으로 뒤져 ST 공식 업로드 툴(`STM32_Programmer_CLI.exe`)을 알아서 찾아서 구동합니다.
* **포트 자동 감지:** 활성화된 보드의 시리얼 COM 포트를 탐색하여 우선 매핑합니다.