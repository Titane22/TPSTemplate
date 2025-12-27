// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Player_Base.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WeaponSystem.h"
#include "Components/EquipmentSystem.h"
#include "Components/HealthSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Weapon/MasterWeapon.h"
#include "InputActionValue.h"
#include "Animation/LocomotionAnimInstance.h"
#include "Animation/MantleSystem.h"
#include "Data/WeaponData.h"
#include "Widget/W_DynamicWeaponHUD.h"
#include "Widget/W_DivisionHUD.h"
#include "Library/InteractiveType.h"
#include "Weapon/Interactor.h"
#include "Weapon/IWeaponPickup.h"

APlayer_Base::APlayer_Base()
	: Super()
{
	// Create Camera Components
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	MantleComponent = CreateDefaultSubobject<UMantleSystem>(TEXT("Mantle Component"));

	// Component Hierarchy Setup
	CameraBoom->SetupAttachment(RootComponent);
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Camera Boom Settings
	CameraBoom->TargetArmLength = 350.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.0f, 80.0f, 40.0f);
	CameraBoom->bInheritRoll = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraLagSpeed = 30.0f;
	CameraBoom->CameraRotationLagSpeed = 30.0f;
	CameraBoom->CameraLagMaxDistance = 5.0f;
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
	CameraBoom->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	CameraBoom->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));

	// Follow Camera Settings
	FollowCamera->bUsePawnControlRotation = false;

	// Initialize Camera offsets
	ShoulderYOffset = 50.0f;
	ShoulderZOffset = 20.0f;
	TargetArmLengths = FVector(200.0f, 100.0f, 0.0f);
}

void APlayer_Base::BeginPlay()
{
	Super::BeginPlay();

	InteractorComponent->CharacterRef = this;
	
	// Get Animation Instance
	LocomotionBP = Cast<ULocomotionAnimInstance>(GetMesh()->GetAnimInstance());

	// Setup Timelines
	if (CrouchCurve)
	{
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindUFunction(this, FName("UpdateCrouchHeight"));
		CrouchTimeline.AddInterpFloat(CrouchCurve, TimelineCallback);
	}

	if (AimCurve)
	{
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindUFunction(this, FName("UpdateAimView"));
		AimTimeline.AddInterpFloat(AimCurve, TimelineCallback);
		AimTimeline.SetPlayRate(4.0f);
	}

	if (ShoulderCameraCurve)
	{
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindUFunction(this, FName("ShoulderCameraChange"));
		ShoulderCameraTimeline.AddInterpFloat(ShoulderCameraCurve, TimelineCallback);
		ShoulderCameraTimeline.SetPlayRate(4.0f);
	}

	// Camera Settings
	CameraBoom->SocketOffset = FVector(0.0f, ShoulderYOffset, ShoulderZOffset);
	CameraBoom->TargetArmLength = TargetArmLengths.X;

	// Create HUD Widgets
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UICrosshair)
		{
			UICrosshair->AddToViewport();
		}
	}
	CanFire();
}

void APlayer_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CrouchTimeline.TickTimeline(DeltaTime);
	AimTimeline.TickTimeline(DeltaTime);
	ShoulderCameraTimeline.TickTimeline(DeltaTime);

	UpdateCrouchHeight();
}

void APlayer_Base::OnLanded(const FHitResult& Hit)
{
	Super::OnLanded(Hit);
	ImpactOnLand();
}

void APlayer_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
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
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayer_Base::Jumping);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayer_Base::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayer_Base::Look);

		// Switch Weapons
		EnhancedInputComponent->BindAction(SwitchWeaponsWheelAction, ETriggerEvent::Triggered, this, &APlayer_Base::SwitchWeapons);
		EnhancedInputComponent->BindAction(SwitchPrimaryWeaponAction, ETriggerEvent::Triggered, this, &APlayer_Base::SwitchToPrimaryWeapon);
		EnhancedInputComponent->BindAction(SwitchHandgunWeaponAction, ETriggerEvent::Triggered, this, &APlayer_Base::SwitchToHandgunWeapon);

		// Shooting
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &APlayer_Base::ShootFire);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &APlayer_Base::Aim);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &APlayer_Base::Reload);

		// Movement
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &APlayer_Base::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayer_Base::SprintCompleted);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &APlayer_Base::ToggleCrouch);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &APlayer_Base::Dodge);

		// Camera
		EnhancedInputComponent->BindAction(CameraChangeAction, ETriggerEvent::Started, this, &APlayer_Base::FlipFlapCameraChange);

		// Interaction
		EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Triggered, this, &APlayer_Base::Interact);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input Handling

