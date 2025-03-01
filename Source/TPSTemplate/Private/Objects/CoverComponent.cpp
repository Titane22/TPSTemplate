// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/CoverComponent.h"
#include "../TPSTemplateCharacter.h"
#include "Objects/CoverableActor.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UCoverComponent::UCoverComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCoverComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}
//
//TEnumAsByte<EDrawDebugTrace::Type> UCoverComponent::GetTraceDebugType(TEnumAsByte<EDrawDebugTrace::Type> ShowTraceType)
//{
//	return ShowTraceType;
//}

// Called every frame
void UCoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (!CharacterRef)
		return;
	if (CurrentCoverdActor)
		return;

	FVector StartLocation = CharacterRef->GetActorLocation();
	FVector ForwardVector = CharacterRef->GetActorForwardVector();
	
	// 가장 가까운 커버 액터를 추적하기 위한 변수들
	ACoverableActor* ClosestCoverActor = nullptr;
	float ClosestDistance = MAX_FLT;
	FHitResult ClosestHitResult;

	// 120도 각도를 커버하기 위해 10도 간격으로 라인 트레이스
	for (int32 i = -60; i <= 60; i += 10)
	{
		// 회전 각도 계산
		FRotator Rotation = FRotator(0.0f, i, 0.0f);
		FVector Direction = Rotation.RotateVector(ForwardVector);
		FVector EndLocation = StartLocation + (Direction * DetectionDistance);

		FHitResult HitResult;
		bool bHit = UKismetSystemLibrary::LineTraceSingle(
			GetWorld(),                          // WorldContextObject
			StartLocation,                       // Start
			EndLocation,                         // End
			UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_WorldDynamic),  // TraceChannel
			false,                              // bTraceComplex
			TArray<AActor*>{ CharacterRef },    // ActorsToIgnore
			EDrawDebugTrace::ForOneFrame,       // DrawDebugType
			HitResult,                          // OutHit
			true,                               // bIgnoreSelf
			FLinearColor::Red,                  // TraceColor
			FLinearColor::Green,                // TraceHitColor
			1.0f                                // DrawTime
		);

		if (bHit)
		{
			if (ACoverableActor* CoverableActor = Cast<ACoverableActor>(HitResult.GetActor()))
			{
				if (CoverableActor != PreviousClosestActor)
				{
					// TODO: 다음 엄폐물 찾기
					// 충돌 지점의 법선 벡터와 방향 벡터의 내적을 계산
					float DotProduct = FVector::DotProduct(HitResult.ImpactNormal, Direction);

					// 내적이 0보다 작으면 두 벡터가 서로 반대 방향임을 의미
					if (DotProduct < 0)
					{
						/*GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
							FString::Printf(TEXT("Hit! 충돌 법선과 방향이 반대 방향입니다. 내적: %f"), DotProduct));*/

							// 현재 히트된 액터와 캐릭터 사이의 거리 계산
						float CurrentDistance = FVector::Dist(StartLocation, HitResult.ImpactPoint);

						// 더 가까운 커버 액터를 찾았다면 업데이트
						if (CurrentDistance < ClosestDistance)
						{
							ClosestDistance = CurrentDistance;
							ClosestCoverActor = CoverableActor;
							ClosestHitResult = HitResult;
						}
					}
				}
			}
		}
	}
	
	if (ClosestCoverActor != nullptr)
	{
		// 커버 액터가 감지되면 카운터 초기화
		LostFrameCount = 0;
		
		// 이전과 다른 액터라면 이전 액터의 위젯 숨기기
		if (PreviousClosestActor != nullptr && PreviousClosestActor != ClosestCoverActor)
		{
			ACoverableActor* PreviousCoverableActor = Cast<ACoverableActor>(PreviousClosestActor);
			if (PreviousCoverableActor)
			{
				PreviousCoverableActor->ShowCoverPrompt(false);
			}
		}
		
		// 현재 액터의 위젯 표시
		ACoverableActor* CoverableActor = Cast<ACoverableActor>(ClosestCoverActor);
		if (CoverableActor)
		{
			CoverableActor->ShowCoverPrompt(true);
		}
		
		// 현재 액터를 이전 액터로 저장
		PreviousClosestActor = ClosestCoverActor;

		if (CharacterRef)
		{
			FCoverInfo coverInfo;
			coverInfo.CoverActor = ClosestCoverActor;
			coverInfo.ImpactPoint = ClosestHitResult.ImpactPoint;
			coverInfo.ImpactNormal = ClosestHitResult.ImpactNormal;
			coverInfo.Distance = ClosestDistance;

			CharacterRef->SetCanTakeCover(true, coverInfo);
		}
	}
	else
	{
		// 커버 액터가 감지되지 않으면 카운터 증가
		LostFrameCount++;
		
		// 디버그 메시지 출력 (선택 사항)
		
		// 일정 프레임 이상 감지 실패가 지속되고, 이전에 감지된 액터가 있는 경우에만 위젯 숨기기
		if (LostFrameCount > LostFrameThreshold + 10 && PreviousClosestActor != nullptr)
		{
			ACoverableActor* PreviousCoverableActor = Cast<ACoverableActor>(PreviousClosestActor);
			if (PreviousCoverableActor)
			{
				// 이전 액터의 위젯 숨기기
				PreviousCoverableActor->ShowCoverPrompt(false);
			}
			
			// 캐릭터에게 커버 불가능 상태 알림
			if (CharacterRef)
			{
				CharacterRef->SetCanTakeCover(false, FCoverInfo());
			}
			
			// 이전 액터 참조 초기화
			PreviousClosestActor = nullptr;
			
			// 디버그 메시지 출력 (선택 사항)
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("커버 감지 실패 - 위젯 숨김"));
		}
	}

	// 아웃라인 깜빡임 효과 (가장 가까운 액터가 있을 경우)
	//if (ClosestCoverActor || (PreviousClosestActor != nullptr && LostFrameCount <= LostFrameThreshold))
	//{
	//	// 현재 감지된 액터가 있거나, 이전 액터가 있고 아직 임계값에 도달하지 않은 경우 아웃라인 효과 유지
	//	ACoverableActor* OutlineActor = ClosestCoverActor ? ClosestCoverActor : PreviousClosestActor;
	//	
	//	OutlineBlinkTimer += DeltaTime;
	//	if (OutlineBlinkTimer >= OutlineBlinkInterval)
	//	{
	//		OutlineBlinkTimer = 0.0f;
	//		bOutlineVisible = !bOutlineVisible;
	//		SetActorOutline(OutlineActor, bOutlineVisible);
	//	}
	//}
	//else
	//{
	//	// 모든 아웃라인 효과 제거
	//	if (bOutlineVisible && PreviousClosestActor == nullptr)
	//	{
	//		bOutlineVisible = false;
	//		// 필요한 경우 모든 액터의 아웃라인 제거 로직 추가
	//	}
	//}
}

// 액터의 아웃라인 설정을 위한 헬퍼 함수
void UCoverComponent::SetActorOutline(ACoverableActor* Actor, bool bEnable)
{
	if (!Actor)
		return;
	
	// 스태틱 메쉬 컴포넌트 가져오기
	UStaticMeshComponent* MeshComponent = Actor->FindComponentByClass<UStaticMeshComponent>();
	if (MeshComponent)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("SetActorOutline"));
		// 커스텀 깊이 활성화 (아웃라인 렌더링에 필요)
		MeshComponent->SetRenderCustomDepth(bEnable);
		
		// 스텐실 값 설정 (아웃라인 색상 제어에 사용)
		MeshComponent->SetCustomDepthStencilValue(bEnable ? 1 : 0);
	}
}

void UCoverComponent::SetCharacterRef(ATPSTemplateCharacter* ToSetCharacterRef)
{
	CharacterRef = ToSetCharacterRef;
}
