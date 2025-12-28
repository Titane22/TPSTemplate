// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/EquipmentSystem.h"
#include "GameFramework/Character.h"
#include "Library/AnimationState.h"
#include "Logging/LogMacros.h"
#include "TPSTemplateCharacter.generated.h"

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
	
	// Movement States
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsCrouch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsSprint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsSliding = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsDodging = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DodgeForward = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DodgeRight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool Right = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool Forward = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsJump = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bInteracting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bCanFire = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bIsAttacking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bCanSwitchWeapon = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bFiring = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool IsAim = false;
	
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
	bool bIsDead = false;

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

	virtual EWeaponSlot GetCurWeaponSlot() const { return EquipmentSystem ? EquipmentSystem->CurrentEquippedSlot : EWeaponSlot::None; }
	
protected:
	virtual void BeginPlay();
};