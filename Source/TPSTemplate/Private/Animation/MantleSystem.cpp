// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/MantleSystem.h"
#include "TPSTemplate/TPSTemplateCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimInstance.h"

// Sets default values for this component's properties
UMantleSystem::UMantleSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

		
}


// Called when the game starts
void UMantleSystem::BeginPlay()
{
	Super::BeginPlay();

	if (ATPSTemplateCharacter* Character = Cast<ATPSTemplateCharacter>(GetOwner()))
	{
		CharacterRef = Character;

		CharacterMovement = CharacterRef->GetCharacterMovement();
		CapsuleComponent = CharacterRef->GetCapsuleComponent();
		MainAnimInst = CharacterRef->GetMesh()->GetAnimInstance();

		// TimelineMantle
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(FVector(0.0f, 0.0f, 0.0f));
		SpawnTransform.SetRotation(FVector(0.0f, 0.0f, 0.0f).Rotation().Quaternion());
		SpawnTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = CharacterRef;
		if (AActor* TimelineActor = GetWorld()->SpawnActor<AActor>(TimelineMantleClass, SpawnTransform, SpawnParams))
		{
			if (UTimelineComponent* Timeline = TimelineActor->FindComponentByClass<UTimelineComponent>())
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Timeline Found"));
				MantleTimeline = Timeline;  
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Timeline Not Found"));
			}
		}
	}
}


// Called every frame
void UMantleSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CharacterRef || !CharacterMovement)
	{
		return;
	}

	if (CharacterMovement->IsFalling() && FallingCatch && CharacterRef->IsControlled())
	{
		if (CharacterMovement->GetCurrentAcceleration().Size() / CharacterMovement->GetMaxAcceleration() > 0.0f)
		{
			// TODO: MantleFallingCheck();
		}
	}
}

bool UMantleSystem::CapsuleHasRoomCheck(UCapsuleComponent* Capsule, FVector TargetLocation, float HeightOffset, float RadiusOffset, TEnumAsByte<EDrawDebugTrace::Type> ParamDebugType)
{
	float capsuleAdjustedHeight = Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere() + RadiusOffset * -1.0f + HeightOffset;
	FVector StartLocation = TargetLocation + 
		FVector(
			0.0f, 
			0.0f, 
			capsuleAdjustedHeight
		);
	FVector EndLocation = TargetLocation -
		FVector(
			0.0f,
			0.0f,
			capsuleAdjustedHeight
		);

	FHitResult HitResult;
	UKismetSystemLibrary::SphereTraceSingleByProfile(
		this,
		StartLocation,
		EndLocation,
		Capsule->GetScaledCapsuleRadius() + RadiusOffset,
		FName("TPS_Character"),
		false,
		TArray<AActor*>(),
		GetTraceDebugType(DebugType),
		HitResult,
		true,
		FLinearColor::Green,
		FLinearColor::Red,
		1.0f
	);

	bool bBlockingHit = HitResult.bBlockingHit;
	bool bInitialOverlap = HitResult.bStartPenetrating;

	return (!bBlockingHit && !bInitialOverlap); // NOR
}

FMantleAsset UMantleSystem::GetMantleAsset(EMantleType MantleType)
{
	switch (MantleType)
	{
	case EMantleType::HighMantle:
	case EMantleType::FallingMantle:
		switch (MantleAnimType)
		{
		case EMantleAnimType::UE5_Manny:
			return UE5_Manny_Mantle_2m;
		case EMantleAnimType::UE5_Quinn:
			return UE5_Quinn_Mantle_2m;
		case EMantleAnimType::Custom:
			return CustomMantle2M;
		default:
			return FMantleAsset();
		}
	case EMantleType::LowMantle:
		switch (MantleAnimType)
		{
		case EMantleAnimType::UE5_Manny:
			switch (HandType)
			{
			case EMantleHandType::LeftHand:
				return UE5_Manny_Mantle_1m_LH;
			case EMantleHandType::RightHand:
				return UE5_Manny_Mantle_1m_RH;
			default:
				return FMantleAsset();
			}
		case EMantleAnimType::UE5_Quinn:
			switch (HandType)
			{
			case EMantleHandType::LeftHand:
				return UE5_Quinn_Mantle_1m_LH;
			case EMantleHandType::RightHand:
				return UE5_Quinn_Mantle_1m_RH;
			default:
				return FMantleAsset();
			}
		case EMantleAnimType::Custom:
			switch (HandType)
			{
			case EMantleHandType::LeftHand:
				return CustomMantle1MLM;
			case EMantleHandType::RightHand:
				return CustomMantle1MRM;
			default:
				return FMantleAsset();
			}
		default:
			return FMantleAsset();
		}
	default:
		return FMantleAsset();
	}
}