void APlayer_Base::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		TurnRate = FMath::Clamp(MovementVector.X, -1.0f, 1.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

		DodgeForward = MovementVector.Y;
		DodgeRight = MovementVector.X;
	}
}

void APlayer_Base::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayer_Base::ShootFire(const FInputActionValue& Value)
{
	if (!IsAim)
		return;

	bFiring = Value.Get<bool>();
	HandleFiring();
}

void APlayer_Base::Aim(const FInputActionValue& Value)
{
	if (!IsPrimaryEquip && !IsPistolEquip)
		return;

	IsAim = Value.Get<bool>();

	if (IsAim)
	{
		// bUseControllerRotationYaw is already true (set in base class)
		AimTimeline.Play();
	}
	else
	{
		// Keep bUseControllerRotationYaw = true (always rotate with mouse in TPS)
		AimTimeline.Reverse();
	}
}

void APlayer_Base::Reload()
{
	AMasterWeapon* MasterWeapon = nullptr;

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
		return;
	}

	if (!MasterWeapon)
		return;

	MasterWeapon->Reload();
}

void APlayer_Base::Sprint(const FInputActionValue& Value)
{
	if (!GetCharacterMovement()->IsCrouching() && !bInteracting)
	{
		IsSprint = true;
		GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	}
}

void APlayer_Base::SprintCompleted(const FInputActionValue& Value)
{
	IsSprint = false;
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
}

void APlayer_Base::ToggleCrouch(const FInputActionValue& Value)
{
	if (!bInteracting && !GetCharacterMovement()->IsFalling())
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

void APlayer_Base::Dodge()
{
	if (bInteracting || GetCharacterMovement()->IsFalling())
		return;

	if (DodgeForward != 0.0f)
	{
		UAnimMontage* MontageToPlay = nullptr;
		if (DodgeForward > 0.0f)
		{
			MontageToPlay = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr,
				TEXT("/Game/ThirdPerson/Blueprints/Animation/Dodge/Montage/DiveRoll_F_Montage")));
		}
		else
		{
			MontageToPlay = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr,
				TEXT("/Game/ThirdPerson/Blueprints/Animation/Dodge/Montage/DiveRoll_B_Montage")));
		}

		PlayDodgeMontage(MontageToPlay);
	}

	if (DodgeRight != 0.0f)
	{
		UAnimMontage* MontageToPlay = nullptr;
		if (DodgeRight > 0.0f)
		{
			MontageToPlay = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr,
				TEXT("/Game/ThirdPerson/Blueprints/Animation/Dodge/Montage/DiveRoll_R_Montage")));
		}
		else
		{
			MontageToPlay = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr,
				TEXT("/Game/ThirdPerson/Blueprints/Animation/Dodge/Montage/DiveRoll_L_Montage")));
		}

		PlayDodgeMontage(MontageToPlay);
	}
}

void APlayer_Base::Jumping()
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

//////////////////////////////////////////////////////////////////////////
// Camera Functions

void APlayer_Base::FlipFlapCameraChange()
{
	if (RightShoulder)
		return;

	static bool FlipFlap = false;
	FlipFlap = !FlipFlap;

	if (FlipFlap)
	{
		ShoulderCameraTimeline.Play();
	}
	else
	{
		ShoulderCameraTimeline.Reverse();
	}
}

void APlayer_Base::ShoulderCameraChange(float Value)
{
	CameraBoom->SocketOffset = FMath::Lerp(
		FVector(0.0f, ShoulderYOffset, ShoulderZOffset),
		FVector(0.0f, ShoulderYOffset * -1.0f, ShoulderZOffset),
		Value);
}

