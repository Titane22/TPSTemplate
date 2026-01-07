// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Hurtbox.generated.h"

class ATPSTemplateCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSTEMPLATE_API UHurtbox : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hurtbox")
	TMap<FName, float> DamageMultipliers;

	ATPSTemplateCharacter* CharacterRef = nullptr;

	UPROPERTY()
	FTimerHandle RecoveryTimer;
public:
	UFUNCTION(BlueprintCallable, Category = "Hurtbox")
	float GetDamageMultiplier(const FName HitBoneName) const;

	void ApplyHitReaction(const FVector& HitLocation, const FVector& HitDirection, const FName BoneName, float Force);

	UFUNCTION(BlueprintCallable, Category = "Hurtbox")
	void RecoverFromHit();
	
private:
	FName ActivatedBoneName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Hurtbox|Hit Reaction")
	float MinRecoveryTime = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Hurtbox|Hit Reaction")
	float MaxRecoveryTime = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Hurtbox|Hit Reaction")
	float BasePhysicsBlendWeight = 0.3f;
};
