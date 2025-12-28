// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/LocomotionAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/TPSTemplateCharacter.h"
#include "Characters/Player_Base.h"
#include "Components/EquipmentSystem.h"

void ULocomotionAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Get owning actor and cast to TPSTemplateCharacter
    if (AActor* OwningActor = GetOwningActor())
    {
        CharacterRef = Cast<ATPSTemplateCharacter>(OwningActor);
        if (CharacterRef)
        {
            //CharacterRef->LocomotionBP = this;
            // Get the movement component
            MovementComponent = CharacterRef->GetCharacterMovement();
        }
    }
}

void ULocomotionAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!IsValid())
        return;

    UpdateCharacterState();
    /*
        Set velocity and ground speed from the movement components velocity.
        Ground speed is calculated from only the X and Y axis of the velocity,
        so moving up or down does not affect it.
    */
    Velocity = MovementComponent->Velocity;
    GroundSpeed = FVector2D(Velocity.X, Velocity.Y).Length();
    /*
        Set Should Move to true only if ground speed is above a small threshold
        (to prevent incredibly small velocities from triggering animations) and
        if there is currently acceleration (input) applied.
    */
    Acceleration = MovementComponent->GetCurrentAcceleration();
    bShouldMove = (GroundSpeed > 3.0f) && (Acceleration != FVector(0.0f, 0.0f, 0.0f));
    /*
        Set Is Falling from the movement components falling state.
    */
    bIsFalling = MovementComponent->IsFalling();

    // Sequence 4
    UpdateLocomotionDirection();
}

void ULocomotionAnimInstance::UpdateCharacterState()
{
    UpdateAcceleration();
    UpdateWallDetection();
    RunningIntoWall();

    // Player-specific properties need to be accessed from APlayer_Base
    if (APlayer_Base* PlayerRef = Cast<APlayer_Base>(CharacterRef))
    {
        bool bIsPistolEquipped = PlayerRef->GetCurWeaponSlot() == EWeaponSlot::Handgun;
        bool bIsPrimaryEquipped = PlayerRef->GetCurWeaponSlot() == EWeaponSlot::Primary;
        bIsCrouching = PlayerRef->IsCrouch;
        bIsSprint = PlayerRef->IsSprint;
        LandState = PlayerRef->CurrentLandState;
        bIsJump = PlayerRef->IsJump;
        bIsAim = PlayerRef->IsAim;
        Pitch = (PlayerRef->GetBaseAimRotation() - PlayerRef->GetActorRotation()).GetNormalized().Pitch;
        bIsPistolEquip = bIsPistolEquipped;
        bIsRifleEquip = bIsPrimaryEquipped;
        DirectionAngle = FMath::FInterpTo(DirectionAngle, PlayerRef->TurnRate, GetDeltaSeconds(), 0.0f);
    }

    // Base character properties (available for all characters)
    AnimationState = CharacterRef->CurrentAnimationState;
}

void ULocomotionAnimInstance::RunningIntoWall()
{
    if (bRunningIntoWall)
    {
        if (bRunningIntoWall)
        {
            bIsWall = true;
            MovementComponent->MaxWalkSpeed = 0.0f;
        }
    }
    else
    {
        bIsWall = false;
        if (bIsSprint)
        {
            MovementComponent->MaxWalkSpeed = 600.0f;
        }
        else
        {
            MovementComponent->MaxWalkSpeed = 300.0f;
        }
    }
}

void ULocomotionAnimInstance::UpdateWallDetection()
{
    bool isValidLocalAcc = LocalAcceleration2D.Size2D() > 0.1f;
    bool isValidLocalVelocity = LocalVelocity2D.Size2D() > 200.0f;
    bool isRange = FVector::DotProduct(
        LocalAcceleration2D.GetSafeNormal(0.0001f),
        LocalVelocity2D.GetSafeNormal(0.0001f)
    ) >= -0.6f &&
        FVector::DotProduct(
            LocalAcceleration2D.GetSafeNormal(0.0001f),
            LocalVelocity2D.GetSafeNormal(0.0001f)
        ) <= 0.6f;

    bRunningIntoWall = isValidLocalAcc && isValidLocalVelocity && isRange;
}