void APlayer_Base::UpdateAimView(float Value)
{
	CameraBoom->TargetArmLength = FMath::Lerp(TargetArmLengths.X, TargetArmLengths.Y, Value);

	CameraBoom->SocketOffset = FMath::Lerp(
		FVector(0.0f, CameraBoom->SocketOffset.Y, ShoulderZOffset),
		FVector(0.0f, CameraBoom->SocketOffset.Y, ShoulderZOffset),
		Value);
}

void APlayer_Base::UpdateCrouchHeight()
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

//////////////////////////////////////////////////////////////////////////
// Dodge Functions

void APlayer_Base::PlayDodgeMontage(UAnimMontage* MontageToPlay)
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(MontageToPlay);
		IsDodging = true;

		FOnMontageEnded CompleteDelegate;
		CompleteDelegate.BindUObject(this, &APlayer_Base::OnDodgeMontageEnded);
		AnimInstance->Montage_SetEndDelegate(CompleteDelegate, MontageToPlay);

		FOnMontageBlendingOutStarted BlendOutDelegate;
		BlendOutDelegate.BindUObject(this, &APlayer_Base::OnDodgeMontageInterrupted);
		AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, MontageToPlay);
	}
}

void APlayer_Base::OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		IsDodging = false;
	}
}

void APlayer_Base::OnDodgeMontageInterrupted(UAnimMontage* Montage, bool bInterrupted)
{
	IsDodging = false;
}

void APlayer_Base::ImpactOnLand()
{
	float AbsVelocity = FMath::Abs(GetCharacterMovement()->Velocity.Z);

	if (AbsVelocity >= 300.0f && AbsVelocity <= 900.0f)
	{
		CurrentLandState = ELandState::Soft;
	}
	else if (AbsVelocity >= 1000.0f && AbsVelocity <= 1200.0f)
	{
		CurrentLandState = ELandState::Normal;
	}
	else if (AbsVelocity > 1250.f)
	{
		CurrentLandState = ELandState::Hard;
	}
}

//////////////////////////////////////////////////////////////////////////
// Weapon Functions

void APlayer_Base::StopFire()
{
	bFiring = false;
	// bUseControllerRotationYaw remains true (always rotate with mouse in TPS)
}

