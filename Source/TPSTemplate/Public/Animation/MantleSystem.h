// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Curves/CurveVector.h"
#include "Library/MantleEnumLib.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MantleSystem.generated.h"

USTRUCT(BlueprintType)
struct FMantleTraceSettings
{
	GENERATED_BODY()

	// Maximum height for mantling detection
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Settings")
	float MaxLedgeHeight = 150.0f; // Falling = 150.0f, Grounded = 250.0f

	// Minimum height for mantling detection
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Settings")
	float MinLedgeHeight = 50.0f;

	// Maximum reach distance for mantling
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Settings")
	float ReachDistance = 70.0f;

	// Radius of forward trace sphere
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Settings")
	float ForwardTraceRadius = 30.0f;

	// Radius of downward trace sphere
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Settings")
	float DownwardTraceRadius = 30.0f;
};

USTRUCT(BlueprintType)
struct FMantleParams
{
	GENERATED_BODY()

	// Animation montage for mantling
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* AnimMontage = nullptr;

	// Curve for position/rotation correction during mantling
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UCurveVector* PositionCorrectionCurve = nullptr;

	// Starting position in animation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	float StartingPosition = 0.0f;

	// Animation play rate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	float PlayRate = 1.0f;

	// Starting offset for mantle
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
	FVector StartingOffset = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FMantleAsset
{
	GENERATED_BODY()

	// Animation montage for mantling
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* AnimMontage = nullptr;

	// Curve for position/rotation correction during mantling
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UCurveVector* PositionCorrectionCurve = nullptr;

	// Starting offset for mantling
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
	FVector StartingOffset;

	// Low height mantle settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Low Mantle")
	float LowHeight = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Low Mantle")
	float LowPlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Low Mantle")
	float LowStartPosition = 0.0f;

	// High height mantle settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "High Mantle")
	float HighHeight = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "High Mantle")
	float HighPlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "High Mantle")
	float HighStartPosition = 0.0f;
};

class ATimelineMantle;

USTRUCT(BlueprintType)
struct FMantleComponentAndTransform
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UPrimitiveComponent* Component = nullptr;

	UPROPERTY(BlueprintReadOnly) 
	FTransform Transform;

	// Default constructor
	FMantleComponentAndTransform()
	{
		Component = nullptr;
		Transform = FTransform::Identity;
	}

	// Constructor with parameters
	FMantleComponentAndTransform(UPrimitiveComponent* InComponent, const FTransform& InTransform)
		: Component(InComponent)
		, Transform(InTransform)
	{
	}

	// Check if the data is valid
	bool IsValid() const
	{
		return Component != nullptr;
	}
};

