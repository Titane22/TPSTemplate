// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/EquipmentSystem.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/Damageable.h"
#include "Library/AnimationState.h"
#include "Logging/LogMacros.h"
#include "TPSTemplateCharacter.generated.h"

class UHurtbox;
class UInventorySystem;
class UHealthSystem;
class UInteractor;
class UCurveFloat;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 * Dodge montage structure for 4-directional dodge animations
 */
USTRUCT(BlueprintType)
struct FDodgeMontages
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* Forward = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* Backward = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* Right = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* Left = nullptr;
};

/**
 * Base Character class for TPS Template
 * Contains only core character functionality shared by all character types
 * Player-specific features are in APlayer_Base
 */
UCLASS(config=Game)
class ATPSTemplateCharacter : public ACharacter, public IDamageable
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UInventorySystem* InventorySystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UHealthSystem* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UInteractor* InteractorComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UHurtbox* Hurtbox;

	// Timelines
	UPROPERTY()
	FTimeline AimTimeline;

	UPROPERTY()
	FTimeline CrouchTimeline;

	// Timeline Curves (Blueprint-configurable)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* AimCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* CrouchCurve = nullptr;

	// Dodge Montages (Blueprint-configurable)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	FDodgeMontages DodgeMontages;

	// Virtual Timeline Callbacks
	UFUNCTION()
	virtual void UpdateAimTimeline(float Value);

	UFUNCTION()
	virtual void UpdateCrouchTimeline(float Value);

	// Montage Callbacks
	UFUNCTION()
	void OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnDodgeMontageInterrupted(UAnimMontage* Montage, bool bInterrupted);

	// Virtual Hooks for Subclasses
	virtual void OnAimStarted();
	virtual void OnAimEnded();
	virtual void OnCrouchStarted();
	virtual void OnCrouchEnded();
	virtual void OnDodgeStarted();
	virtual void OnDodgeEnded(UAnimMontage* Montage, bool bInterrupted);

	// Internal Helper
	virtual UAnimMontage* GetDodgeMontage(float ForwardInput, float RightInput);
	void PlayDodgeMontageInternal(UAnimMontage* MontageToPlay);

public:
	
	// Movement States
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsCrouch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsSprint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsSliding = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsDodging = false;

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
	bool bIsAim = false;
	
public:
	ATPSTemplateCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TMap<EEquipmentSlot, UChildActorComponent*> EquippedChilds;
	
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

	UFUNCTION()
	void StartRagdoll();

	UInventorySystem* GetInventorySystem() const { return InventorySystem; }
	
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

	virtual EEquipmentSlot GetCurWeaponSlot() const { return EquipmentSystem ? EquipmentSystem->CurrentEquippedSlot : EEquipmentSlot::None; }

	virtual void SetupEquipChildActor(EEquipmentSlot Slot);
	
	// Core Action Functions (Callable by AI or Player)
	void StartAim();
	void StopAim();
	void StartCrouch();
	void StopCrouch();
	void PerformDodge(float ForwardInput, float RightInput);

	virtual float TakeDamage_Implementation(float DamageAmount, const FDamageEvent& DamageEvent,
	                                        const FName HitBoneName, AController* EventInstigator, AActor* DamageCauser) override;

	virtual bool IsDead_Implementation() const override;
protected:
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnDeath();

	UFUNCTION()
	void OnHealthChanged(float NewHealth, float Damage);
};