// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CoverComponent.generated.h"

class ACoverableActor;
class ATPSTemplateCharacter;

USTRUCT(BlueprintType)
struct FCoverInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category = "Cover")
	ACoverableActor* CoverActor = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "Cover")
	FVector ImpactPoint = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite, Category = "Cover")
	FVector ImpactNormal = FVector::UpVector;
	
	UPROPERTY(BlueprintReadWrite, Category = "Cover")
	float Distance = 0.0f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSTEMPLATE_API UCoverComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCoverComponent();

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configs", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EDrawDebugTrace::Type> DebugType;*/

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	float DetectionDistance = 350.0f;

	bool bIsCovering;

	ACoverableActor* CurrentCoverdActor = nullptr;

	ATPSTemplateCharacter* CharacterRef = nullptr;

	/*UFUNCTION(BlueprintCallable, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EDrawDebugTrace::Type> GetTraceDebugType(TEnumAsByte<EDrawDebugTrace::Type> ShowTraceType);*/

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetCharacterRef(ATPSTemplateCharacter* ToSetCharacterRef);
	
private:
	// 가장 가까운 커버 액터 추적
	ACoverableActor* PreviousClosestActor = nullptr;
	
	// 아웃라인 깜빡임 효과를 위한 변수들
	float OutlineBlinkTimer = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Cover|Visual", meta = (AllowPrivateAccess = "true"))
	float OutlineBlinkInterval = 0.5f;

	bool bOutlineVisible = true;
	
	// 액터 아웃라인 설정 헬퍼 함수
	void SetActorOutline(ACoverableActor* Actor, bool bEnable);

	int LostFrameCount = 0;

    const int LostFrameThreshold = 6; 
};
