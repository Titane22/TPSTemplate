// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Hurtbox.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSTEMPLATE_API UHurtbox : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hurtbox")
	TMap<FName, float> DamageMultipliers;

public:
	UFUNCTION(BlueprintCallable, Category = "Hurtbox")
	float GetDamageMultiplier(const FName HitBoneName) const;
};
