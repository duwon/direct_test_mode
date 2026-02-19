# Direct Test Mode (DTM) 전달 문서

## 1. 문서 목적
이 문서는 `C:\ble\direct_test_mode` 프로젝트를 내용을 정리한 문서입니다.

- 적용된 설정 값
- 하드웨어 연결 방식
- 컴파일/플래시 절차
- 동작 확인 및 트러블슈팅

## 2. 기준 환경

- SDK: nRF Connect SDK `v3.1.0-6c6e5b32496e`
- Zephyr: `v4.1.99-1612683d4010`
- 대상 보드: `nrf52833dk/nrf52833`
- OS 기준: Windows PowerShell

## 3. 프로젝트 커스터마이징 요약

### 3.1 DTM UART 포트 고정
DTM 통신 포트를 `UART0`의 `P0.15(TX) / P0.17(RX)`로 고정했습니다.

- 파일: `boards/nrf52833dk_nrf52833.overlay`
- 설정:
  - `chosen { ncs,dtm-uart = &uart0; }`
  - `uart0_default`/`uart0_sleep`에 `P0.15/P0.17` 지정
  - `current-speed = <19200>;`
  - `/delete-property/ hw-flow-control;` (HWFC 사용 안 함)
  - `/delete-node/ group2;` (기본 핀맵 잔여 충돌 제거)

### 3.2 콘솔/로그 출력 비활성화
DTM 2-wire 바이너리 프레임에 텍스트가 섞이지 않도록 UART 텍스트 출력을 끄도록 설정했습니다.

- 파일: `prj.conf`
- 설정:
  - `CONFIG_UART_CONSOLE=n`
  - `CONFIG_LOG=n`
  - `CONFIG_NCS_BOOT_BANNER=n`
  - `CONFIG_BOOT_BANNER=n`
  - `CONFIG_EARLY_CONSOLE=n`
  - `CONFIG_PRINTK=n`

### 3.3 애플리케이션 코드 정리
`main.c`에서 `printk` 출력 코드를 제거했습니다.

- 파일: `src/main.c`
- 영향:
  - 부팅/오류 텍스트가 UART로 출력되지 않음
  - DTM 명령/응답 프레임만 UART에 남도록 유지

## 4. 하드웨어 연결

### 4.1 UART 배선
외부 테스터(또는 USB-UART 어댑터)와 아래처럼 연결합니다.

- `nRF52833 P0.15 (TX)` -> `상대 RX`
- `nRF52833 P0.17 (RX)` <- `상대 TX`
- `GND` <-> `GND`

### 4.2 UART 통신 파라미터

- Baudrate: `19200`
- Data bits: `8`
- Parity: `None`
- Stop bit: `1`
- Flow control: `None`

### 4.3 중요 주의사항
`nRF52833 DK`의 기본 J-Link VCOM 경로는 보드 기본 UART 핀맵(일반적으로 P0.06/P0.08 기준)을 사용합니다.  
현재 프로젝트는 DTM UART를 `P0.15/P0.17`로 바꿨기 때문에, 고객사 테스트 장비는 이 핀에 직접 연결되어야 합니다.

## 5. 빌드 방법

## 5.1 권장: NCS 전용 터미널에서 `west build`
`nRF Connect for VS Code` 또는 `Toolchain Manager`의 전용 터미널을 사용합니다.

```powershell
cd C:\ble\direct_test_mode
west build -p always -b nrf52833dk/nrf52833 .
```

- `-p always`: pristine build (캐시/설정 꼬임 방지)
- 결과물:
  - `build\direct_test_mode\zephyr\zephyr.hex`
  - `build\direct_test_mode\zephyr\zephyr.elf`

## 5.2 대안: 이미 configure된 빌드 디렉터리 재컴파일
현재 저장소처럼 `build`가 이미 생성되어 있으면 증분 빌드가 가능합니다.

```powershell
cd C:\ble\direct_test_mode
cmake --build build
```

## 5.3 완전 초기화 후 재빌드(선택)
빌드 캐시가 의심될 때 사용합니다.

```powershell
cd C:\ble\direct_test_mode
Remove-Item -Recurse -Force build
west build -p always -b nrf52833dk/nrf52833 .
```

## 6. 플래시 방법

```powershell
cd C:\ble\direct_test_mode
west flash --build-dir build
```

Sysbuild 도메인 기준으로는 아래도 사용 가능합니다.

```powershell
west flash --build-dir build --domain direct_test_mode
```

## 7. 동작 확인 방법

### 7.1 직렬 포트 툴로 기본 확인
RealTerm/TeraTerm/Terminal에서 포트를 열고 16진수 DTM 명령을 전송합니다.

- 예시 명령: `0x00 0x00` (Reset)
- 기대 응답: `0x00 0x00` (Success status event)

### 7.2 DTM 앱 사용 시 확인 항목

- 대상 포트를 다른 프로그램이 점유하지 않아야 함
- 앱의 UART 파라미터가 `19200 8N1 no flow control`과 일치해야 함
- 물리 배선이 `P0.15/P0.17`에 연결되어야 함

## 8. 트러블슈팅

### 8.1 "Cannot communicate with the device"가 나오는 경우

- 잘못된 포트 선택 여부 확인
- 배선(TX/RX 교차, GND 공통) 확인
- Baudrate가 `19200`인지 확인
- 하드웨어 플로우컨트롤(CTS/RTS)이 꺼져 있는지 확인
- 이전 펌웨어가 남아있지 않도록 pristine build + 재플래시 수행

### 8.2 부팅 문자열이 UART로 보이는 경우
현재 설정에서는 부팅 배너/printk가 비활성화되어야 합니다.  
문자열이 보이면 구버전 바이너리가 올라간 경우가 많습니다.

- `west build -p always ...`
- `west flash ...`
- 다시 확인
