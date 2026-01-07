// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/TPSTemplate_AIController.h"

ATPSTemplate_AIController::ATPSTemplate_AIController()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception");
	SetPerceptionComponent(*AIPerception);

	// Sight Configuration - 시각 설정
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");
	SightConfig->SightRadius = 1200.0f;  // 기본 시야 거리
	SightConfig->LoseSightRadius = 1500.0f;  // 시야를 잃는 거리
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;  // 주변 시야각 (180도가 전체)
	SightConfig->SetMaxAge(5.0f);  // 마지막으로 본 후 기억 유지 시간
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;  // 마지막 위치 근처에서 자동 탐지 거리
	AIPerception->ConfigureSense(*SightConfig);

	// Hearing Configuration - 청각 설정
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>("Hearing Config");
	HearingConfig->HearingRange = 2000.0f;  // 소리 감지 거리
	HearingConfig->SetMaxAge(3.0f);  // 소리 기억 유지 시간
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	AIPerception->ConfigureSense(*HearingConfig);

	// Damage Configuration - 피해 감지 설정
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>("Damage Config");
	DamageConfig->SetMaxAge(2.0f);  // 피해 기억 유지 시간
	AIPerception->ConfigureSense(*DamageConfig);

	// 주요 감각을 시각으로 설정
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ATPSTemplate_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AIPerception)
	{
		AIPerception->OnPerceptionUpdated.AddDynamic(this, &ATPSTemplate_AIController::OnPerceptionUpdated);
	}
}

void ATPSTemplate_AIController::OnPerceptionUpdated(const TArray<AActor*>& UpdateActors)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("Update Actor's Length: %d"), UpdateActors.Num()));
}
