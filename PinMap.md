# Yahboom ROS Robot Expansion Board (STM32F103RCT6) Pinout & Timer Mapping

이 문서는 **Yahboom ROS Robot Expansion Board (V3.0)** 기반의 **2륜 구동(2WD) 차동 제어 모바일 로봇 플랫폼**을 구현하기 위한 STM32F103RCT6 하드웨어 핀맵 및 타이머 매핑 정보를 정리한 문서입니다.

![STM32F103RCT6 Coprocessor Schematic](img/schematic.png)

기존 팩토리 펌웨어 소스 코드의 Board Support Package(BSP) 분석 및 회로도 검증을 거쳐 작성되었습니다.

---

## 1. 2륜 구동 모터 및 엔코더 설정 (핵심)

2WD 로봇 플랫폼 구현을 위해 **모터 1(좌측)** 및 **모터 2(우측)** 채널만 사용합니다.

### A. 모터 속도 제어 (PWM)
모터 드라이버는 모터 하나당 2개의 PWM 입력을 받는 **Dual PWM 방식**을 사용합니다.
* **TIM8** 타이머 하나로 2개의 모터를 제어합니다.



| 모터 채널 | 핀 번호 (Net) | STM32 핀 | 타이머 채널 | CubeMX 설정 모드 | 관련 소스 코드 링크 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Motor 1 A** | Pin 37 (`M1A`) | **PC6** | TIM8 CH1 | PWM Generation CH1 | [bsp_motor.h](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_motor.h#L52) |
| **Motor 1 B** | Pin 38 (`M1B`) | **PC7** | TIM8 CH2 | PWM Generation CH2 | [bsp_motor.h](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_motor.h#L55) |
| **Motor 2 A** | Pin 39 (`M2A`) | **PC8** | TIM8 CH3 | PWM Generation CH3 | [bsp_motor.h](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_motor.h#L60) |
| **Motor 2 B** | Pin 40 (`M2B`) | **PC9** | TIM8 CH4 | PWM Generation CH4 | [bsp_motor.h](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_motor.h#L63) |

### B. 엔코더 피드백 (Encoder Mode)
* 엔코더 A/B상 펄스를 하드웨어적으로 카운트하기 위해 **TIM2**와 **TIM4**를 **Encoder Mode**로 활성화합니다.



| 인코더 채널 | 핀 번호 (Net) | STM32 핀 | 사용 타이머 | 리맵(Remap) 여부 | 관련 소스 코드 링크 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Encoder 1 A** | Pin 50 (`H1A`) | **PA15** | TIM2 CH1 | **Partial Remap 1** 필요 | [bsp_encoder.h](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_encoder.h#L18) |
| **Encoder 1 B** | Pin 55 (`H1B`) | **PB3** | TIM2 CH2 | **Partial Remap 1** 필요 | [bsp_encoder.h](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_encoder.h#L21) |
| **Encoder 2 A** | Pin 59 (`H2A`) | **PB7** | TIM4 CH1 | X (기본 매핑) | [bsp_encoder.h](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_encoder.h#L27) |
| **Encoder 2 B** | Pin 58 (`H2B`) | **PB6** | TIM4 CH2 | X (기본 매핑) | [bsp_encoder.h](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_encoder.h#L30) |

> [!IMPORTANT]
> **주의: TIM2 Encoder Remap 및 JTAG 해제**
> `PA15`와 `PB3`은 기본적으로 JTAG 디버깅 핀으로 묶여 있습니다. 디버깅 인터페이스로 SWD를 사용하도록 설정하고, JTAG을 비활성화해야 해당 핀들을 TIM2 엔코더 입력 핀으로 사용할 수 있습니다.

---

## 2. 기타 주변장치 및 통신 핀맵

상위 SBC(Raspberry Pi/Jetson)와의 통신, 센서 정보 수집, 서보 제어 등을 위한 핀맵입니다.

| 기능 분류 | 주변장치 명칭 | STM32 핀맵 | 설명 / CubeMX 설정 | 관련 소스 코드 링크 |
| :--- | :--- | :--- | :--- | :--- |
| **상위 PC 통신** | **USART1** | **PA9 (TX)**, **PA10 (RX)** | USB-to-Serial(CH340)로 상위 ROS 노드와 통신. (DMA1 Channel 4 송신 활용) | [bsp_usart.c:USART1_Init](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_usart.c#L61) |
| **무선 조종 수신** | **USART2** | **PA3 (RXD2)** | SBUS 리시버 연결용. (100,000bps, 8-E-2 또는 9-E-2 구성, RX 단방향 통신) | [bsp_usart.c:USART2_Init](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_usart.c#L191) |
| **버스 서보 제어** | **USART3** | **PC10 (TXD3)**, **PC11 (RXD3)** | 직렬 버스 스마트 서보 모터 제어용. **Partial Remap** 활성화 필요. | [bsp_usart.c:USART3_Init](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_usart.c#L269) |
| **PWM 서보** | **S1 ~ S4** | **PC3, PC2, PC1, PC0** | GPIO Output으로 설정하여 **TIM7 인터럽트**(10us 주기)를 통한 Software PWM 구동. | [bsp_pwmServo.h](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_pwmServo.h#L24-L33) |
| **전압 측정** | **ADC1** | **PC4 (BAT)** | 배터리 전압 아날로그 입력 (ADC1_IN14). $10\text{ k}\Omega / 3.3\text{ k}\Omega$ 저항 분배를 거쳐 실제 전압의 약 $1/4.03$ 측정. | [bsp_adc.c](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_adc.c) |
| **IMU 센서** | **MPU9250 / ICM20948** | **PB12 (NSS)**, **PB13 (SCLK)**, **PB14 (SDO)**, **PB15 (SDI)** | **ICM20948:** 하드웨어 **SPI2** 통신 구동.<br>**MPU9250:** `PB13 (SCL)`, `PB15 (SDA)`로 **Software I2C** 구동. | [bsp_mpuiic.h](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_mpuiic.h#L20-L29) |
| **OLED / 외장 I2C** | **Software I2C** | **PB10 (SCL)**, **PB11 (SDA)** | OLED 스크린 구동 및 외부 I2C 슬레이브 확장용. | [bsp_io_i2c.c](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_io_i2c.c#L8-L10) |
| **RGB LED** | **SPI3 + DMA2** | **PB5 (RGB)** | WS2812B 구동용. SPI3 MOSI Remap 핀으로 DMA2 Channel 2를 사용해 고속 전송. | [bsp_rgb.c](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_rgb.c#L36-L51) |
| **CAN 통신** | **CAN_RX / CAN_TX** | **PB8 (CAN_RX)**, **PB9 (CAN_TX)** | CAN 통신용 핀. 리맵(Remap) 활성화 필요. | [bsp_can.c](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_can.c) |
| **경보 부저** | **Buzzer** | **PC5** | GPIO Output. High 출력 시 부저 켜짐. | [bsp_beep.h](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_beep.h#L12) |
| **사용자 스위치** | **KEY1** | **PD2** | GPIO Input. 풀업 설정 권장. | [bsp_key.h](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_key.h#L7) |
| **상태 표시 LED** | **LED / SW_LED** | **PC13 (State LED)**, **PA12 (Switch LED)** | 동작 상태 표시 및 모듈 전원 표시 LED. GPIO Output. | [bsp_common.h](file:///c:/ws/project_jelly/yahboom_jga/extracted/Factory_Source/V3.5.1/Source/BSP/bsp_common.h#L49-L55) |

---

## 3. STM32CubeMX 설정 가이드 (체크리스트)

새 큐브MX 프로젝트 생성 시 아래 단계를 반드시 수행해 주세요.

1. **SYS (Debug) 설정:** 
   * `SYS` $\rightarrow$ `Debug` $\rightarrow$ **Serial Wire** 선택 (PA15, PB3 핀을 Encoder 핀으로 사용하기 위해 필수적임).
2. **Clock 설정 (RCC):** 
   * `RCC` $\rightarrow$ `HSE` $\rightarrow$ **Crystal/Ceramic Resonator** 선택.
   * `Clock Configuration` 탭으로 이동하여 **System Clock(SYSCLK)을 72MHz**로 설정. -> ## 4. 참고
3. **TIM8 (모터 PWM):**
   * `TIM8` 활성화 $\rightarrow$ Channel 1/2/3/4 모두 **PWM Generation CHx**로 설정.
   ![STM32CubeMX TIM8 PWM Configuration](img/tim8_config.png)
4. **TIM2 (모터 1 엔코더):**
   * `TIM2` 활성화 $\rightarrow$ Combined Channels $\rightarrow$ **Encoder Mode** 선택.
   * 핀매핑이 `PA15` 및 `PB3`으로 선택되었는지 확인 (Remap 활성화 확인).
   * Parameter Settings에서 **Encoder Mode**를 **Encoder Mode TI1 and TI2**로 설정.
   ![STM32CubeMX TIM2 Encoder Configuration](img/tim2_config.png)
5. **TIM4 (모터 2 엔코더):**
   * `TIM4` 활성화 $\rightarrow$ Combined Channels $\rightarrow$ **Encoder Mode** 선택 (`PB6`, `PB7` 자동 매핑).
   * Parameter Settings에서 **Encoder Mode**를 **Encoder Mode TI1 and TI2**로 설정.
6. **USART1 (ROS 통신):**
   * `USART1` $\rightarrow$ **Asynchronous** 활성화 (`PA9`, `PA10` 매핑).
    * DMA Settings 탭 $\rightarrow$ `USART1_TX`를 추가하여 DMA 채널 할당.
   ![STM32CubeMX USART1 DMA Configuration](img/usart1_dma_config.png)

7. **USART3 (직렬 버스 서보 - 필요 시):**
   * `USART3` $\rightarrow$ **Asynchronous** 활성화.
    * 핀매핑이 `PC10`, `PC11` 리맵으로 제대로 들어가는지 확인.

---

## 4. Clock Configuration (시스템 클럭 72MHz 설정 세부 단계)

야붐 로봇 보드에 장착된 8MHz 외부 크리스탈(HSE)을 소스로 삼아 최대 시스템 클럭인 **72MHz**를 설정하는 구체적인 단계입니다.

![STM32CubeMX Clock Configuration](img/clock_config.png)

1. **외부 클럭 소스 활성화 (Pinout & Configuration 탭):**
   * 좌측 메뉴 `System Core` $\rightarrow$ `RCC`로 이동합니다.
   * **High Speed Clock (HSE)** 설정을 **Crystal/Ceramic Resonator**로 변경합니다. (이 작업을 완료해야 `Clock Configuration` 탭에서 HSE 관련 회로 라인이 활성화됩니다.)

2. **클럭 트리 구성 (Clock Configuration 탭):**
   * **PLL Source Mux** (아래쪽 중간): 입력 소스를 **HSE**로 변경합니다. (HSE 바로 옆 드롭다운은 `/ 1`로 설정하여 8MHz가 그대로 입력되도록 합니다.)
   * **System Clock Mux** (정중앙): 입력 소스를 **PLLCLK**로 변경합니다.
   * **HCLK (MHz)** 설정: 정중앙 우측의 파란색 `HCLK (MHz)` 텍스트 박스에 **`72`**를 입력하고 **Enter** 키를 누릅니다.
   * **해결 팝업 창 처리:** "Resolve Clock Issues" 팝업창이 표시되면 **OK**를 누릅니다. STM32CubeMX가 자동으로 PLL 배율(`PLLMul`을 `x9`로) 및 기타 버스 분주비를 안전하게 구성해 줍니다.

3. **최종 설정 검증:**
   * **SYSCLK (MHz) / HCLK (MHz):** `72`
   * **PLLMul:** `* 9` (HSE 8MHz $\times$ 9 = 72MHz)
   * **APB1 Prescaler (PCLK1):** `/ 2` (**36MHz Max** - STM32F1 시리즈 한계치 속도로 반드시 2분주되어야 정상 구동됩니다.)
   * **APB2 Prescaler (PCLK2):** `/ 1` (72MHz)

---

## 5. Project Manager Settings (프로젝트 및 코드 생성 설정)

코드 생성 및 IDE 빌드/업로드를 위해 `Project Manager` 탭에서 설정해야 할 필수 및 권장 옵션입니다.

![STM32CubeMX Project Manager Configuration](img/project_manager_config.png)

1. **Project Settings (기본 프로젝트 설정):**
   * **Project Name:** `yahboom_jga` (현재 작업 공간 폴더와 매치)
   * **Project Location:** `C:\ws\project_jelly\` (프로젝트 폴더가 작업 공간 내에 생성되도록 지정)
   * **Toolchain / IDE:** **STM32CubeIDE** 선택 (Generate Under Root 체크)

2. **Code Generator Settings (코드 생성 옵션 - 권장):**
   * 왼쪽 메뉴의 **Code Generator** 탭으로 이동합니다.
   * **Generated files** 그룹 박스에서 **`Generate peripheral initialization as a pair of '.c/.h' files per peripheral`** 항목에 **체크**를 해줍니다.
   * *이 설정을 활성화해야 GPIO, TIMER, USART 초기화 코드가 독립된 `.c`/`.h` 파일 쌍으로 생성되어 코드가 깔끔해지고 드라이버 코드 이식이 편해집니다.*