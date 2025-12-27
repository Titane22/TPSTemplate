// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TPSTemplateCharacter.h"
#include "Components/TimelineComponent.h"
#include "Player_Base.generated.h"

// Forward declarations
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UMantleSystem;
struct FInputActionValue;

UENUM(BlueprintType)
enum class ELandState : uint8
{
	Normal		UMETA(DisplayName = "Normal"),
	Soft		UMETA(DisplayName = "Soft"),
	Hard		UMETA(DisplayName = "Hard")
};

/**
 * Player-specific character class with input handling, camera, and player-only features
 */
UCLASS()
class TPSTEMPLATE_API APlayer_Base : public ATPSTemplateCharacter
{
	GENERATED_BODY()

protected:
	// Camera Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchWeaponsWheelAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchPrimaryWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchHandgunWeaponAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShootAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractionAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CameraChangeAction;

	// Player-specific Components
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UMantleSystem* MantleComponent;

	// Timelines
	UPROPERTY()
	FTimeline CrouchTimeline;

	UPROPERTY()
	FTimeline AimTimeline;

	UPROPERTY()
	FTimeline ShoulderCameraTimeline;

	// Input Handling Functions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ShootFire(const FInputActionValue& Value);
	void Aim(const FInputActionValue& Value);
	void Reload();
	void Sprint(const FInputActionValue& Value);
	void SprintCompleted(const FInputActionValue& Value);
	void ToggleCrouch(const FInputActionValue& Value);
	void Dodge();
	void Jumping();
	void FlipFlapCameraChange();

	// Timeline Update Functions
	UFUNCTION()
	void UpdateAimView(float Value);

	UFUNCTION()
	void ShoulderCameraChange(float Value);

	UFUNCTION()
	void UpdateCrouchHeight();

	UFUNCTION()
	void PlayDodgeMontage(UAnimMontage* MontageToPlay);

	// Weapon Functions
	void StopFire();
	void HandleFiring();
	bool CanFire();
	bool CanSwitchWeapon();

	UFUNCTION()
	void ImpactOnLand();

public:
	APlayer_Base();

	// Movement States
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsCrouch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsSprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsSliding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsDodging;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DodgeForward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DodgeRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool Right;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool Forward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsJump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bInteracting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsCovering;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsCoverable;

	// State Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ELandState CurrentLandState;

	// Weapon Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class UW_DynamicWeaponHUD* CurrentWeaponUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class UUserWidget* UICrosshair;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class AMasterWeapon* CurrentWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool IsAim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class UUserWidget* UIAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool IsPistolEquip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool IsPrimaryEquip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool IsWeaponEquip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bCanFire = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool IsAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bCanSwitchWeapon = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bFiring;

	// Camera variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	bool RightShoulder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float ShoulderYOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float ShoulderZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float TargetSpringAimZ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	FVector TargetArmLengths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float TurnRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Timeline")
	UCurveFloat* CrouchCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Timeline")
	UCurveFloat* AimCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Timeline")
	UCurveFloat* ShoulderCameraCurve;

	class ULocomotionAnimInstance* LocomotionBP;

	// Cover System Functions
	void EnterCoverState();
	void ExitCoverState();
	void MoveToCover();
	void UpdateCoverMovement();
	void CancelCoverMove();

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnDodgeMontageInterrupted(UAnimMontage* Montage, bool bInterrupted);

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnLanded(const FHitResult& Hit);

	UFUNCTION()
	UUserWidget* AddWeaponUI(UWeaponData* WeaponData);

	virtual void SwitchWeapons() override;
	virtual void SwitchToPrimaryWeapon() override;
	virtual void SwitchToHandgunWeapon() override;
	virtual void ReadyToFire(class AMasterWeapon* MasterWeapon, class UWeaponData* CurrentWeaponDataAsset) override;
	virtual void Interact() override;

	UFUNCTION()
	void ClearWeaponUI();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);
};