void UMantleSystem::MantleUpdate(float BlendIn)
{
	if (!MantleLedgeLS.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UMantleSystem::MantleUpdate Is NULL"));
		return;
	}
	float positionAlpha;
	float XYCorrectionAlpha;
	float ZCorrectionAlpha;
	FTransform lerpedTarget;

	// Step 1: Continually update the mantle target from the stored local transform to follow along with moving objects.
	{
		FTransform worldTransform = MantleLedgeLS.Component->GetComponentToWorld();
		FVector worldLocation = worldTransform.GetLocation();
		FRotator worldRotation = worldTransform.Rotator();
		FVector worldScale = worldTransform.GetScale3D();

		FVector inverseLocation = worldTransform.InverseTransformPosition(worldLocation);
		FRotator inverseRotation = worldTransform.InverseTransformRotation(worldRotation.Quaternion()).Rotator();
		FVector inverseScale = worldTransform.InverseTransformVector(worldScale);
		MantleTarget = FTransform(
			inverseRotation,
			inverseLocation,
			inverseScale
		);
	}
	// Step 2: Update the Position and Correction Alphas using the Position/Correction curve set for each Mantle.
	{
		FVector curveVector = MantleParams.PositionCorrectionCurve->GetVectorValue(MantleTimeline->GetPlaybackPosition() + MantleParams.StartingPosition);
		positionAlpha = curveVector.X;
		XYCorrectionAlpha = curveVector.Y;
		ZCorrectionAlpha = curveVector.Z;
	}
	// Step 3: Lerp multiple transforms together for independent control over the horizontal and vertical blend
	{
		// Blend into the animated horizontal and rotation offset using the Y value of the Position/Correction Curve.
		FTransform lerped_XY_Transform = FTransform(
			FRotator(
				FMath::Lerp(MantleActualStartOffset.Rotator().Pitch, MantleAnimatedStartOffset.Rotator().Pitch, XYCorrectionAlpha),
				FMath::Lerp(MantleActualStartOffset.Rotator().Yaw, MantleAnimatedStartOffset.Rotator().Yaw, XYCorrectionAlpha),
				FMath::Lerp(MantleActualStartOffset.Rotator().Roll, MantleAnimatedStartOffset.Rotator().Roll, XYCorrectionAlpha)
			),
			FVector(
				FMath::Lerp(MantleActualStartOffset.GetLocation().X, MantleAnimatedStartOffset.GetLocation().X, XYCorrectionAlpha),
				FMath::Lerp(MantleActualStartOffset.GetLocation().Y, MantleAnimatedStartOffset.GetLocation().Y, XYCorrectionAlpha),
				MantleActualStartOffset.GetLocation().Z  // Z값은 변경하지 않음
			),
			FVector(
				FMath::Lerp(MantleActualStartOffset.GetScale3D().X, 1.0f, XYCorrectionAlpha),
				FMath::Lerp(MantleActualStartOffset.GetScale3D().Y, 1.0f, XYCorrectionAlpha),
				FMath::Lerp(MantleActualStartOffset.GetScale3D().Z, 1.0f, XYCorrectionAlpha)
			)
		);
		
		// Blend into the animated vertical offset using the Z value of the Position/Correction Curve.
		FTransform lerped_Z_Transform = FTransform(
			FRotator(
				FMath::Lerp(MantleActualStartOffset.Rotator().Pitch, MantleAnimatedStartOffset.Rotator().Pitch, ZCorrectionAlpha),
				FMath::Lerp(MantleActualStartOffset.Rotator().Yaw, MantleAnimatedStartOffset.Rotator().Yaw, ZCorrectionAlpha),
				FMath::Lerp(MantleActualStartOffset.Rotator().Roll, MantleAnimatedStartOffset.Rotator().Roll, ZCorrectionAlpha)
			),
			FVector(
				MantleActualStartOffset.GetLocation().X,  // X값은 변경하지 않음
				MantleActualStartOffset.GetLocation().Y,  // Y값은 변경하지 않음
				FMath::Lerp(MantleActualStartOffset.GetLocation().Z, MantleAnimatedStartOffset.GetLocation().Z, ZCorrectionAlpha)
			),
			FVector(
				FMath::Lerp(MantleActualStartOffset.GetScale3D().X, 1.0f, ZCorrectionAlpha),
				FMath::Lerp(MantleActualStartOffset.GetScale3D().Y, 1.0f, ZCorrectionAlpha),
				FMath::Lerp(MantleActualStartOffset.GetScale3D().Z, 1.0f, ZCorrectionAlpha)
			)
		);
		FTransform lerpedFinalTransform = FTransform(
			lerped_XY_Transform.Rotator(),
			FVector(
				lerped_XY_Transform.GetLocation().X,
				lerped_XY_Transform.GetLocation().Y,
				lerped_Z_Transform.GetLocation().Z
			),
			FVector(1.0f, 1.0f, 1.0f)
		);
		// Blend from the currently blending transforms into the final mantle target using the X value of the Position/Correction Curve.
		FVector blendedLocation = MantleTarget.GetLocation() + lerpedFinalTransform.GetLocation();
		FRotator blendedRotation = FRotator(
			MantleTarget.Rotator().Pitch + lerpedFinalTransform.Rotator().Pitch,
			MantleTarget.Rotator().Yaw + lerpedFinalTransform.Rotator().Yaw,
			MantleTarget.Rotator().Roll + lerpedFinalTransform.Rotator().Roll
		);
		FVector blendedScale = MantleTarget.GetScale3D() + lerpedFinalTransform.GetScale3D();
		FTransform blendedTransform = FTransform(
			FRotator(
				FMath::Lerp(blendedRotation.Pitch, MantleTarget.Rotator().Pitch, positionAlpha),
				FMath::Lerp(blendedRotation.Yaw, MantleTarget.Rotator().Yaw, positionAlpha),
				FMath::Lerp(blendedRotation.Roll, MantleTarget.Rotator().Roll, positionAlpha)
			),
			FVector(
				FMath::Lerp(blendedLocation.X, MantleTarget.GetLocation().X, positionAlpha),
				FMath::Lerp(blendedLocation.Y, MantleTarget.GetLocation().Y, positionAlpha),
				FMath::Lerp(blendedLocation.Z, MantleTarget.GetLocation().Z, positionAlpha)
			),
			FVector(
				FMath::Lerp(blendedScale.X, MantleTarget.GetScale3D().X, positionAlpha),
				FMath::Lerp(blendedScale.Y, MantleTarget.GetScale3D().Y, positionAlpha),
				FMath::Lerp(blendedScale.Z, MantleTarget.GetScale3D().Z, positionAlpha)
			)
		);

		// Initial Blend In (controlled in the timeline curve) to allow the actor to blend into the Position/Correction curve at the midoint. This prevents pops when mantling an object lower than the animated mantle.
		FTransform sumMantleTransform = FTransform(
			FRotator(
				MantleTarget.Rotator().Pitch + MantleActualStartOffset.Rotator().Pitch,
				MantleTarget.Rotator().Yaw + MantleActualStartOffset.Rotator().Yaw,
				MantleTarget.Rotator().Roll + MantleActualStartOffset.Rotator().Roll
			),
			MantleTarget.GetLocation() + MantleActualStartOffset.GetLocation(),
			MantleTarget.GetScale3D() + MantleActualStartOffset.GetScale3D()
		);

		lerpedTarget = FTransform(
			FRotator(
				FMath::Lerp(blendedTransform.Rotator().Pitch, sumMantleTransform.Rotator().Pitch, BlendIn),
				FMath::Lerp(blendedTransform.Rotator().Yaw, sumMantleTransform.Rotator().Yaw, BlendIn),
				FMath::Lerp(blendedTransform.Rotator().Roll, sumMantleTransform.Rotator().Roll, BlendIn)
			),
			FVector(
				FMath::Lerp(blendedTransform.GetLocation().X, sumMantleTransform.GetLocation().X, BlendIn),
				FMath::Lerp(blendedTransform.GetLocation().Y, sumMantleTransform.GetLocation().Y, BlendIn),
				FMath::Lerp(blendedTransform.GetLocation().Z, sumMantleTransform.GetLocation().Z, BlendIn)
			),
			FVector(
				FMath::Lerp(blendedTransform.GetScale3D().X, sumMantleTransform.GetScale3D().X, BlendIn),
				FMath::Lerp(blendedTransform.GetScale3D().Y, sumMantleTransform.GetScale3D().Y, BlendIn),
				FMath::Lerp(blendedTransform.GetScale3D().Z, sumMantleTransform.GetScale3D().Z, BlendIn)
			)
		);
	}
	// Step 4: Set the actors location and rotation to the Lerped Target.
	{
		FVector newLocation = FVector(lerpedTarget.GetLocation().X, lerpedTarget.GetLocation().Y, lerpedTarget.GetLocation().Z + Mantle_Z_Offset);
		FRotator newRotation = lerpedTarget.Rotator();
		FHitResult HitResult;
		SetActorLocationAndRotation(newLocation, newRotation, false, false, HitResult);
	}
}