void APlayer_Base::HandleFiring()
{
	UE_LOG(LogTemp, Log, TEXT("[HandleFiring] Called - bFiring: %s, bCanFire: %s"),
		bFiring ? TEXT("true") : TEXT("false"),
		bCanFire ? TEXT("true") : TEXT("false"));

	if (!bFiring)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HandleFiring] Blocked: bFiring is false"));
		return;
	}

	if (!bCanFire)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HandleFiring] Blocked: bCanFire is false"));
		return;
	}

	if (!CanFire())
	{
		UE_LOG(LogTemp, Warning, TEXT("[HandleFiring] Blocked: CanFire() returned false"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[HandleFiring] All checks passed - IsPrimaryEquip: %s, IsPistolEquip: %s"),
		IsPrimaryEquip ? TEXT("true") : TEXT("false"),
		IsPistolEquip ? TEXT("true") : TEXT("false"));

	if (IsPrimaryEquip)
	{
		AMasterWeapon* MasterWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
		UWeaponData* CurrentWeaponDataAsset = MasterWeapon->WeaponData;
		UE_LOG(LogTemp, Log, TEXT("[HandleFiring] Firing Primary Weapon"));
		ReadyToFire(MasterWeapon, CurrentWeaponDataAsset);
	}
	else if (IsPistolEquip)
	{
		AMasterWeapon* MasterWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
		UWeaponData* CurrentWeaponDataAsset = MasterWeapon->WeaponData;
		UE_LOG(LogTemp, Log, TEXT("[HandleFiring] Firing Handgun Weapon"));
		ReadyToFire(MasterWeapon, CurrentWeaponDataAsset);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[HandleFiring] No weapon equipped!"));
	}
}

bool APlayer_Base::CanFire()
{
	bool bCanJumpNow = CanJump();  // 한 번만 호출하고 저장
	bool bCanShoot = !IsSprint && !IsDodging && bCanJumpNow && bCanSwitchWeapon;
	bool bResult = bCanShoot || IsCrouch;

	UE_LOG(LogTemp, Log, TEXT("[CanFire] IsSprint: %s, IsDodging: %s, CanJump: %s, bCanSwitchWeapon: %s, IsCrouch: %s, Result: %s"),
		IsSprint ? TEXT("true") : TEXT("false"),
		IsDodging ? TEXT("true") : TEXT("false"),
		bCanJumpNow ? TEXT("true") : TEXT("false"),
		bCanSwitchWeapon ? TEXT("true") : TEXT("false"),
		IsCrouch ? TEXT("true") : TEXT("false"),
		bResult ? TEXT("true") : TEXT("false"));

	return bResult;
}

bool APlayer_Base::CanSwitchWeapon()
{
	return bCanSwitchWeapon && !bInteracting;
}

void APlayer_Base::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogTemp, Warning, TEXT("[OnMontageEnded] Called! bInterrupted: %s, Montage: %s"),
		bInterrupted ? TEXT("true") : TEXT("false"),
		Montage ? *Montage->GetName() : TEXT("null"));

	if (!bInterrupted)
	{
		bCanSwitchWeapon = true;
		UE_LOG(LogTemp, Log, TEXT("[OnMontageEnded] bCanSwitchWeapon set to TRUE"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[OnMontageEnded] Montage was interrupted, bCanSwitchWeapon remains FALSE"));
	}
}

UUserWidget* APlayer_Base::AddWeaponUI(UWeaponData* WeaponData)
{
	if (!IsPlayerControlled())
		return nullptr;

	ClearWeaponUI();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		UUserWidget* WeaponUI = CreateWidget<UUserWidget>(PlayerController, WeaponData->WeaponUI);
		if (WeaponUI)
		{
			CurrentWeaponUI = Cast<UW_DynamicWeaponHUD>(WeaponUI);
			if (!CurrentWeaponUI)
			{
				return nullptr;
			}

			if (IsPrimaryEquip)
			{
				CurrentWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
			}
			else
			{
				CurrentWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
			}

			if (!CurrentWeapon || !CurrentWeapon->WeaponSystem)
			{
				return nullptr;
			}

			CurrentWeaponUI->SetWeaponData(
				WeaponData->WeaponUITexture,
				WeaponData->WeaponName,
				CurrentWeapon->WeaponSystem->Weapon_Details.Weapon_Data.MaxAmmo,
				CurrentWeapon->WeaponSystem->Weapon_Details.Weapon_Data.CurrentAmmo);

			CurrentWeaponUI->AddToViewport();

			return CurrentWeaponUI;
		}
	}

	return nullptr;
}

void APlayer_Base::SwitchWeapons()
{
}

void APlayer_Base::SwitchToPrimaryWeapon()
{
	if (!CanSwitchWeapon() || IsPrimaryEquip)
		return;

	if (!LocomotionBP)
	{
		return;
	}

	bCanSwitchWeapon = false;
	IsPistolEquip = false;

	// 1. Unequip Pistol
	EquipmentSystem->SetWeaponState(
		EquipmentSystem->HandgunWeaponClass,
		EAnimationState::Pistol,
		EWeaponState::Unequip,
		FName(""),
		FName("PistolHost_Socket"),
		EWeaponSlot::Handgun
	);
	IsPrimaryEquip = true;

	// 2. Equip Rifle
	EquipmentSystem->SetWeaponState(
		EquipmentSystem->PrimaryWeaponClass,
		EAnimationState::RifleShotgun,
		EWeaponState::Equip,
		FName("Rifle_Socket"),
		FName(""),
		EWeaponSlot::Primary
	);

	// 3. Wait for Child Actor to initialize and update UI
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			if (PrimaryChild)
			{
				if (AMasterWeapon* Weapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor()))
				{
					if (Weapon->WeaponData)
					{
						AddWeaponUI(Weapon->WeaponData);
						if (LocomotionBP)
						{
							LocomotionBP->LeftHandIKOffset = Weapon->WeaponData->LeftHandIKOffset;
						}
					}
				}
			}
		},
		0.1f,
		false
	);

	// Play Animation Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UE_LOG(LogTemp, Log, TEXT("[SwitchToPrimaryWeapon] AnimInstance: %s"), AnimInstance ? TEXT("Valid") : TEXT("NULL"));

	if (AnimInstance)
	{
		UAnimMontage* RifleEquipMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr,
			TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Rifle/Montages/MM_Rifle_Equip1")));

		UE_LOG(LogTemp, Log, TEXT("[SwitchToPrimaryWeapon] RifleEquipMontage: %s"),
			RifleEquipMontage ? *RifleEquipMontage->GetName() : TEXT("NULL"));

		if (RifleEquipMontage)
		{
			AnimInstance->Montage_Play(RifleEquipMontage, 1.0f);

			FOnMontageEnded CompleteDelegate;
			CompleteDelegate.BindUObject(this, &APlayer_Base::OnMontageEnded);
			AnimInstance->Montage_SetEndDelegate(CompleteDelegate, RifleEquipMontage);

			UE_LOG(LogTemp, Log, TEXT("[SwitchToPrimaryWeapon] Montage delegate set successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[SwitchToPrimaryWeapon] Failed to load RifleEquipMontage! Setting bCanSwitchWeapon = true"));
			bCanSwitchWeapon = true;  // 몽타주 로드 실패 시 즉시 복구
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[SwitchToPrimaryWeapon] AnimInstance is NULL! Setting bCanSwitchWeapon = true"));
		bCanSwitchWeapon = true;  // AnimInstance 없으면 즉시 복구
	}
}