// Declare delegates for mantle events
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMantleStartDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMantleEndDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSTEMPLATE_API UMantleSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMantleSystem();

	// Event dispatchers for mantle system
	UPROPERTY(BlueprintAssignable, Category = "Mantle|Events")
	FOnMantleStartDelegate OnMantleStart;

	UPROPERTY(BlueprintAssignable, Category = "Mantle|Events")
	FOnMantleEndDelegate OnMantleEnd;

	UFUNCTION(BlueprintCallable, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	void MantleUpdate(float BlendIn);

	UFUNCTION(BlueprintCallable, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	void MantleEnd();

	UFUNCTION(BlueprintCallable, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	bool MantleCheck(FMantleTraceSettings ParamTraceSettings);

	UFUNCTION(BlueprintCallable, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	bool MantleGroundCheck();

	UFUNCTION(BlueprintCallable, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	bool MantleFallingCheck();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Function to broadcast mantle start event
	UFUNCTION(BlueprintCallable, Category = "Mantle|Events")
	void BroadcastMantleStart() { OnMantleStart.Broadcast(); }

	// Function to broadcast mantle end event
	UFUNCTION(BlueprintCallable, Category = "Mantle|Events")
	void BroadcastMantleEnd() { OnMantleEnd.Broadcast(); }

	/// <summary>
	/// Refernces
	/// </summary>
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	class UCharacterMovementComponent* CharacterMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	UAnimInstance* MainAnimInst;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* CapsuleComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	class ATPSTemplateCharacter* CharacterRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	class UTimelineComponent* MantleTimeline;
	/// <summary>
	/// Mantle System
	/// </summary>
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	FMantleComponentAndTransform MantleLedgeLS;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	FMantleParams MantleParams;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	FTransform MantleAnimatedStartOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	FTransform MantleActualStartOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	FTransform MantleTarget;
	/// <summary>
	/// Anims
	/// </summary>
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims", meta = (AllowPrivateAccess = "true"))
	EMantleAnimType MantleAnimType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims", meta = (AllowPrivateAccess = "true"))
	FMantleAsset CustomMantle2M;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims", meta = (AllowPrivateAccess = "true"))
	FMantleAsset CustomMantle1MRM;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims", meta = (AllowPrivateAccess = "true"))
	FMantleAsset CustomMantle1MLM;
	/// <summary>
	/// Configs
	/// </summary>
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configs", meta = (AllowPrivateAccess = "true"))
	EMantleHandType HandType = EMantleHandType::LeftHand;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configs", meta = (AllowPrivateAccess = "true"))
	FMantleTraceSettings FallingTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configs", meta = (AllowPrivateAccess = "true"))
	FMantleTraceSettings GroundedTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configs", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configs", meta = (AllowPrivateAccess = "true"))
	bool FallingCatch = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configs", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EDrawDebugTrace::Type> DebugType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configs", meta = (AllowPrivateAccess = "true"))
	float Mantle_Z_Offset;

	/// <summary>
	/// Anim Defaults
	/// </summary>
	// Manny character mantle assets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Defaults", meta = (AllowPrivateAccess = "true"))
	FMantleAsset UE5_Manny_Mantle_2m;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Defaults", meta = (AllowPrivateAccess = "true"))
	FMantleAsset UE5_Manny_Mantle_1m_RH;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Defaults", meta = (AllowPrivateAccess = "true"))
	FMantleAsset UE5_Manny_Mantle_1m_LH;

	// Quinn character mantle assets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Defaults", meta = (AllowPrivateAccess = "true"))
	FMantleAsset UE5_Quinn_Mantle_2m;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Defaults", meta = (AllowPrivateAccess = "true"))
	FMantleAsset UE5_Quinn_Mantle_1m_RH;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Defaults", meta = (AllowPrivateAccess = "true"))
	FMantleAsset UE5_Quinn_Mantle_1m_LH;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle|Timeline", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ATimelineMantle> TimelineMantleClass;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION(BlueprintCallable, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	bool CapsuleHasRoomCheck(UCapsuleComponent* Capsule, FVector TargetLocation, float HeightOffset, float RadiusOffset, TEnumAsByte<EDrawDebugTrace::Type> ParamDebugType);

	UFUNCTION(BlueprintCallable, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	FMantleAsset GetMantleAsset(EMantleType MantleType);


	UFUNCTION(BlueprintCallable, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	void MantleStart(float MantleHeight, FMantleComponentAndTransform MantleLedgeWS, EMantleType MantleType);


	UFUNCTION(BlueprintCallable, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	void SetHandType(EMantleHandType ParamHandType);

	UFUNCTION(BlueprintCallable, Category = "Mantle System", meta = (AllowPrivateAccess = "true"))
	void SetAnimType(EMantleAnimType ParamAnimType);
	
	/// <summary>
	/// Rotation System
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Rotation System", meta = (AllowPrivateAccess = "true"))
	bool SetActorLocationAndRotation(FVector NewLocation, FRotator NewRotation, bool bSweep, bool bTeleport, FHitResult& SweepHitResult);

	/// <summary>
	/// Debug
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EDrawDebugTrace::Type> GetTraceDebugType(TEnumAsByte<EDrawDebugTrace::Type> ShowTraceType);
	/// <summary>
	/// Utility
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Utility", meta = (AllowPrivateAccess = "true"))
	void GetControlForwardRightVector(FVector& ForwardVector, FVector& RightVector);

	UFUNCTION(BlueprintCallable, Category = "Utility", meta = (AllowPrivateAccess = "true"))
	FVector GetCapsuleBaseLocation(float ZOffset);

	UFUNCTION(BlueprintCallable, Category = "Utility", meta = (AllowPrivateAccess = "true"))
	FVector GetCapsuleLocationFromBase(FVector BaseLocation, float ZOffset);

	UFUNCTION(BlueprintCallable, Category = "Input", meta = (AllowPrivateAccess = "true"))
	FVector GetPlayerMovementInput();

	UFUNCTION(BlueprintCallable, Category = "Defaults", meta = (AllowPrivateAccess = "true"))
	float GetLastDirection();
};
