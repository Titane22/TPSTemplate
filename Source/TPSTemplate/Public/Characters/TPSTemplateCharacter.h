// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Library/AnimationState.h"
#include "Logging/LogMacros.h"
#include "TPSTemplateCharacter.generated.h"

// Forward declarations
class UEquipmentSystem;
class UHealthSystem;
class UInteractor;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 * Base Character class for TPS Template
 * Contains only core character functionality shared by all character types
 * Player-specific features are in APlayer_Base
 */
UCLASS(config=Game)
class ATPSTemplateCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	// Weapon Socket Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Primary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Handgun;

	// Core Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UEquipmentSystem* EquipmentSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UHealthSystem* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UInteractor* InteractorComponent;

public:
	ATPSTemplateCharacter();

	// Weapon Child Actor Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* PrimaryChild;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* HandgunChild;

	// Animation State
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EAnimationState CurrentAnimationState;

	// State Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool Dead;

	// Death and Ragdoll
	UFUNCTION()
	void Die();

	UFUNCTION()
	void StartRagdoll();

	// Virtual functions that can be overridden by player/AI
	virtual void Interact();

	UFUNCTION(BlueprintCallable, Category = "InputAction")
	virtual void SwitchWeapons();

	UFUNCTION(BlueprintCallable, Category = "InputAction")
	virtual void SwitchToPrimaryWeapon();

	UFUNCTION(BlueprintCallable, Category = "InputAction")
	virtual void SwitchToHandgunWeapon();

	virtual void ReadyToFire(class AMasterWeapon* MasterWeapon, class UWeaponData* CurrentWeaponDataAsset);

	FORCEINLINE class UHealthSystem* GetHealthComponent() const { return HealthComponent; }

protected:
	virtual void BeginPlay();
};