void UMantleSystem::MantleEnd()
{
	if (!CharacterMovement)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMantleSystem::MantleEnd Is NULL"));
		return;
	}

	CharacterMovement->SetMovementMode(EMovementMode::MOVE_Walking);
	BroadcastMantleEnd();
}

void UMantleSystem::MantleStart(float MantleHeight, FMantleComponentAndTransform MantleLedgeWS, EMantleType MantleType)
{
	if (!CharacterRef || !CharacterMovement)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMantleSystem::MantleStart Is NULL"));
		return;
	}
	BroadcastMantleStart();
	FMantleAsset mantleAsset;

	// Step 1: Get the Mantle Asset and use it to set the new Mantle Params.
	{
		mantleAsset = GetMantleAsset(MantleType);

		MantleParams = FMantleParams(
			mantleAsset.AnimMontage,
			mantleAsset.PositionCorrectionCurve,
			FMath::GetMappedRangeValueClamped(
				FVector2D(mantleAsset.LowHeight, mantleAsset.HighHeight),      // InRange
				FVector2D(mantleAsset.LowPlayRate, mantleAsset.HighPlayRate),  // OutRange
				MantleHeight                                                    // Value
			),
			FMath::GetMappedRangeValueClamped(
				FVector2D(mantleAsset.LowHeight, mantleAsset.HighHeight),				// InRange
				FVector2D(mantleAsset.LowStartPosition, mantleAsset.HighStartPosition),  // OutRange
				MantleHeight															// Value
			),
			mantleAsset.StartingOffset
		);
		// Step 2: Convert the world space target to the mantle component's local space for use in moving objects.
		{
			if (MantleLedgeWS.Component)
			{
				MantleLedgeLS.Component = MantleLedgeWS.Component;
				FTransform componentWorldTransform = MantleLedgeWS.Component->GetComponentToWorld();
				FTransform inverseTransform = componentWorldTransform.Inverse();
				MantleLedgeLS.Transform = UKismetMathLibrary::ComposeTransforms(
					MantleLedgeWS.Transform,
					inverseTransform
				);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("MantleLedgeWS.Component is null, using world space transform"));
				return;
			}
		}
		// Step 3: Set the Mantle Target and calculate the Starting Offset (offset amount between the actor and target transform).
		{
			MantleTarget = MantleLedgeLS.Transform;
			FTransform sourceTransform = CharacterRef->GetActorTransform();
			MantleActualStartOffset = FTransform(
				FRotator(
					sourceTransform.Rotator().Pitch - MantleTarget.Rotator().Pitch,
					sourceTransform.Rotator().Yaw - MantleTarget.Rotator().Yaw,
					sourceTransform.Rotator().Roll - MantleTarget.Rotator().Roll
				),
				sourceTransform.GetLocation() - MantleTarget.GetLocation(),
				sourceTransform.GetScale3D() - MantleTarget.GetScale3D()
			);
		}
		// Step 4: Calculate the Animated Start Offset from the Target Location. This would be the location the actual animation starts at relative to the Target Transform. 
		{
			FVector sourceNormal = MantleTarget.GetRotation().Vector();
			FVector horizontalOffset = sourceNormal * MantleParams.StartingOffset.Y;
			FVector sourceLocation = MantleTarget.GetLocation() - FVector(horizontalOffset.X, horizontalOffset.Y, MantleParams.StartingOffset.Z);
			FRotator sourceRotation = MantleTarget.Rotator();
			FVector sourceScale = FVector(1.0f, 1.0f, 1.0f);

			FVector targetLocation = MantleTarget.GetLocation();
			FRotator targetRotation = MantleTarget.Rotator();
			FVector targetScale = MantleTarget.GetScale3D();

			MantleAnimatedStartOffset = FTransform(
				FRotator(sourceRotation.Pitch - targetRotation.Pitch,
					sourceRotation.Yaw - targetRotation.Yaw,
					sourceRotation.Roll - targetRotation.Roll
				),
				sourceLocation - targetLocation,
				sourceScale - targetScale
			);
		}
		// Step 5: Clear the Character Movement Mode and set the Movement State to Mantling
		{
			CharacterMovement->SetMovementMode(EMovementMode::MOVE_None);
		}
		// Step 6: Configure the Mantle Timeline so that it is the same length as the Lerp/Correction curve minus the starting position, and plays at the same speed as the animation. Then start the timeline.
		{
			float minTime, maxTime;// minTime is Dummy
			MantleParams.PositionCorrectionCurve->GetTimeRange(minTime, maxTime);
			float newLength = maxTime - MantleParams.StartingPosition;

			MantleTimeline->SetTimelineLength(newLength);
			MantleTimeline->SetPlayRate(MantleParams.PlayRate);
			MantleTimeline->PlayFromStart();
		}
		// Step 7: Play the Anim Montage if valid.
		{
			if (!MantleParams.AnimMontage)
			{
				UE_LOG(LogTemp, Warning, TEXT("MantleParams.AnimMontage is null"));
				return;
			}
			CharacterRef->GetMesh()->GetAnimInstance()->Montage_Play(
				MantleParams.AnimMontage,
				MantleParams.PlayRate,
				EMontagePlayReturnType::MontageLength,
				MantleParams.StartingPosition
			);
		}
	}
}

