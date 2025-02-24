// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPSTemplateCharacter.h"
#include "Public/Weapon_AssultRifle.h"
#include "Public/AWeapon_Handgun.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "WeaponSystem.h"
#include "HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Animation/LocomotionAnimInstance.h"
#include "Animation/MantleSystem.h"
#include "WeaponDataAsset.h"
#include "./Public/Weapon_AssultRifle.h"
#include "./Public/AWeapon_Handgun.h"
#include "./Public/Weapon/DA_Rifle.h"
#include "./Public/Weapon/DA_Pistol.h"
#include "./Public/Weapon/Interactor.h"
#include "./Public/Weapon/IWeaponPickup.h"
#include "./Public/Widget/W_DynamicWeaponHUD.h"
#include "Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ATPSTemplateCharacter


ATPSTemplateCharacter::ATPSTemplateCharacter()
{
	// Capsule Component Settings
	// Set the default collision capsule size (radius, height)
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Controller Rotation Settings
	// Configure controller rotation to not directly affect the character
	bUseControllerRotationPitch = false;  // Disable pitch rotation
	bUseControllerRotationYaw = false;     // Disable yaw rotation
	bUseControllerRotationRoll = false;   // Disable roll rotation

	// Character Movement Settings
	// Configure character movement and rotation properties
	GetCharacterMovement()->bOrientRotationToMovement = true;     // Rotate character towards movement direction
	GetCharacterMovement()->bUseControllerDesiredRotation = false;// Don't use controller rotation
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 540.0f);  // Rotation speed
	GetCharacterMovement()->JumpZVelocity = 400.f;               // Jump strength
	GetCharacterMovement()->AirControl = 0.35f;                  // Air control
	GetCharacterMovement()->MaxWalkSpeed = 420.f;                // Maximum movement speed
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;           // Minimum movement speed
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f; // Deceleration when stopping
	GetCharacterMovement()->MaxAcceleration = 1000.0f;           // Maximum acceleration
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true; // Allow ledge movement while crouching

	// Navigation Settings
	// Configure agent properties for AI navigation system
	GetCharacterMovement()->NavAgentProps.AgentRadius = 42.0f;            // Agent radius
	GetCharacterMovement()->NavAgentProps.AgentHeight = 192.0f;          // Agent height
	GetCharacterMovement()->NavAgentProps.AgentStepHeight = -1.0f;       // Agent step height
	GetCharacterMovement()->NavAgentProps.NavWalkingSearchHeightScale = 0.5f;  // Path search height scale

	// Component Creation
	// Create various components needed for gameplay
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Primary = CreateDefaultSubobject<USceneComponent>(TEXT("Primary"));
	PrimaryChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("PrimaryChild"));
	Handgun = CreateDefaultSubobject<USceneComponent>(TEXT("Handgun"));
	HandgunChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("HandgunChild"));
	WeaponSystem = CreateDefaultSubobject<UWeaponSystem>(TEXT("WeaponSystem"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	InteractorComponent = CreateDefaultSubobject<UInteractor>(TEXT("Interactor Component"));
	MantleComponent = CreateDefaultSubobject<UMantleSystem>(TEXT("Mantle Component"));

	// Component Hierarchy Setup
	// Set up parent-child relationships for components
	CameraBoom->SetupAttachment(RootComponent);
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Primary->SetupAttachment(RootComponent);
	Handgun->SetupAttachment(RootComponent);
	PrimaryChild->SetupAttachment(Primary);
	HandgunChild->SetupAttachment(Handgun);

	// Weapon Class Setup
	if (PrimaryChild)
	{
		PrimaryChild->SetChildActorClass(AWeapon_AssultRifle::StaticClass());
	}
	if (HandgunChild)
	{
		HandgunChild->SetChildActorClass(AAWeapon_Handgun::StaticClass());
	}

	// Camera Boom Settings
	// Configure spring arm for third-person camera system
	CameraBoom->TargetArmLength = 350.0f;                        // Camera distance
	CameraBoom->bUsePawnControlRotation = true;                  // Controller-based rotation
	CameraBoom->SocketOffset = FVector(0.0f, 80.0f, 40.0f);      // Socket offset
	CameraBoom->bInheritRoll = false;                           // Disable roll rotation inheritance
	CameraBoom->bEnableCameraLag = true;                        // Enable camera lag effect
	CameraBoom->bEnableCameraRotationLag = true;                // Enable camera rotation lag
	CameraBoom->CameraLagSpeed = 30.0f;                         // Camera lag speed
	CameraBoom->CameraRotationLagSpeed = 30.0f;                 // Camera rotation lag speed
	CameraBoom->CameraLagMaxDistance = 5.0f;                    // Maximum lag distance
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f)); // Relative location
	CameraBoom->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f)); // Relative rotation
	CameraBoom->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));   // Relative scale

	// Follow Camera Settings
	// Configure the actual camera component
	FollowCamera->bUsePawnControlRotation = false;               // Independent from pawn rotation

	ShoulderYOffset = 50.0f;
	ShoulderZOffset = 20.0f;
	TargetArmLengths = FVector(200.0f, 100.0f, 0.0f);
}

void ATPSTemplateCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CrouchTimeline.TickTimeline(DeltaTime);
	AimTimeline.TickTimeline(DeltaTime);
	ShoulderCameraTimeline.TickTimeline(DeltaTime);

	UpdateCrouchHeight();
}

void ATPSTemplateCharacter::OnLanded(const FHitResult& Hit)
{
	Super::OnLanded(Hit);

	ImpactOnLand();
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATPSTemplateCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATPSTemplateCharacter::Jumping);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::Look);

		// Switch Weapons
		EnhancedInputComponent->BindAction(SwitchWeaponsAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::SwitchWeapons);

		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::ShootFire);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::Aim);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::Reload);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATPSTemplateCharacter::SprintCompleted);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ATPSTemplateCharacter::ToggleCrouch);
		EnhancedInputComponent->BindAction(CameraChangeAction, ETriggerEvent::Started, this, &ATPSTemplateCharacter::FlipFlapCameraChange);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::Dodge);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATPSTemplateCharacter::BeginPlay()
{
	Super::BeginPlay();
	InteractorComponent->CharacterRef = this;

	if (InteractorComponent)
	{
		InteractorComponent->DetectionDistance = 350.0f;
		InteractorComponent->InteractionMethod = EInteractionMethod::Camera;
		InteractorComponent->HasInteraction = false;
		InteractorComponent->InteractorActive = true;
	}
}

void ATPSTemplateCharacter::Die()
{
	if (Dead)
		return;

	FVector InteractionActorLocation = GetCapsuleComponent()->GetComponentLocation();

	FTransform InteractionSpawnTransform;
	InteractionSpawnTransform.SetLocation(FVector(InteractionActorLocation.X, InteractionActorLocation.Y, InteractionActorLocation.Z + 20.0f));
	InteractionSpawnTransform.SetRotation(FQuat::Identity);
	InteractionSpawnTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

	Dead = true;
	InteractorComponent->DestroyComponent();

	if (AMasterWeapon* PrimaryWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor()))
	{
		GetWorld()->SpawnActor<AActor>(PrimaryWeapon->WeaponPickupClass, InteractionSpawnTransform);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Die()::PrimaryWeapon Is Null"));
	}
	if (AMasterWeapon* SecondaryWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor()))
	{
		GetWorld()->SpawnActor<AActor>(SecondaryWeapon->WeaponPickupClass, InteractionSpawnTransform);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Die()::SecondaryWeapon Is Null"));
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

void ATPSTemplateCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		TurnRate = FMath::Clamp(MovementVector.X, -1.0f, 1.0f);
		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
		DodgeForward = MovementVector.Y;
		DodgeRight = MovementVector.X;
	}
}

void ATPSTemplateCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ATPSTemplateCharacter::SwitchWeapons()
{
	//SwitchToPrimaryWeapon();
}

