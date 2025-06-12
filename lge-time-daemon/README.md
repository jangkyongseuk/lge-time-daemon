# LGE Time Daemon

RTC 디바이스에서 시간을 읽어 시스템 시간을 UTC로 동기화하는 데몬 프로그램입니다.

## 개요

LGE Time Daemon은 다음과 같은 기능을 제공합니다:

1. RTC 디바이스에서 지속적으로 날짜/시간 읽기
2. 날짜/시간 형식 검증
3. 시간 변경 시에만 시스템 시간 업데이트 (UTC 타임존 설정)
4. 시간 변경 여부에 따른 동적 슬립 간격 조정
5. 커널 메시지 버퍼에 작업 로그 기록

## 아키텍처

### 주요 컴포넌트

- **RTC_DEVICE**: RTC 하드웨어 디바이스 파일 경로 (`/dev/rtc0`)
- **sleep_interval**: 동적 슬립 지속시간 (시간 변경 시 짧아짐)
- **old_time**: 변경 감지를 위한 이전 시간 추적
- **DEFAULT_DATETIME**: RTC 읽기 실패 시 대체 시간

### 타임존 처리

- 모든 시간은 UTC 타임존(GMT+0)으로 설정
- 타임존 변환은 수행하지 않음
- 시스템 시간은 UTC로 유지

### 에러 처리

- RTC 파일 열기 가능 여부 확인
- 날짜/시간 형식 검증
- 모든 작업 및 에러 로그 기록

### 성능 고려사항

- 시간이 안정적일 때 긴 슬립 간격 사용
- 필요할 때만 시스템 시간 업데이트

## 빌드 및 설치

### 필요 조건

- CMake 3.16 이상
- C++17 지원 컴파일러
- Google Test (자동 다운로드)
- lcov (코드 커버리지용, 선택사항)

### 빌드

```bash
mkdir build
cd build
cmake ..
make
```

### 디버그 빌드 (코드 커버리지 포함)

```bash
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### 테스트 실행

```bash
# 유닛 테스트
make test

# 또는 직접 실행
./lge-time-test
```

### 코드 커버리지

```bash
# 디버그 빌드에서만 가능
make coverage
```

커버리지 리포트는 `coverage_html/index.html`에서 확인할 수 있습니다.

## 사용법

### 데몬 실행

```bash
sudo ./lge-time
```

### 시스템 서비스로 설치

```bash
sudo make install
sudo systemctl enable lge-time
sudo systemctl start lge-time
```

### 로그 확인

```bash
# 시스템 로그 확인
sudo journalctl -u lge-time -f

# 또는 dmesg
dmesg | grep lge-time
```

## 설정

### RTC 디바이스 경로 변경

`lge-time-lib.h`에서 `RTC_DEVICE` 상수를 수정:

```cpp
static const std::string RTC_DEVICE = "/dev/rtc1";  // 예시
```

### 슬립 간격 조정

`lge-time-lib.h`에서 슬립 간격 상수를 수정:

```cpp
static const int NORMAL_SLEEP_INTERVAL = 120;  // 2분
static const int FAST_SLEEP_INTERVAL = 10;     // 10초
```

## API 문서

### LgeTimeLib 클래스

#### 주요 메서드

- `readRtcTime()`: RTC 디바이스에서 시간 읽기
- `validateDatetime(const std::string&)`: 날짜/시간 형식 검증
- `setSystemTime(const std::string&)`: 시스템 시간 설정
- `run()`: 메인 데몬 루프 실행
- `stop()`: 데몬 중지

#### 상태 확인 메서드

- `getSleepInterval()`: 현재 슬립 간격 반환
- `getOldTime()`: 이전 시간 반환
- `isRunning()`: 실행 상태 확인

## 테스트

프로젝트는 Google Test를 사용한 포괄적인 유닛 테스트를 포함합니다:

- 날짜/시간 형식 검증 테스트
- RTC 디바이스 읽기 테스트
- 시스템 시간 설정 테스트
- 메인 루프 로직 테스트
- 성능 테스트
- 경계값 테스트

## 라이선스

이 프로젝트는 LG Electronics의 내부 프로젝트입니다.

## 기여

코드 기여 시 다음 가이드라인을 준수해주세요:

- MISRA C++ 코딩 룰 준수
- AUTOSAR C++ 코딩 룰 준수
- Scott Meyers의 Effective C++ 가이드라인 준수
- 모든 새로운 기능에 대한 유닛 테스트 작성
- 코드 커버리지 90% 이상 유지