bool UMantleSystem::MantleCheck(FMantleTraceSettings ParamTraceSettings)
{
	if (!CharacterMovement || !CapsuleComponent || !CharacterRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMantleSystem::MantleCheck Is NULL"));
		return false;
	}
	float mantleHeight;
	FVector initialTraceImpactPoint;
	FVector initialTraceNormal;
	FVector downTraceLocation;
	FTransform targetTransform;
	UPrimitiveComponent* hitComponent;
	EMantleType mantleType;
	// Step 1: Trace forward to find a wall / object the character cannot walk on.
	{
		float ZOffset = (ParamTraceSettings.MinLedgeHeight + ParamTraceSettings.MaxLedgeHeight) / 2.0f;
		FVector StartLocation = GetCapsuleBaseLocation(2.0f) + (GetPlayerMovementInput() * -30.0f) + FVector(0.0f, 0.0f, ZOffset);
		FVector EndLocation = StartLocation + GetPlayerMovementInput() * ParamTraceSettings.ReachDistance;

		FHitResult HitResult;
		if (UKismetSystemLibrary::CapsuleTraceSingle(
			this,
			StartLocation,
			EndLocation,
			ParamTraceSettings.ForwardTraceRadius,
			(ParamTraceSettings.MaxLedgeHeight - ParamTraceSettings.MinLedgeHeight) / 2.0f + 1.0f,
			TraceChannel,
			false,
			TArray<AActor*>(),
			GetTraceDebugType(DebugType),
			HitResult,
			true,
			FLinearColor::Red,
			FLinearColor::Green,
			1.0f
		))
		{
			bool bWalkable = CharacterMovement->IsWalkable(HitResult);
			bool bBlockingHit = HitResult.bBlockingHit;
			bool bInitialOverlap = !HitResult.bStartPenetrating; // InitialOverlap
			if (bWalkable && bBlockingHit && bInitialOverlap)
			{
				initialTraceImpactPoint = HitResult.ImpactPoint;
				initialTraceNormal = HitResult.ImpactNormal;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("UMantleSystem::MantleCheck bWalkable && bBlockingHit && bInitialOverlap Is False"));
				return false;
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("UMantleSystem::MantleCheck CapsuleTraceSingle Is Failed"));
			return false;
		}
	}
	// Step 2: Trace downward from the first trace's Impact Point and determine if the hit location is walkable.
	{
		FVector EndLocation = FVector(initialTraceImpactPoint.X, initialTraceImpactPoint.Y, GetCapsuleBaseLocation(2.0f).Z) + initialTraceNormal * (-15.0f);
		FVector StartLocation = EndLocation + FVector(0.0f, 0.0f, ParamTraceSettings.MaxLedgeHeight + ParamTraceSettings.DownwardTraceRadius + 1.0f);
		FHitResult HitResult;

		if (UKismetSystemLibrary::SphereTraceSingle(
			this,
			StartLocation,
			EndLocation,
			ParamTraceSettings.DownwardTraceRadius,
			TraceChannel,
			false,
			TArray<AActor*>(),
			GetTraceDebugType(DebugType),
			HitResult,
			true,
			FLinearColor::Yellow,
			FLinearColor::Red,
			1.0f
		))
		{
			bool bWalkable = CharacterMovement->IsWalkable(HitResult);
			bool bBlockingHit = HitResult.bBlockingHit;
			if (bWalkable && bBlockingHit)
			{
				downTraceLocation = FVector(HitResult.Location.X, HitResult.Location.Y, HitResult.ImpactPoint.Z);
				hitComponent = HitResult.GetComponent();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("UMantleSystem::MantleCheck bWalkable && bBlockingHit Is False"));
				return false;
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("UMantleSystem::MantleCheck SphereTraceSingle Is Failed"));
			return false;
		}
	}
	// Step 3: Check if the capsule has room to stand at the downward trace's location. If so, set that location as the Target Transform and calculate the mantle height.
	{
		FVector TargetLocation = GetCapsuleLocationFromBase(downTraceLocation, 2.0f);
		if (CapsuleHasRoomCheck(CapsuleComponent, TargetLocation, 0.0f, 0.0f, GetTraceDebugType(DebugType)))
		{
			FVector NormalizedDirection = initialTraceNormal * FVector(-1.0f, -1.0f, 0.0f);
			FRotator TargetRotation = UKismetMathLibrary::MakeRotFromX(NormalizedDirection);

			targetTransform = FTransform(
				TargetRotation,
				TargetLocation,
				FVector(1.0f, 1.0f, 1.0f)
			);
			FVector HeightDifference = TargetLocation - CharacterRef->GetActorLocation();
			mantleHeight = HeightDifference.Z;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("UMantleSystem::MantleCheck CapsuleHasRoomCheck Is Failed"));
			return false;
		}
	}
	// Step 4: Determine the Mantle Type by checking the movement mode and Mantle Height.
	{
		if (!CharacterMovement->IsFalling())
		{
			if (mantleHeight > 125.0f)
			{
				mantleType = EMantleType::HighMantle;
			}
			else
			{
				mantleType = EMantleType::LowMantle;
			}
		}
		else
		{
			mantleType = EMantleType::FallingMantle;
		}
	}
	// Step 5: If everything checks out, start the Mantle
	FMantleComponentAndTransform mantleLedgeWS = FMantleComponentAndTransform(
		hitComponent,
		targetTransform
	);
	MantleStart(mantleHeight, mantleLedgeWS, mantleType);
	return true;
}

