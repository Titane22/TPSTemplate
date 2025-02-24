// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "Public/AnimationState.h"
#include "Logging/LogMacros.h"
#include "Components/TimelineComponent.h" 
#include "TPSTemplateCharacter.generated.h"

// 전방 선언
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AMasterWeapon;
class UHealthComponent;
class UInteractor;
class UWeaponSystem;
class ULocomotionAnimInstance;
class UWeaponDataAsset;
class UW_DynamicWeaponHUD;
class UMantleSystem;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(BlueprintType)
enum class ELandState : uint8
{
	Normal		UMETA(DisplayName = "Normal"),
	Soft		UMETA(DisplayName = "Soft"),
	Hard		UMETA(DisplayName = "Hard")
};

UCLASS(config=Game)
class ATPSTemplateCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Primary Socket*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Primary;

	/** Hand-Gun Socket*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Handgun;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Switch Weapons Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchWeaponsAction;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UWeaponSystem* WeaponSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UInteractor* InteractorComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UMantleSystem* MantleComponent;

	UPROPERTY()
	FTimeline CrouchTimeline;

	UPROPERTY()
	FTimeline AimTimeline;

	UPROPERTY()
	FTimeline ShoulderCameraTimeline;

	UFUNCTION()
	void UpdateAimView(float Value);

	UFUNCTION()
	void PlayDodgeMontage(UAnimMontage* MontageToPlay);

public:
	ATPSTemplateCharacter();

	/** Primary Child Socket*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* PrimaryChild;

	/** Handgun Child Socket*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* HandgunChild;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float TurnRate;

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
	bool Dead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bInteracting;

	// State Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ELandState CurrentLandState;

	// 현재 애니메이션 상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EAnimationState CurrentAnimationState;

	// Weapon Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class UW_DynamicWeaponHUD* CurrentWeaponUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class UUserWidget* UICrosshair;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class AMasterWeapon* CurrentWeapon;

	// Weapon States
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool IsAim;

	// Crosshair UI
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class ULocomotionAnimInstance* LocomotionBP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Timeline")
	UCurveFloat* CrouchCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Timeline")
	UCurveFloat* AimCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Timeline")
	UCurveFloat* ShoulderCameraCurve;

	UFUNCTION()
	void Die();

	UFUNCTION()
	void StartRagdoll();

	virtual void Interact();
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			
	UFUNCTION(BlueprintCallable, Category = "InputAction")
	virtual void SwitchWeapons();

	UFUNCTION(BlueprintCallable, Category = "InputAction")
	virtual void SwitchToPrimaryWeapon();

	UFUNCTION(BlueprintCallable, Category = "InputAction")
	virtual void SwitchToHandgunWeapon();

	UFUNCTION()
	void ShootFire(const FInputActionValue& Value);

	UFUNCTION()
	void Aim(const FInputActionValue& Value);

	UFUNCTION()
	void Reload();

	UFUNCTION()
	void Sprint(const FInputActionValue& Value);

	UFUNCTION()
	void SprintCompleted(const FInputActionValue& Value);

	UFUNCTION()
	void ToggleCrouch(const FInputActionValue& Value);

	UFUNCTION()
	void Dodge();

	UFUNCTION()
	void Jumping();

	UFUNCTION()
	void FlipFlapCameraChange();

	UFUNCTION()
	void ShoulderCameraChange(float Value);

	UFUNCTION()
	void UpdateCrouchHeight();

	void StopFire();

	void HandleFiring();

	bool CanFire();

	bool CanSwitchWeapon();

	virtual void ReadyToFire(class AMasterWeapon* MasterWeapon, class UWeaponDataAsset* CurrentWeaponDataAsset);

	UFUNCTION()
	void ImpactOnLand();
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay();

	virtual void Tick(float DeltaTime);

	virtual void OnLanded(const FHitResult& Hit);
public:
	/** Returns MainCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns HealthComponent subobject **/
	FORCEINLINE class UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void OnDodgeMontageInterrupted(UAnimMontage* Montage, bool bInterrupted);
};