void ATPSTemplateCharacter::SwitchToPrimaryWeapon()
{
	if (!CanSwitchWeapon() || IsPrimaryEquip)
		return;

	bCanSwitchWeapon = false;
	IsPistolEquip = false;

	LocomotionBP->LeftHandIKOffset = WeaponSystem->RifleData->LeftHandIKOffset;

	WeaponSystem->Pistol_State(WeaponSystem->PistolData->WeaponClass, EAnimationState::Pistol, EWeaponState::Unequip, FName(""), FName("PistolHost_Socket"));
	IsPrimaryEquip = true;

	WeaponSystem->Rifle_State(WeaponSystem->RifleData->WeaponClass, EAnimationState::RifleShotgun, EWeaponState::Equip, FName("Rifle_Socket"), FName(""));
	{
		AMasterWeapon* MasterWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
	}
	// Play Montage with Delay
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimInstance is NULL"));
		return;
	}

	if (AnimInstance)
	{
		UAnimMontage* RifleEquipMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Rifle/Montages/MM_Rifle_Equip1")));
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("RifleEquipMontage %s"));
		// Play Montage
		AnimInstance->Montage_Play(RifleEquipMontage, 1.0f);

		// Set Montage End Delegate
		FOnMontageEnded CompleteDelegate;
		CompleteDelegate.BindUObject(this, &ATPSTemplateCharacter::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(CompleteDelegate, RifleEquipMontage);
	}
}

void ATPSTemplateCharacter::SwitchToHandgunWeapon()
{
	if (!CanSwitchWeapon() || IsPistolEquip)
		return;

	bCanSwitchWeapon = false;
	IsPrimaryEquip = false;

	LocomotionBP->LeftHandIKOffset = WeaponSystem->PistolData->LeftHandIKOffset;

	WeaponSystem->Rifle_State(WeaponSystem->RifleData->WeaponClass, EAnimationState::RifleShotgun, EWeaponState::Unequip, FName(""), FName("RifleHost_Socket"));
	IsPistolEquip = true;

	WeaponSystem->Pistol_State(WeaponSystem->PistolData->WeaponClass, EAnimationState::Pistol, EWeaponState::Equip, FName("Pistol_Socket"), FName(""));

	// Play Montage with Delay
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimInstance is NULL"));
		return;
	}

	if (AnimInstance)
	{
		UAnimMontage* PistolEquipMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Pistol/Montages/MM_Pistol_Equip2")));
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("RifleEquipMontage %s"));
		// Play Montage
		AnimInstance->Montage_Play(PistolEquipMontage, 1.0f);

		// Set Montage End Delegate
		FOnMontageEnded CompleteDelegate;
		CompleteDelegate.BindUObject(this, &ATPSTemplateCharacter::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(CompleteDelegate, PistolEquipMontage);
	}
}

bool ATPSTemplateCharacter::CanSwitchWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("bCanSwitchWeapon: %d, bInteracting: %d"), bCanSwitchWeapon, bInteracting);
	return bCanSwitchWeapon && !bInteracting;
}

void ATPSTemplateCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// After Montage Ended
	if (!bInterrupted)
	{
		bCanSwitchWeapon = true;
	}
}

void ATPSTemplateCharacter::ShootFire(const FInputActionValue& Value)
{
	if (!IsAim)
		return;
	bFiring = Value.Get<bool>();
	
    HandleFiring();
}

void ATPSTemplateCharacter::Aim(const FInputActionValue& Value)
{
	if (!IsPrimaryEquip && !IsPistolEquip)
		return;
	IsAim = Value.Get<bool>();

	if (IsAim)
	{
		bUseControllerRotationYaw = true;
		AimTimeline.Play();
	}
	else
	{
		bUseControllerRotationYaw = false; 
		AimTimeline.Reverse();
	}
}

void ATPSTemplateCharacter::Reload()
{
    AMasterWeapon* MasterWeapon = nullptr;  // nullptr로 초기화

    if (IsPrimaryEquip)
    {
        MasterWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
    }
    else if (IsPistolEquip)
    {
        MasterWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
    }
    else
    {
        return; // 무기를 들고 있지 않은 경우
    }

    if (!MasterWeapon)
        return;

    MasterWeapon->Reload();
}