void APlayer_Base::SwitchToHandgunWeapon()
{
	if (!CanSwitchWeapon() || IsPistolEquip)
		return;

	if (!LocomotionBP)
	{
		return;
	}

	bCanSwitchWeapon = false;
	IsPrimaryEquip = false;

	// 1. Unequip Rifle
	EquipmentSystem->SetWeaponState(
		EquipmentSystem->PrimaryWeaponClass,
		EAnimationState::RifleShotgun,
		EWeaponState::Unequip,
		FName(""),
		FName("RifleHost_Socket"),
		EWeaponSlot::Primary
	);
	IsPistolEquip = true;

	// 2. Equip Pistol
	EquipmentSystem->SetWeaponState(
		EquipmentSystem->HandgunWeaponClass,
		EAnimationState::Pistol,
		EWeaponState::Equip,
		FName("Pistol_Socket"),
		FName(""),
		EWeaponSlot::Handgun
	);

	// 3. Wait for Child Actor to initialize and update UI
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			if (HandgunChild)
			{
				if (AMasterWeapon* Weapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor()))
				{
					if (Weapon->WeaponData)
					{
						AddWeaponUI(Weapon->WeaponData);
						if (LocomotionBP)
						{
							LocomotionBP->LeftHandIKOffset = Weapon->WeaponData->LeftHandIKOffset;
						}
					}
				}
			}
		},
		0.1f,
		false
	);

	// Play Animation Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		UAnimMontage* PistolEquipMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr,
			TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Pistol/Montages/MM_Pistol_Equip2")));

		AnimInstance->Montage_Play(PistolEquipMontage, 1.0f);

		FOnMontageEnded CompleteDelegate;
		CompleteDelegate.BindUObject(this, &APlayer_Base::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(CompleteDelegate, PistolEquipMontage);
	}
}

