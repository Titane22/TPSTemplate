// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class ATPSTemplateCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSTEMPLATE_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	float MaxHealth = 500.0f;

	float Health;

	bool StartWithMaxHealth;

public:	
	ATPSTemplateCharacter* CharacterRef;

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool ApplyDamage(float Damage);

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(float Value);

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetMaxHealth(float Value);
};
