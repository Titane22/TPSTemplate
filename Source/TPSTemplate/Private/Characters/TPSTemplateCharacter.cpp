// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/TPSTemplateCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/EquipmentSystem.h"
#include "Components/HealthSystem.h"
#include "Components/InventorySystem.h"
#include "Components/WeaponSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon/Interactor.h"
#include "Weapon/Interaction.h"
#include "Weapon/MasterWeapon.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ATPSTemplateCharacter

ATPSTemplateCharacter::ATPSTemplateCharacter()
{
	// Enable Tick for timeline updates
	PrimaryActorTick.bCanEverTick = true;

	// Capsule Component Settings
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Controller Rotation Settings
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;  // Always rotate with mouse (TPS Shooter style)
	bUseControllerRotationRoll = false;

	// Character Movement Settings
	GetCharacterMovement()->bOrientRotationToMovement = false;  // Don't rotate to movement direction
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 540.0f);
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 420.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->MaxAcceleration = 1000.0f;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;

	// Navigation Settings
	GetCharacterMovement()->NavAgentProps.AgentRadius = 42.0f;
	GetCharacterMovement()->NavAgentProps.AgentHeight = 192.0f;
	GetCharacterMovement()->NavAgentProps.AgentStepHeight = -1.0f;
	GetCharacterMovement()->NavAgentProps.NavWalkingSearchHeightScale = 0.5f;

	// Component Creation
	Primary = CreateDefaultSubobject<USceneComponent>(TEXT("Primary"));
	PrimaryChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("PrimaryChild"));
	Handgun = CreateDefaultSubobject<USceneComponent>(TEXT("Handgun"));
	HandgunChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("HandgunChild"));
	EquipmentSystem = CreateDefaultSubobject<UEquipmentSystem>(TEXT("EquipmentSystem"));
	HealthComponent = CreateDefaultSubobject<UHealthSystem>(TEXT("HealthComponent"));
	InteractorComponent = CreateDefaultSubobject<UInteractor>(TEXT("Interactor Component"));
	InventorySystem = CreateDefaultSubobject<UInventorySystem>(TEXT("Inventory System"));

	// Component Hierarchy Setup
	Primary->SetupAttachment(RootComponent);
	Handgun->SetupAttachment(RootComponent);
	PrimaryChild->SetupAttachment(Primary);
	HandgunChild->SetupAttachment(Handgun);
}

void ATPSTemplateCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Timeline initialization
	if (AimCurve)
	{
		FOnTimelineFloat Callback;
		Callback.BindUFunction(this, FName("UpdateAimTimeline"));
		AimTimeline.AddInterpFloat(AimCurve, Callback);
		AimTimeline.SetPlayRate(4.0f);
	}

	if (CrouchCurve)
	{
		FOnTimelineFloat Callback;
		Callback.BindUFunction(this, FName("UpdateCrouchTimeline"));
		CrouchTimeline.AddInterpFloat(CrouchCurve, Callback);
	}

	if (EquipmentSystem)
	{
		EquipmentSystem->CharacterRef = this;
		UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] EquipmentSystem initialized successfully"), *GetName());
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("[%s] EquipmentSystem is nullptr! Check Blueprint setup."), *GetName());
	}

	// Weapon initial setup
	// NOTE: PrimaryChild and HandgunChild Child Actor Class should be set in Blueprint
	if (PrimaryChild && PrimaryChild->GetChildActor())
	{
		// Attach to back (storage state)
		PrimaryChild->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			FName("RifleHost_Socket")
		);

		// Set WeaponSystem reference
		if (AMasterWeapon* PrimaryWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor()))
		{
			if (PrimaryWeapon->WeaponSystem)
			{
				PrimaryWeapon->WeaponSystem->CharacterRef = this;
			}
		}
	}

	if (HandgunChild && HandgunChild->GetChildActor())
	{
		// Attach to hand (default state)
		HandgunChild->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			FName("PistolHost_Socket")
		);

		// Set WeaponSystem reference
		if (AMasterWeapon* HandgunWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor()))
		{
			if (HandgunWeapon->WeaponSystem)
			{
				HandgunWeapon->WeaponSystem->CharacterRef = this;
			}
		}
	}
}

void ATPSTemplateCharacter::Die()
{
	if (bIsDead)
		return;

	FVector InteractionActorLocation = GetCapsuleComponent()->GetComponentLocation();

	FTransform InteractionSpawnTransform;
	InteractionSpawnTransform.SetLocation(FVector(InteractionActorLocation.X, InteractionActorLocation.Y, InteractionActorLocation.Z + 20.0f));
	InteractionSpawnTransform.SetRotation(FQuat::Identity);
	InteractionSpawnTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

	bIsDead = true;
	InteractorComponent->DestroyComponent();

	if (AMasterWeapon* PrimaryWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor()))
	{
		GetWorld()->SpawnActor<AActor>(PrimaryWeapon->WeaponPickupClass, InteractionSpawnTransform);
	}

	if (AMasterWeapon* HandgunWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor()))
	{
		GetWorld()->SpawnActor<AActor>(HandgunWeapon->WeaponPickupClass, InteractionSpawnTransform);
	}

	// 1. Disable capsule collision and enable ragdoll physics
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);

	// 2. Disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	// 3. Detach controller from character
	if (AController* CharacterController = GetController())
	{
		CharacterController->UnPossess();
	}
}