bool UMantleSystem::MantleGroundCheck()
{
	return MantleCheck(GroundedTraceSettings);
}

bool UMantleSystem::MantleFallingCheck()
{
	return MantleCheck(FallingTraceSettings);
}

void UMantleSystem::SetHandType(EMantleHandType ParamHandType)
{
	HandType = ParamHandType;
}

void UMantleSystem::SetAnimType(EMantleAnimType ParamAnimType)
{
	MantleAnimType = ParamAnimType;
	MainAnimInst = CharacterRef->GetMesh()->GetAnimInstance();
}

/// <summary>
/// Update the Actors Location and Rotation as well as the Target Rotation variable to keep everything in sync.
/// </summary>
/// <param name="NewLocation">The new location to move the actor to</param>
/// <param name="NewRotation">The new rotation to set the actor to</param>
/// <param name="bSweep">Whether to sweep to the destination location, triggering overlaps along the way and stopping short if blocked</param>
/// <param name="bTeleport">Whether to teleport the physics state (if physics collision is enabled)</param>
/// <param name="SweepHitResult">If swept, hit result will contain sweep result up to first blocking hit</param>
/// <returns>Returns true if the location and rotation were successfully set</returns>
bool UMantleSystem::SetActorLocationAndRotation(FVector NewLocation, FRotator NewRotation, bool bSweep, bool bTeleport, FHitResult& SweepHitResult)
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMantleSystem::SetActorLocationAndRotation Is NULL"));
		return false;
	}
	
	// 캐릭터의 위치와 회전을 업데이트하고 결과를 반환
	bool ReturnValue = CharacterRef->SetActorLocationAndRotation(
		NewLocation,      // 새로운 위치
		NewRotation,      // 새로운 회전
		bSweep,          // 스윕 체크 여부
		&SweepHitResult,  // 스윕 결과
		bTeleport ?		// 텔레포트 여부
		ETeleportType::TeleportPhysics : ETeleportType::None        
	);
	
	
	if (DebugType != EDrawDebugTrace::None)
	{
		FString DebugMessage = FString::Printf(TEXT("Location: %s, Rotation: %s, Success: %s"), 
			*NewLocation.ToString(), 
			*NewRotation.ToString(), 
			ReturnValue ? TEXT("True") : TEXT("False"));
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, DebugMessage);
	}
	
	return ReturnValue;
}