void ATPSTemplateCharacter::Sprint(const FInputActionValue& Value)
{
	if (!GetCharacterMovement()->IsCrouching() && !bInteracting)
	{
		IsSprint = true;
		GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	}
}

void ATPSTemplateCharacter::SprintCompleted(const FInputActionValue& Value)
{
	IsSprint = false;  // 스프린트 종료
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
}

bool ATPSTemplateCharacter::CanFire()
{
	
	bool bCanShoot = !IsSprint && !IsDodging && CanJump() && bCanSwitchWeapon;
	if (bCanShoot || IsCrouch)
		return true;
	else
		return false;
}


void ATPSTemplateCharacter::StopFire()
{
    bFiring = false;
	bUseControllerRotationYaw = false;
}

void ATPSTemplateCharacter::HandleFiring()
{
    if (!bFiring)
        return;

    if (!bCanFire)
        return;

    if (!CanFire())
        return;

    if (IsPrimaryEquip)
    {
		AMasterWeapon* MasterWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
		UWeaponDataAsset* CurrentWeaponDataAsset = WeaponSystem->RifleData;
		ReadyToFire(MasterWeapon, CurrentWeaponDataAsset);
    }
	else if(IsPistolEquip)
	{
		AMasterWeapon* MasterWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
		UWeaponDataAsset* CurrentWeaponDataAsset = WeaponSystem->PistolData;
		ReadyToFire(MasterWeapon, CurrentWeaponDataAsset);
	}
}

void ATPSTemplateCharacter::ReadyToFire(AMasterWeapon* MasterWeapon, UWeaponDataAsset* CurrentWeaponDataAsset)
{
	if (!MasterWeapon || !CurrentWeaponDataAsset)
		return;
	
	bCanFire = false;
	MasterWeapon->Fire();

	// Process next shot based on fire mode
	float FireDelay = CurrentWeaponDataAsset->FireRate;
	EFireMode CurrentFireMode = CurrentWeaponDataAsset->FireMode;
	FTimerHandle TimerHandle;

	switch (CurrentFireMode)
	{
	case EFireMode::FullAuto:
		// Auto-fire is scheduled with a timer
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				bCanFire = true;
				HandleFiring(); // Recursive firing process
				//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Fire!!!!!!!!!!!!!!!!!!!!!!!!!"));
			},
			FireDelay,
			false
		);
		break;
	default:
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				bCanFire = true;
			},
			FireDelay,
			false
		);
		break;
	}
}

void ATPSTemplateCharacter::ToggleCrouch(const FInputActionValue& Value)
{
    if (!bInteracting)  // 다른 상호작용 중이 아닐 때만
    {
        if (!IsCrouch)
        {
			IsCrouch = true;
			Crouch();
			CrouchTimeline.Play();
        }
        else
        {
			IsCrouch = false;
			UnCrouch();
			CrouchTimeline.Reverse();  
        }
    }
}

void ATPSTemplateCharacter::UpdateAimView(float Value)
{
	CameraBoom->TargetArmLength = FMath::Lerp(TargetArmLengths.X, TargetArmLengths.Y, Value);
	
	CameraBoom->SocketOffset = FMath::Lerp(FVector(0.0f, CameraBoom->SocketOffset.Y, ShoulderZOffset), 
		FVector(0.0f, CameraBoom->SocketOffset.Y, ShoulderZOffset), 
		Value);
}

void ATPSTemplateCharacter::PlayDodgeMontage(UAnimMontage* MontageToPlay)
{
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        AnimInstance->Montage_Play(MontageToPlay);
		IsDodging = true;

        FOnMontageEnded CompleteDelegate;
        CompleteDelegate.BindUObject(this, &ATPSTemplateCharacter::OnDodgeMontageEnded);
        AnimInstance->Montage_SetEndDelegate(CompleteDelegate, MontageToPlay);

        FOnMontageBlendingOutStarted BlendOutDelegate;
        BlendOutDelegate.BindUObject(this, &ATPSTemplateCharacter::OnDodgeMontageInterrupted);
        AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, MontageToPlay);
    }
}