void ATPSTemplateCharacter::StartRagdoll()
{
	// Disable Character Movement
	GetCharacterMovement()->DisableMovement();

	// Enable Ragdoll Physics on body mesh
	GetMesh()->SetSimulatePhysics(true);

	// Disable Capsule Component Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Hide Weapons
	PrimaryChild->SetVisibility(false, true);
	HandgunChild->SetVisibility(false, true);
}

void ATPSTemplateCharacter::Interact()
{
	// Base implementation - can be overridden
}

void ATPSTemplateCharacter::SwitchWeapons()
{
	// Base implementation - can be overridden
}

void ATPSTemplateCharacter::SwitchToPrimaryWeapon()
{
	// Base implementation - can be overridden
}

void ATPSTemplateCharacter::SwitchToHandgunWeapon()
{
	// Base implementation - can be overridden
}

void ATPSTemplateCharacter::ReadyToFire(AMasterWeapon* MasterWeapon, UWeaponData* CurrentWeaponDataAsset)
{
	// Base implementation - can be overridden
}

void ATPSTemplateCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimTimeline.TickTimeline(DeltaTime);
	CrouchTimeline.TickTimeline(DeltaTime);
}

//////////////////////////////////////////////////////////////////////////
// Aim Actions

void ATPSTemplateCharacter::StartAim()
{
	if (bIsAim) return;

	bIsAim = true;
	OnAimStarted();

	if (AimCurve)
	{
		AimTimeline.Play();
	}
}

void ATPSTemplateCharacter::StopAim()
{
	if (!bIsAim) return;

	bIsAim = false;
	OnAimEnded();

	if (AimCurve)
	{
		AimTimeline.Reverse();
	}
}

void ATPSTemplateCharacter::OnAimStarted()
{
	// Virtual hook - base implementation empty
}

void ATPSTemplateCharacter::OnAimEnded()
{
	// Virtual hook - base implementation empty
}

//////////////////////////////////////////////////////////////////////////
// Crouch Actions

void ATPSTemplateCharacter::StartCrouch()
{
	if (IsCrouch || bInteracting || GetCharacterMovement()->IsFalling())
		return;

	IsCrouch = true;
	Crouch();
	OnCrouchStarted();

	if (CrouchCurve)
	{
		CrouchTimeline.Play();
	}
}

void ATPSTemplateCharacter::StopCrouch()
{
	if (!IsCrouch) return;

	IsCrouch = false;
	UnCrouch();
	OnCrouchEnded();

	if (CrouchCurve)
	{
		CrouchTimeline.Reverse();
	}
}

void ATPSTemplateCharacter::OnCrouchStarted()
{
	// Virtual hook - base implementation empty
}

void ATPSTemplateCharacter::OnCrouchEnded()
{
	// Virtual hook - base implementation empty
}

//////////////////////////////////////////////////////////////////////////
// Dodge Actions

UAnimMontage* ATPSTemplateCharacter::GetDodgeMontage(float ForwardInput, float RightInput)
{
	// Priority: Forward/Backward over Left/Right
	if (FMath::Abs(ForwardInput) > 0.1f)
	{
		return (ForwardInput > 0.0f) ? DodgeMontages.Forward : DodgeMontages.Backward;
	}
	else if (FMath::Abs(RightInput) > 0.1f)
	{
		return (RightInput > 0.0f) ? DodgeMontages.Right : DodgeMontages.Left;
	}

	return nullptr;
}

void ATPSTemplateCharacter::PerformDodge(float ForwardInput, float RightInput)
{
	if (bInteracting || GetCharacterMovement()->IsFalling() || bIsDodging)
		return;

	UAnimMontage* Montage = GetDodgeMontage(ForwardInput, RightInput);
	if (Montage)
	{
		PlayDodgeMontageInternal(Montage);
	}
}

void ATPSTemplateCharacter::PlayDodgeMontageInternal(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(MontageToPlay);
	bIsDodging = true;
	OnDodgeStarted();

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ATPSTemplateCharacter::OnDodgeMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);

	FOnMontageBlendingOutStarted BlendDelegate;
	BlendDelegate.BindUObject(this, &ATPSTemplateCharacter::OnDodgeMontageInterrupted);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendDelegate, MontageToPlay);
}

void ATPSTemplateCharacter::OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		bIsDodging = false;
		OnDodgeEnded(Montage, bInterrupted);
	}
}

void ATPSTemplateCharacter::OnDodgeMontageInterrupted(UAnimMontage* Montage, bool bInterrupted)
{
	bIsDodging = false;
	OnDodgeEnded(Montage, true);
}

void ATPSTemplateCharacter::OnDodgeStarted()
{
	// Virtual hook - base implementation empty
}

void ATPSTemplateCharacter::OnDodgeEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// Virtual hook - base implementation empty
}

//////////////////////////////////////////////////////////////////////////
// Timeline Callbacks

void ATPSTemplateCharacter::UpdateAimTimeline(float Value)
{
	// Base implementation - empty (플레이어에서 카메라 로직 추가)
}

void ATPSTemplateCharacter::UpdateCrouchTimeline(float Value)
{
	// Capsule radius 조정
	if (IsCrouch)
	{
		GetCapsuleComponent()->SetCapsuleRadius(33.0f);
	}
	else
	{
		GetCapsuleComponent()->SetCapsuleRadius(35.0f);
	}
}