TEnumAsByte<EDrawDebugTrace::Type> UMantleSystem::GetTraceDebugType(TEnumAsByte<EDrawDebugTrace::Type> ShowTraceType)
{
	return ShowTraceType;
}

void UMantleSystem::GetControlForwardRightVector(FVector& ForwardVector, FVector& RightVector)
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMantleSystem::GetControlForwardRightVector Is NULL"));
		ForwardVector = FVector(0.0f, 0.0f, 0.0f);
		RightVector = FVector(0.0f, 0.0f, 0.0f);
		return;
	}

	float CharacterRotYaw = CharacterRef->GetControlRotation().Yaw;
	ForwardVector = FVector(0.0f, 0.0f, CharacterRotYaw);
	RightVector = FVector(0.0f, 0.0f, CharacterRotYaw);
}

FVector UMantleSystem::GetCapsuleBaseLocation(float ZOffset)
{
	if (!CapsuleComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMantleSystem::GetCapsuleBaseLocation Is NULL"));
		return FVector::ZeroVector; 
	}
	
	FVector WorldLocation = CapsuleComponent->GetComponentLocation();
	FVector UpVector = CapsuleComponent->GetUpVector();
	// 캡슐의 절반 높이 + ZOffset만큼 아래로 이동
	FVector BaseLocation = WorldLocation - (UpVector * (CapsuleComponent->GetScaledCapsuleHalfHeight() + ZOffset));
	
	return BaseLocation;
}

FVector UMantleSystem::GetCapsuleLocationFromBase(FVector BaseLocation, float ZOffset)
{
	if (!CapsuleComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMantleSystem::GetCapsuleLocationFromBase Is NULL"));
		return FVector::ZeroVector;
	}
	float OffsetSum = CapsuleComponent->GetScaledCapsuleHalfHeight() + ZOffset;
	return BaseLocation + FVector(0.0f, 0.0f, OffsetSum);
}

FVector UMantleSystem::GetPlayerMovementInput()
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMantleSystem::GetPlayerMovementInput Is NULL"));
		return FVector(0.0f, 0.0f, 0.0f);
	}
	FRotator RotSum = FRotator(CharacterRef->GetActorRotation().Pitch, CharacterRef->GetActorRotation().Yaw + GetLastDirection(), CharacterRef->GetActorRotation().Roll);
	FVector ReturnValue = RotSum.Vector();
	return ReturnValue;
}

float UMantleSystem::GetLastDirection()
{
	if (!MainAnimInst)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMantleSystem::GetLastDirection Is NULL"));
		return 0.0f;
	}
	return MainAnimInst->CalculateDirection(CharacterRef->GetActorForwardVector(), CharacterRef->GetActorRotation());
}