void ATPSTemplateCharacter::OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // 몽타주가 정상적으로 끝났을 때의 처리
    if (!bInterrupted)
    {
        IsDodging = false;
    }
}

void ATPSTemplateCharacter::OnDodgeMontageInterrupted(UAnimMontage* Montage, bool bInterrupted)
{
    IsDodging = false;
}

void ATPSTemplateCharacter::ImpactOnLand()
{
	float AbsVelocity = FMath::Abs(GetCharacterMovement()->Velocity.Z);
	if (AbsVelocity >= 300.0f && AbsVelocity <= 900.0f)
	{
		CurrentLandState = ELandState::Soft;
	}
	
	if (AbsVelocity >= 1000.0f && AbsVelocity <= 1200.0f)
	{
		CurrentLandState = ELandState::Normal;
	}

	if (AbsVelocity > 1250.f)
	{
		CurrentLandState = ELandState::Hard;
	}
}

void ATPSTemplateCharacter::Dodge()
{
	if (bInteracting || GetCharacterMovement()->IsFalling())
		return;

	if (DodgeForward != 0.0f)
	{
		UAnimMontage* MontageToPlay = nullptr;
		if (DodgeForward > 0.0f)
		{
			MontageToPlay = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, TEXT("/Game/ThirdPerson/Blueprints/Animation/Dodge/Montage/DiveRoll_F_Montage")));
		}
		else
		{
			MontageToPlay = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, TEXT("/Game/ThirdPerson/Blueprints/Animation/Dodge/Montage/DiveRoll_B_Montage")));
		}

		PlayDodgeMontage(MontageToPlay);
	}

	if (DodgeRight != 0.0f)
	{
		UAnimMontage* MontageToPlay = nullptr;
		if (DodgeRight > 0.0f)
		{
			MontageToPlay = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, TEXT("/Game/ThirdPerson/Blueprints/Animation/Dodge/Montage/DiveRoll_R_Montage")));
		}
		else
		{
			MontageToPlay = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, TEXT("/Game/ThirdPerson/Blueprints/Animation/Dodge/Montage/DiveRoll_L_Montage")));
		}

		PlayDodgeMontage(MontageToPlay);
	}
}

void ATPSTemplateCharacter::Jumping()
{
	if (bInteracting || !CanJump())
		return;
	FTimerHandle TimerHandle;
	if (MantleComponent->MantleGroundCheck())
	{
		bUseControllerRotationYaw = false;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				bUseControllerRotationYaw = true;
			},
			2.0f,
			false
		);
	}
	else
	{
		bInteracting = true;
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Called"));
		Jump();
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				bInteracting = false;
			},
			1.0f,
			false
		);
	}
}

void ATPSTemplateCharacter::Interact()
{
	
}

void ATPSTemplateCharacter::FlipFlapCameraChange()
{
	if (RightShoulder)
		return;
	static bool FlipFlap = false;
	FlipFlap = !FlipFlap;  // 현재 값의 반대로 설정

	if (FlipFlap)
	{
		ShoulderCameraTimeline.Play();
	}
	else
	{
		ShoulderCameraTimeline.Reverse();
	}
}

void ATPSTemplateCharacter::ShoulderCameraChange(float Value)
{
	CameraBoom->SocketOffset = FMath::Lerp(
		FVector(0.0f, ShoulderYOffset, ShoulderZOffset),
		FVector(0.0f, ShoulderYOffset * -1.0f, ShoulderZOffset), 
		Value);
}

void ATPSTemplateCharacter::UpdateCrouchHeight()
{
	if (IsCrouch)
	{
		GetCapsuleComponent()->SetCapsuleRadius(33.0f);
	}
	else
	{
		GetCapsuleComponent()->SetCapsuleRadius(35.0f);
	}
}