void ULocomotionAnimInstance::UpdateAcceleration()
{
    // TODO: Vector Length XY Squared
    float AccelerationLength = LocalAcceleration2D.SizeSquared2D();
    bHasAcceleration = !FMath::IsNearlyEqual(AccelerationLength, 0.0f, 0.000001f);

    WorldAcceleration2D = MovementComponent->GetCurrentAcceleration() * FVector(1.0f, 1.0f, 0.0f);
    LocalAcceleration2D = WorldRotation.UnrotateVector(WorldAcceleration2D);
    WorldVelocity2D = WorldVelocity * FVector(1.0f, 1.0f, 0.0f);
    LocalVelocity2D = WorldRotation.UnrotateVector(WorldVelocity2D);

    WorldRotation = GetOwningActor()->GetActorRotation();
    WorldVelocity = GetOwningActor()->GetVelocity();
}

void ULocomotionAnimInstance::UpdateLocomotionDirection()
{
    FVector VelocityXY = FVector(Velocity.X, Velocity.Y, 0.0f);
    FRotator ActorRotation = CharacterRef->GetActorRotation();

    // Normalize the direction to -180 to 180 range
    Direction = FRotator::NormalizeAxis(CalculateDirection(VelocityXY, ActorRotation));
    // To Set Movement State;
    if (Direction >= -70.0f && Direction <= 70.0f)
    {
        MovementInput = EMovementState::Forward;
    }
    else if (Direction > 70.0f && Direction <= 110.0f)
    {
        MovementInput = EMovementState::Right;
    }
    else if (Direction >= -110.0f && Direction <= -70.0f)
    {
        MovementInput = EMovementState::Backward;
    }
    else
    {
        MovementInput = EMovementState::Left;
    }

    FOrientationAngle = Direction;
    ROrientationAngle = Direction - 90;
    BOrientationAngle = Direction - 180;
    LOrientationAngle = Direction + 90;

    // Turn in Place
    TurnInPlace();

}

void ULocomotionAnimInstance::TurnInPlace()
{
    if (bShouldMove || bIsFalling)
    {
        // TODO: RootYawOffset Default Value
        RootYawOffset = FMath::FInterpTo(RootYawOffset, 0.0f, GetDeltaSeconds(), 20.0f);
        MovingRotation = CharacterRef->GetActorRotation();
        LastMovingRotation = MovingRotation;
    }
    else
    {
        LastMovingRotation = MovingRotation;
        MovingRotation = CharacterRef->GetActorRotation();
        // Delta(Rotator)
        RootYawOffset = RootYawOffset - (MovingRotation - LastMovingRotation).GetNormalized().Yaw;

        // TODO: Curve Name?
        if (GetCurveValue(FName("IsTurn")) > 0.0f)
        {
            LastDistanceCurve = DistanceCurve;

            DistanceCurve = GetCurveValue(FName("DistanceCurve"));
            DeltaDistanceCurve = DistanceCurve - LastDistanceCurve;
            if (RootYawOffset > 0.0f)
            {
                RootYawOffset = RootYawOffset - DeltaDistanceCurve;
            }
            else
            {
                RootYawOffset = RootYawOffset + DeltaDistanceCurve;
            }
            AbsRootYawOffset = FMath::Abs(RootYawOffset);
            if (AbsRootYawOffset > 45.0f)
            {
                YawExcess = AbsRootYawOffset - 45.0f;
                if (RootYawOffset > 0.0f)
                {
                    RootYawOffset = RootYawOffset - YawExcess;
                }
                else
                {
                    RootYawOffset = RootYawOffset + YawExcess;
                }
            }
        }
    }
}