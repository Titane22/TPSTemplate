# 프로젝트 관리 템플릿

## 📋 프로젝트 개요
- **프로젝트명**: TPSTemplate (FireFlies)
- **시작일**: 2024년 초 (약 11개월 전)
- **목적**: 공부 및 프로젝트
- **기술 스택**: Unreal Engine 5.4, C++, Blueprint, Enhanced Input, UMG, Niagara, AI Module 

---

## 🎯 목표
- [ ] 인벤토리: 같은 Instance ID의 Occupied Slot 무시
- [ ] 인벤토리: 이미 장착중인 슬롯에 장비 변경 시 위치 변경 (현재 TryAddItemEmptySpace 사용)
- [ ] 인벤토리: 무게 표시
- [ ] 인벤토리: 상점
- [ ] 인벤토리: 우클릭 컨텍스트 메뉴(장착, 버리기 등)
- [ ] 인벤토리: 아이템 밖으로 드롭 시 버리기
- [ ] 인벤토리: 아이템 호버 시 정보 표시
- [ ] 생존 게임을 위한 랜덤 아이템 스포너
- [ ] AI 테스트 레벨 구축

---

## 📝 진행 중 (In Progress)
- [ ] 피격 시스템 구현 (HurtBox, HitBox, HealthComponent)

---

## ✅ 완료된 작업 (Done)

### 최근 작업 (최근 2주)
- [x] 장비 시스템 개선 - 동적 슬롯 관리 및 장착 가능 체크 구현 (5분 전)
- [x] 장비 시스템 리팩토링 - EEquipmentSlot 도입 및 EquipmentBase 클래스 추가 (2일 전)
- [x] 루팅 시스템 구현 - 인벤토리 관리를 위한 루팅 컴포넌트 추가 (3일 전)
- [x] 그리드 기반 인벤토리 시스템 - 아이템 데이터 구조와 함께 구현 (5일 전)
- [x] 조준/앉기 타임라인 리팩토링 - 베이스 클래스로 이동 및 함수 정리 (6일 전)
- [x] 무기/상호작용 시스템 리팩토링 - 기능성 및 명확성 개선 (8일 전)
- [x] 맨틀 시스템 리팩토링 - TimelineMantle 제거, 수동 추적 방식으로 개선 (9일 전)
- [x] 플레이어 캐릭터 베이스 클래스 - 포괄적인 입력 처리 및 카메라 관리 구현 (9일 전)

### 핵심 시스템
- [x] 기본 캐릭터 시스템 구현 (ATPSTemplateCharacter) - 이동, 점프, 앉기, 구르기, 조준 기능
- [x] 플레이어 전용 클래스 구현 (APlayer_Base) - 입력 처리, 카메라, 플레이어 전용 기능
- [x] 적 캐릭터 기본 클래스 구현 (ATPSTemplate_Enemy_Base)
- [x] 무기 시스템 구현 (AMasterWeapon, UWeaponSystem) - 발사, 재장전, 탄약 관리
- [x] 장비 시스템 구현 (UEquipmentSystem) - 무기 슬롯 관리, 무기 전환, 픽업/드롭
- [x] 그리드 기반 인벤토리 시스템 구현 (UInventorySystem) - Resident Evil 스타일, 아이템 배치/이동
- [x] 루팅 시스템 구현 (ULootingSystem) - 가중치 기반 루트 생성
- [x] 체력 시스템 구현 (UHealthSystem) - 데미지/힐 처리
- [x] 이동 애니메이션 시스템 구현 (ULocomotionAnimInstance) - 방향, 속도, 상태 추적
- [x] 벽 타기 시스템 구현 (UMantleSystem) - 낙하/지상 상태별 맨틀, 위치 보정
- [x] 상호작용 시스템 구현 (AInteraction, UInteractionData) - DataAsset 기반 상호작용
- [x] 무기 HUD 시스템 구현 (UW_DynamicWeaponHUD) - 탄약 표시, 무기 정보
- [x] 플레이어 컨트롤러 구현 (AShooterPlayerController)
- [x] AI 컨트롤러 기본 구현 (ATPSTemplate_AIController)
- [x] 데이터 에셋 시스템 - WeaponData, ItemData, LootTableData, InteractionData
- [x] 애니메이션 상태 관리 (EAnimationState, AnimationState 라이브러리)
- [x] 무기 픽업/드롭 시스템 - Interaction 기반 무기 획득
- [x] 카메라 시스템 - SpringArm, 조준 타임라인, 어깨 카메라 전환
- [x] 아이템 스포너 및 루트 컨테이너 구현 (AItemSpawner, ALootContainer)

### 초기 작업 (11개월 전)
- [x] 커버 시스템 #1
- [x] 맨틀 시스템 완성
- [x] 맨틀 시스템 초기화
- [x] Health Bar HUD 추가
- [x] 아이템 픽업 및 드롭
- [x] 히트 적용 및 플레이어/적 캐릭터 상속 구조
- [x] 추가 입력 액션
- [x] 발사 액션 업데이트 및 재장전 액션 추가
- [x] 라이플 발사 액션 (WIP)
- [x] DynamicHUD 추가
- [x] AimOffset 및 발사 액션

---

## 📌 할 일 (TODO)
- [ ] 할 일 1
- [ ] 할 일 2
- [ ] 할 일 3

---

## 🐛 이슈 / 버그
### 버그
- [ ] 버그 설명 1
  - 재현 방법:
  - 예상 원인:
  - 우선순위: High/Medium/Low

### 개선 사항
- [ ] 개선 사항 1
- [ ] 개선 사항 2

---

## 📚 참고 자료
- [링크 제목](URL)
- 문서/리소스 이름

---

## 💡 노트 / 아이디어
- 아이디어 1
- 아이디어 2

---

## 📅 일지
### YYYY-MM-DD
- 작업 내용
- 배운 점
- 다음 작업 계획

---

## 🔗 관련 링크
- [GitHub](링크)
- [문서](링크)
- [참고 프로젝트](링크)
