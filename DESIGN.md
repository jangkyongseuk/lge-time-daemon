# LGE Time Daemon - 설계 문서

## 1. 시스템 개요

LGE Time Daemon은 RTC(Real-Time Clock) 디바이스에서 시간을 읽어 시스템 시간을 UTC로 동기화하는 데몬 프로그램입니다. MISRA C++, AUTOSAR C++, 그리고 Scott Meyers의 Effective C++ 가이드라인을 준수하여 개발되었습니다.

## 2. 아키텍처 설계

### 2.1 전체 시스템 아키텍처

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   RTC Device    │    │  LGE Time       │    │  System Time    │
│   (/dev/rtc0)   │───▶│  Daemon         │───▶│  (UTC)          │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                              │
                              ▼
                       ┌─────────────────┐
                       │  Syslog         │
                       │  (Kernel Log)   │
                       └─────────────────┘
```

### 2.2 클래스 구조

#### 2.2.1 LgeTimeLib 클래스

**책임:**
- RTC 디바이스 읽기
- 날짜/시간 형식 검증
- 시스템 시간 설정
- 데몬 루프 관리
- 로깅 관리

**주요 속성:**
- `old_time`: 이전 시간 추적용
- `sleep_interval`: 동적 슬립 간격
- `running`: 데몬 실행 상태

**주요 메서드:**
- `readRtcTime()`: RTC에서 시간 읽기
- `validateDatetime()`: 형식 검증
- `setSystemTime()`: 시스템 시간 설정
- `run()`: 메인 루프 실행

### 2.3 데이터 흐름

```
RTC Device Read → Format Validation → Time Comparison → System Time Update → Sleep
     ↓                    ↓                  ↓               ↓              ↓
  /dev/rtc0         YYYY-MM-DD HH:MM:SS   old_time vs    settimeofday()   Dynamic
                                         current_time                     Interval
```

## 3. 상세 설계

### 3.1 시간 동기화 알고리즘

```cpp
while (running) {
    current_time = readRtcTime();
    
    if (current_time != old_time) {
        if (setSystemTime(current_time)) {
            old_time = current_time;
            sleep_interval = FAST_SLEEP_INTERVAL;  // 5초
        }
    } else {
        sleep_interval = NORMAL_SLEEP_INTERVAL;    // 60초
    }
    
    sleep(sleep_interval);
}
```

### 3.2 에러 처리 전략

1. **RTC 디바이스 접근 실패**
   - 기본 시간으로 대체 (2024-01-01 00:00:00)
   - 에러 로그 기록
   - 계속 실행

2. **시간 형식 검증 실패**
   - 시스템 시간 업데이트 건너뛰기
   - 에러 로그 기록
   - 다음 사이클에서 재시도

3. **시스템 시간 설정 실패**
   - 권한 부족 또는 시스템 에러
   - 에러 로그 기록
   - 계속 실행

### 3.3 성능 최적화

1. **동적 슬립 간격**
   - 시간 변경 시: 5초 간격 (빠른 응답)
   - 시간 안정 시: 60초 간격 (CPU 절약)

2. **정규식 캐싱**
   - 컴파일된 정규식 재사용
   - 매번 컴파일 오버헤드 방지

3. **메모리 효율성**
   - RAII 패턴 사용
   - 스마트 포인터 활용
   - 메모리 누수 방지

## 4. 보안 고려사항

### 4.1 권한 관리
- root 권한 필요 (시스템 시간 변경)
- 최소 권한 원칙 적용
- systemd 보안 설정 활용

### 4.2 입력 검증
- 정규식을 통한 엄격한 형식 검증
- 버퍼 오버플로우 방지
- 악의적 입력 차단

## 5. 테스트 전략

### 5.1 유닛 테스트
- 각 메서드별 독립 테스트
- 경계값 테스트
- 에러 케이스 테스트
- 성능 테스트

### 5.2 통합 테스트
- RTC 디바이스 모킹
- 시스템 시간 설정 테스트
- 전체 워크플로우 테스트

### 5.3 코드 커버리지
- 목표: 90% 이상
- lcov를 통한 리포트 생성
- CI/CD 파이프라인 통합

## 6. 배포 및 운영

### 6.1 시스템 서비스
- systemd 서비스로 등록
- 자동 재시작 설정
- 로그 관리

### 6.2 모니터링
- syslog를 통한 로그 기록
- journalctl을 통한 로그 확인
- 시스템 상태 모니터링

## 7. 확장성 고려사항

### 7.1 다중 RTC 지원
- 설정 파일을 통한 디바이스 경로 변경
- 여러 RTC 디바이스 동시 모니터링

### 7.2 타임존 지원
- 현재는 UTC 고정
- 향후 설정 가능한 타임존 지원

### 7.3 네트워크 시간 동기화
- NTP 클라이언트와의 연동
- 하이브리드 시간 동기화

## 8. 코딩 표준 준수

### 8.1 MISRA C++ 준수사항
- 동적 메모리 할당 최소화
- 예외 처리 명시적 구현
- 타입 안전성 보장

### 8.2 AUTOSAR C++ 준수사항
- 명명 규칙 준수
- 코드 구조화
- 문서화 표준

### 8.3 Effective C++ 준수사항
- RAII 패턴 활용
- const 정확성
- 복사 생성자/대입 연산자 관리
