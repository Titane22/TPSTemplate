// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TPSTemplate/TPSTemplateCharacter.h"
#include "../AnimationState.h"
#include "LocomotionAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class TPSTEMPLATE_API ULocomotionAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
private:
	UFUNCTION()
	void UpdateLocomotionDirection();

	void TurnInPlace();

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void UpdateCharacterState();

	void RunningIntoWall();

	void UpdateWallDetection();

	void UpdateAcceleration();


	UPROPERTY(BlueprintReadOnly, Category = "Components")
	class ATPSTemplateCharacter* CharacterRef;

	UPROPERTY(BlueprintReadOnly, Category = "Components")
	class UCharacterMovementComponent* MovementComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Components")
	EAnimationState AnimationState;

	UPROPERTY(BlueprintReadOnly, Category = "MovementData")
	FVector Velocity;

	UPROPERTY(BlueprintReadOnly, Category = "MovementData")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "MovementData")
	bool bShouldMove;

	UPROPERTY(BlueprintReadOnly, Category = "MovementData")
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "MovementData")
	FVector Acceleration;

	UPROPERTY(BlueprintReadOnly, Category = "MovementData")
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Orientation Angle")
	float FOrientationAngle;
	UPROPERTY(BlueprintReadOnly, Category = "Orientation Angle")
	float ROrientationAngle;
	UPROPERTY(BlueprintReadOnly, Category = "Orientation Angle")
	float BOrientationAngle;
	UPROPERTY(BlueprintReadOnly, Category = "Orientation Angle")
	float LOrientationAngle;

	UPROPERTY(BlueprintReadOnly, Category = "Orientation Angle")
	float DirectionAngle;

	UPROPERTY(BlueprintReadOnly, Category = "Turn in Place")
	float RootYawOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Turn in Place")
	FRotator MovingRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Turn in Place")
	FRotator LastMovingRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Turn in Place")
	float LastDistanceCurve;

	UPROPERTY(BlueprintReadOnly, Category = "Turn in Place")
	float DistanceCurve;

	UPROPERTY(BlueprintReadOnly, Category = "Turn in Place")
	float DeltaDistanceCurve;

	UPROPERTY(BlueprintReadOnly, Category = "Turn in Place")
	float AbsRootYawOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Turn in Place")
	float YawExcess;

	UPROPERTY(BlueprintReadOnly, Category = "Turn in Place")
	float Pitch;

	UPROPERTY(BlueprintReadOnly, Category = "World Acceleration")
	FVector LocalAcceleration2D;

	UPROPERTY(BlueprintReadOnly, Category = "World Acceleration")
	FVector WorldAcceleration2D;

	UPROPERTY(BlueprintReadOnly, Category = "World Acceleration")
	FRotator WorldRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Acceleration")
	bool bHasAcceleration;

	UPROPERTY(BlueprintReadOnly, Category = "Acceleration")
	bool bRunningIntoWall;

	UPROPERTY(BlueprintReadOnly, Category = "Acceleration")
	bool bIsWall;

	UPROPERTY(BlueprintReadOnly, Category = "World Velocity")
	FVector WorldVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "World Velocity")
	FVector WorldVelocity2D;

	UPROPERTY(BlueprintReadOnly, Category = "World Velocity")
	FVector LocalVelocity2D;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprint;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Is Crouching?"))
	bool bIsCrouching;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	ELandState LandState;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Is Jump?"))
	bool bIsJump;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	EMovementState MovementInput;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Is Aim?"))
	bool bIsAim;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Is Weapon Equip?"))
	bool bIsWeaponEquip;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Is Pistol Equip?"))
	bool bIsPistolEquip;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Is Rifle Equip?"))
	bool bIsRifleEquip;

	UPROPERTY(BlueprintReadOnly, Category = "Alpha")
	float Alpha = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Alpha")
	float UpperBodyAlpha = 1.0f;

	UPROPERTY(BlueprintReadOnly)
	FVector LeftHandIKOffset;
public:

	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool IsValid() const { return CharacterRef != nullptr; }
};
