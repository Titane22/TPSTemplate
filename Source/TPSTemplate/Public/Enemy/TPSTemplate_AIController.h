// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Interfaces/EnemyState.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "TPSTemplate_AIController.generated.h"

UENUM(BlueprintType)
enum class ESensorType : uint8
{
	None		UMETA(DisplayName = "None"),
	Sight		UMETA(DisplayName = "Sight"),
	Hearing		UMETA(DisplayName = "Hearing"),
	Damage		UMETA(DisplayName = "Damage")
};

UENUM(BlueprintType)
enum class EAIState : uint8
{
	Passive			UMETA(DisplayName = "Passive"),
	Attacking		UMETA(DisplayName = "Attacking"),
	Investigating	UMETA(DisplayName = "Investigating")
};
/**
 * 
 */
UCLASS()
class TPSTEMPLATE_API ATPSTemplate_AIController : public AAIController, public IEnemyState
{
	GENERATED_BODY()
public:
	ATPSTemplate_AIController();

	virtual void OnPossess(APawn* InPawn);

	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdateActors);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	UAIPerceptionComponent* AIPerception;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	UAISenseConfig_Hearing* HearingConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	UAISenseConfig_Damage* DamageConfig;

	EAIState AIState;
};