void APlayer_Base::ReadyToFire(AMasterWeapon* MasterWeapon, UWeaponData* CurrentWeaponDataAsset)
{
	if (!MasterWeapon || !CurrentWeaponDataAsset)
		return;
	
	bCanFire = false;
	MasterWeapon->Fire();

	if (CurrentWeaponUI)
	{
		CurrentWeaponUI->UpdateAmmoCount(
			MasterWeapon->WeaponSystem->Weapon_Details.Weapon_Data.MaxAmmo,
			MasterWeapon->WeaponSystem->Weapon_Details.Weapon_Data.CurrentAmmo);
	}

	float FireDelay = CurrentWeaponDataAsset->FireRate;
	EFireMode CurrentFireMode = CurrentWeaponDataAsset->FireMode;
	FTimerHandle TimerHandle;

	switch (CurrentFireMode)
	{
	case EFireMode::FullAuto:
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				bCanFire = true;
				HandleFiring();
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

void APlayer_Base::Interact()
{
	if (!InteractorComponent->HasInteraction)
		return;

	switch (InteractorComponent->GetInteractionType())
	{
	case EInteractiveType::Default:
		InteractorComponent->StartInteraction(GetController());
		break;

	case EInteractiveType::Pickup:
		break;

	case EInteractiveType::WeaponPickup:
		AIWeaponPickup* PickupToWeapon = Cast<AIWeaponPickup>(InteractorComponent->InteractionActor);

		if (PickupToWeapon)
		{
			AMasterWeapon* MasterWeapon = nullptr;
			FVector InteractionActorLocation = InteractorComponent->InteractionActor->GetActorLocation();

			FTransform InteractionSpawnTransform;
			InteractionSpawnTransform.SetLocation(FVector(InteractionActorLocation.X, InteractionActorLocation.Y, InteractionActorLocation.Z + 20.0f));
			InteractionSpawnTransform.SetRotation(FQuat::Identity);
			InteractionSpawnTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (!AnimInstance)
			{
				return;
			}

			switch (PickupToWeapon->WeaponData->WeaponType)
			{
			case EWeaponType::Pistol:
				MasterWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
				if (!MasterWeapon)
				{
					return;
				}

				GetWorld()->SpawnActor<AActor>(
					MasterWeapon->WeaponPickupClass,
					InteractionSpawnTransform
				);

				if (IsPrimaryEquip)
				{
					IsPrimaryEquip = false;
					EquipmentSystem->SetWeaponState(
						EquipmentSystem->PrimaryWeaponClass,
						EAnimationState::RifleShotgun,
						EWeaponState::Unequip,
						FName(""),
						FName("RifleHost_Socket"),
						EWeaponSlot::Primary
					);
				}

				HandgunChild->SetChildActorClass(PickupToWeapon->WeaponData->WeaponClass->StaticClass());
				IsPistolEquip = true;

				EquipmentSystem->SetWeaponState(
					EquipmentSystem->HandgunWeaponClass,
					EAnimationState::Pistol,
					EWeaponState::Equip,
					FName("PistolHost_Socket"),
					FName(""),
					EWeaponSlot::Handgun
				);

				if (AnimInstance)
				{
					UAnimMontage* PistolEquipMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr,
						TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Pistol/Montages/MM_Pistol_Equip2")));

					AnimInstance->Montage_Play(PistolEquipMontage, 1.0f);

					FOnMontageEnded CompleteDelegate;
					CompleteDelegate.BindUObject(this, &APlayer_Base::OnMontageEnded);
					AnimInstance->Montage_SetEndDelegate(CompleteDelegate, PistolEquipMontage);

					InteractorComponent->StartInteraction(GetController());
				}
				break;

			case EWeaponType::RifleAndShotgun:
				MasterWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
				if (!MasterWeapon)
				{
					return;
				}

				GetWorld()->SpawnActor<AActor>(
					MasterWeapon->WeaponPickupClass,
					InteractionSpawnTransform
				);

				if (IsPistolEquip)
				{
					IsPistolEquip = false;
					EquipmentSystem->SetWeaponState(
						EquipmentSystem->HandgunWeaponClass,
						EAnimationState::Pistol,
						EWeaponState::Equip,
						FName(""),
						FName("Pistol_Socket"),
						EWeaponSlot::Handgun
					);
				}

				HandgunChild->SetChildActorClass(PickupToWeapon->WeaponData->WeaponClass->StaticClass());
				IsPrimaryEquip = true;

				EquipmentSystem->SetWeaponState(
					EquipmentSystem->PrimaryWeaponClass,
					EAnimationState::RifleShotgun,
					EWeaponState::Unequip,
					FName("RifleHost_Socket"),
					FName(""),
					EWeaponSlot::Primary
				);

				if (AnimInstance)
				{
					UAnimMontage* RifleEquipMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr,
						TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Rifle/Montages/MM_Rifle_Equip1")));

					AnimInstance->Montage_Play(RifleEquipMontage, 1.0f);

					FOnMontageEnded CompleteDelegate;
					CompleteDelegate.BindUObject(this, &APlayer_Base::OnMontageEnded);
					AnimInstance->Montage_SetEndDelegate(CompleteDelegate, RifleEquipMontage);

					InteractorComponent->StartInteraction(GetController());
				}
				break;
			}
		}
		break;
	}
}

void APlayer_Base::ClearWeaponUI()
{
	if (!CurrentWeaponUI)
		return;

	CurrentWeaponUI->RemoveFromParent();
}
