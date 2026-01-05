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
#include "Weapon/Interaction.h"
#include "Data/InteractionData.h"
#include "Data/InteractionContext.h"
#include "Kismet/GameplayStatics.h"

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

	// Get Animation Instance
	LocomotionBP = Cast<ULocomotionAnimInstance>(GetMesh()->GetAnimInstance());

	// Setup Timelines (Aim and Crouch timelines initialized in base class)
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

	// Interaction 이벤트 바인딩 (Data-Driven)
	// 레벨의 모든 Interaction에 이벤트 핸들러 연결
	TArray<AActor*> FoundInteractions;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteraction::StaticClass(), FoundInteractions);

	for (AActor* Actor : FoundInteractions)
	{
		if (AInteraction* Interaction = Cast<AInteraction>(Actor))
		{
			Interaction->OnInteractionExecuted.AddDynamic(this, &APlayer_Base::OnInteractionExecuted_Handler);
			UE_LOG(LogTemp, Log, TEXT("Bound interaction event: %s"), *Interaction->GetName());
		}
	}

	EquipmentSystem->OnEquipmentStateChanged.AddDynamic(this, &APlayer_Base::OnEquipped);
}

void APlayer_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // Base class ticks Aim and Crouch timelines

	ShoulderCameraTimeline.TickTimeline(DeltaTime);
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
	if (bIsUIState)
		return;
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayer_Base::ShootFire(const FInputActionValue& Value)
{
	if (!bIsAim)
		return;

	bFiring = Value.Get<bool>();
	HandleFiring();
}

void APlayer_Base::Aim(const FInputActionValue& Value)
{
	if (!EquipmentSystem || EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::None)
		return;

	bool bShouldAim = Value.Get<bool>();

	if (bShouldAim)
	{
		StartAim(); // Base class function
	}
	else
	{
		StopAim(); // Base class function
	}
}

void APlayer_Base::Reload()
{
	if (!EquipmentSystem)
		return;

	AMasterWeapon* MasterWeapon = nullptr;

	if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Primary)
	{
		MasterWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
	}
	else if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Handgun)
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
	if (IsCrouch)
	{
		StopCrouch(); // Base class function
	}
	else
	{
		StartCrouch(); // Base class function
	}
}

void APlayer_Base::Dodge()
{
	PerformDodge(DodgeForward, DodgeRight); // Base class function
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

void APlayer_Base::UpdateAimTimeline(float Value)
{
	Super::UpdateAimTimeline(Value); // Call base class (empty implementation)

	// Player-specific camera adjustments
	CameraBoom->TargetArmLength = FMath::Lerp(TargetArmLengths.X, TargetArmLengths.Y, Value);

	CameraBoom->SocketOffset = FMath::Lerp(
		FVector(0.0f, CameraBoom->SocketOffset.Y, ShoulderZOffset),
		FVector(0.0f, CameraBoom->SocketOffset.Y, ShoulderZOffset),
		Value);
}

void APlayer_Base::UpdateCrouchTimeline(float Value)
{
	Super::UpdateCrouchTimeline(Value); // Base class handles capsule radius

	// Player could add camera height adjustments here if needed
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

void APlayer_Base::OnEquipped()
{
	if (!CurrentWeapon || !CurrentWeapon->WeaponData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OnEquipped] CurrentWeapon not ready yet"));
		return;
	}
	
}

//////////////////////////////////////////////////////////////////////////
// Weapon Functions

void APlayer_Base::StopFire()
{
	bFiring = false;
}

void APlayer_Base::HandleFiring()
{
	if (!EquipmentSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HandleFiring] Blocked: EquipmentSystem is null"));
		return;
	}

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

	if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Primary)
	{
		AMasterWeapon* MasterWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
		UWeaponData* CurrentWeaponDataAsset = MasterWeapon->WeaponData;
		ReadyToFire(MasterWeapon, CurrentWeaponDataAsset);
	}
	else if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Handgun)
	{
		AMasterWeapon* MasterWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
		UWeaponData* CurrentWeaponDataAsset = MasterWeapon->WeaponData;
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
	bool bCanShoot = !IsSprint && !bIsDodging && bCanJumpNow && bCanSwitchWeapon;
	bool bResult = bCanShoot || IsCrouch;

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
	UE_LOG(LogTemp, Log, TEXT("[AddWeaponUI] Called - WeaponData: %s, EquipmentSystem: %s"),
		WeaponData ? *WeaponData->GetName() : TEXT("NULL"),
		EquipmentSystem ? TEXT("Valid") : TEXT("NULL"));

	if (!IsPlayerControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AddWeaponUI] Not player controlled, returning nullptr"));
		return nullptr;
	}

	if (!EquipmentSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[AddWeaponUI] EquipmentSystem is nullptr!"));
		return nullptr;
	}

	ClearWeaponUI();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		UUserWidget* WeaponUI = CreateWidget<UUserWidget>(PlayerController, WeaponData->WeaponUI);
		if (WeaponUI)
		{
			CurrentWeaponUI = Cast<UW_DynamicWeaponHUD>(WeaponUI);
			if (!CurrentWeaponUI)
			{
				UE_LOG(LogTemp, Error, TEXT("[AddWeaponUI] Failed to cast to UW_DynamicWeaponHUD"));
				return nullptr;
			}

			if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Primary)
			{
				CurrentWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
			}
			else if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Handgun)
			{
				CurrentWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
			}

			if (!CurrentWeapon || !CurrentWeapon->WeaponSystem)
			{
				ClearWeaponUI();
				CurrentWeaponUI = nullptr;
				return nullptr;
			}

			CurrentWeaponUI->SetWeaponData(
				WeaponData->WeaponUITexture,
				WeaponData->ItemName.ToString(),
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
	if (!EquipmentSystem || !CanSwitchWeapon() || EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Primary)
		return;

	if (!LocomotionBP)
	{
		return;
	}
	if (!EquipmentSystem->IsEquipped(EEquipmentSlot::Primary))
	{
		UE_LOG(LogTemp, Warning, TEXT("There is no Equipped Primary Weapon"));
		return;
	}
	
	bCanSwitchWeapon = false;

	EquipmentSystem->SwitchToWeapon(EEquipmentSlot::Primary);

	if (PrimaryChild)
	{
		if (AMasterWeapon* Weapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor()))
		{
			CurrentWeapon = Weapon;
		}
	}
	
	// 3. Wait for Child Actor to initialize and update UI
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			if (CurrentWeapon->WeaponData)
			{
				AddWeaponUI(CurrentWeapon->WeaponData);
				if (LocomotionBP)
				{
					LocomotionBP->LeftHandIKOffset = CurrentWeapon->WeaponData->LeftHandIKOffset;
				}
			}
		},
		0.1f,
		false
	);

	// Play Animation Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UE_LOG(LogTemp, Log, TEXT("[OnEquipped] AnimInstance: %s"), AnimInstance ? TEXT("Valid") : TEXT("NULL"));

	if (AnimInstance)
	{
		UAnimMontage* RifleEquipMontage = CurrentWeapon->WeaponData->WeaponEquipMontage;

		UE_LOG(LogTemp, Log, TEXT("[OnEquipped] RifleEquipMontage: %s"),
			RifleEquipMontage ? *RifleEquipMontage->GetName() : TEXT("NULL"));

		if (RifleEquipMontage)
		{
			AnimInstance->Montage_Play(RifleEquipMontage, 1.0f);

			FOnMontageEnded CompleteDelegate;
			CompleteDelegate.BindUObject(this, &APlayer_Base::OnMontageEnded);
			AnimInstance->Montage_SetEndDelegate(CompleteDelegate, RifleEquipMontage);

			UE_LOG(LogTemp, Log, TEXT("[OnEquipped] Montage delegate set successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[OnEquipped] Failed to load RifleEquipMontage! Setting bCanSwitchWeapon = true"));
			bCanSwitchWeapon = true;  // 몽타주 로드 실패 시 즉시 복구
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[OnEquipped] AnimInstance is NULL! Setting bCanSwitchWeapon = true"));
		bCanSwitchWeapon = true;  // AnimInstance 없으면 즉시 복구
	}
}

void APlayer_Base::SwitchToHandgunWeapon()
{
	if (!EquipmentSystem || !CanSwitchWeapon() || EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Handgun)
		return;

	if (!LocomotionBP)
	{
		return;
	}
	if (!EquipmentSystem->IsEquipped(EEquipmentSlot::Handgun))
	{
		UE_LOG(LogTemp, Warning, TEXT("There is no Equipped Handgun Weapon"));
		return;
	}
	bCanSwitchWeapon = false;

	EquipmentSystem->SwitchToWeapon(EEquipmentSlot::Handgun);

	if (HandgunChild)
	{
		if (AMasterWeapon* Weapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor()))
		{
			CurrentWeapon = Weapon;
			
		}
	}
	
	// 3. Wait for Child Actor to initialize and update UI
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			if (CurrentWeapon->WeaponData)
			{
				AddWeaponUI(CurrentWeapon->WeaponData);
				if (LocomotionBP)
				{
					LocomotionBP->LeftHandIKOffset = CurrentWeapon->WeaponData->LeftHandIKOffset;
				}
			}
		},
		0.1f,
		false
	);

	// TODO: Move to OnEquipped Delegate Func
	// Play Animation Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UE_LOG(LogTemp, Log, TEXT("[OnEquipped] AnimInstance: %s"), AnimInstance ? TEXT("Valid") : TEXT("NULL"));

	if (AnimInstance)
	{
		UAnimMontage* RifleEquipMontage = CurrentWeapon->WeaponData->WeaponEquipMontage;

		UE_LOG(LogTemp, Log, TEXT("[OnEquipped] RifleEquipMontage: %s"),
			RifleEquipMontage ? *RifleEquipMontage->GetName() : TEXT("NULL"));

		if (RifleEquipMontage)
		{
			AnimInstance->Montage_Play(RifleEquipMontage, 1.0f);

			FOnMontageEnded CompleteDelegate;
			CompleteDelegate.BindUObject(this, &APlayer_Base::OnMontageEnded);
			AnimInstance->Montage_SetEndDelegate(CompleteDelegate, RifleEquipMontage);

			UE_LOG(LogTemp, Log, TEXT("[OnEquipped] Montage delegate set successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[OnEquipped] Failed to load RifleEquipMontage! Setting bCanSwitchWeapon = true"));
			bCanSwitchWeapon = true;  // 몽타주 로드 실패 시 즉시 복구
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[OnEquipped] AnimInstance is NULL! Setting bCanSwitchWeapon = true"));
		bCanSwitchWeapon = true;  // AnimInstance 없으면 즉시 복구
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
	// 새로운 Data-Driven 방식: Interactor가 모든 것을 처리!
	if (!InteractorComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("InteractorComponent is null!"));
		return;
	}

	// Interactor에게 상호작용 실행 요청
	// - TriggerInteraction()이 자동으로:
	//   1. 현재 Interaction 유효성 체크
	//   2. Hold 타입이면 시작, 아니면 즉시 실행
	//   3. OnInteractionExecuted 이벤트 브로드캐스트
	InteractorComponent->TriggerInteraction();

	// 이제 모든 로직은 이벤트 핸들러에서 처리됨!
	// (BeginPlay에서 바인딩한 OnInteractionExecuted_Handler 참조)
}

void APlayer_Base::ClearWeaponUI()
{
	if (!CurrentWeaponUI)
		return;

	CurrentWeaponUI->RemoveFromParent();
}

//==============================================================================
// Interaction Event Handlers (Data-Driven)
//==============================================================================

void APlayer_Base::OnInteractionExecuted_Handler(AInteraction* Interaction, const FInteractionContext& Context)
{
	if (!Interaction || !Context.InteractionData)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnInteractionExecuted_Handler: Invalid parameters"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Interaction executed: %s (Type: %d)"),
		*Interaction->GetName(),
		static_cast<int32>(Context.InteractionData->InteractionType));

	// 타입별 처리
	switch (Context.InteractionData->InteractionType)
	{
	case EInteractiveType::WeaponPickup:
		UE_LOG(LogTemp, Warning, TEXT(">>> WeaponPickup case triggered!"));
		HandleWeaponPickup(Interaction, Context);
		break;

	case EInteractiveType::Pickup:
		UE_LOG(LogTemp, Warning, TEXT(">>> General Pickup case triggered!"));
		break;

	case EInteractiveType::Default:
	default:
		UE_LOG(LogTemp, Warning, TEXT(">>> Default case triggered! InteractionType=%d"),
			static_cast<int32>(Context.InteractionData->InteractionType));
		break;
	}
}

void APlayer_Base::HandleWeaponPickup(AInteraction* Interaction, const FInteractionContext& Context)
{
	if (!Interaction || !Context.InteractionData || !EquipmentSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("HandleWeaponPickup: Invalid parameters"));
		return;
	}

	// PayloadData에서 무기 정보 추출
	const TMap<FString, FString>& PayloadData = Context.InteractionData->PayloadData;

	FString WeaponTypeStr = PayloadData.FindRef("WeaponType");
	FString WeaponClassPath = PayloadData.FindRef("WeaponClass");

	if (WeaponTypeStr.IsEmpty() || WeaponClassPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("HandleWeaponPickup: Missing WeaponType or WeaponClass in PayloadData"));
		return;
	}

	// WeaponType 파싱
	EEquipmentSlot TargetSlot = EEquipmentSlot::Primary;
	FString MontagePath;

	if (WeaponTypeStr.Equals("Pistol"))
	{
		TargetSlot = EEquipmentSlot::Handgun;
		MontagePath = TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Pistol/Montages/MM_Pistol_Equip2");
	}
	else if (WeaponTypeStr.Equals("RifleAndShotgun"))
	{
		TargetSlot = EEquipmentSlot::Primary;
		MontagePath = TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Rifle/Montages/MM_Rifle_Equip1");
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HandleWeaponPickup: Undefined WeaponTypeStr"));
	}

	// 무기 클래스 로드
	TSubclassOf<AMasterWeapon> NewWeaponClass = LoadClass<AMasterWeapon>(nullptr, *WeaponClassPath);
	if (!NewWeaponClass)
	{
		UE_LOG(LogTemp, Error, TEXT("HandleWeaponPickup: Failed to load weapon class: %s"), *WeaponClassPath);
		return;
	}

	// 기존 무기의 탄약 상태 저장 (드롭용)
	FWeaponAmmoState DroppedAmmoState;
	bool bHasSavedAmmo = false;

	UChildActorComponent* CurrentTargetChild = (TargetSlot == EEquipmentSlot::Primary) ? PrimaryChild : HandgunChild;
	if (CurrentTargetChild)
	{
		if (AMasterWeapon* CurWeapon = Cast<AMasterWeapon>(CurrentTargetChild->GetChildActor()))
		{
			if (CurWeapon->WeaponSystem)
			{
				// 현재 무기의 탄약 정보 저장
				DroppedAmmoState.CurrentAmmo = CurWeapon->WeaponSystem->Weapon_Details.Weapon_Data.CurrentAmmo;
				DroppedAmmoState.MaxAmmo = CurWeapon->WeaponSystem->Weapon_Details.Weapon_Data.MaxAmmo;
				DroppedAmmoState.ClipAmmo = CurWeapon->WeaponSystem->Weapon_Details.Weapon_Data.ClipAmmo;
				DroppedAmmoState.DifferentAmmo = CurWeapon->WeaponSystem->Weapon_Details.Weapon_Data.DifferentAmmo;
				DroppedAmmoState.AmmoCount = CurWeapon->WeaponSystem->Weapon_Details.Weapon_Data.Ammo_Count;
				bHasSavedAmmo = true;

				UE_LOG(LogTemp, Log, TEXT("HandleWeaponPickup: Saved ammo state - CurrentAmmo: %d, MaxAmmo: %d"),
					DroppedAmmoState.CurrentAmmo, DroppedAmmoState.MaxAmmo);
			}
		}
	}

	// EquipmentSystem을 통해 무기 픽업 및 장착
	TSubclassOf<AMasterWeapon> DroppedWeaponClass;
	if (!EquipmentSystem->PickupAndEquipWeapon(NewWeaponClass, TargetSlot, DroppedWeaponClass))
	{
		UE_LOG(LogTemp, Error, TEXT("HandleWeaponPickup: Failed to pickup weapon"));
		return;
	}

	// 무기 장착 플래그 설정 (발사/조준 활성화)
	if (TargetSlot == EEquipmentSlot::Primary)
	{
		EquipmentSystem->CurrentEquippedSlot = EEquipmentSlot::Primary;
	}
	else if (TargetSlot == EEquipmentSlot::Handgun)
	{
		EquipmentSystem->CurrentEquippedSlot = EEquipmentSlot::Handgun;
	}
	else
	{
		EquipmentSystem->CurrentEquippedSlot = EEquipmentSlot::None;
	}

	// 기존 무기를 바닥에 드롭
	if (DroppedWeaponClass)
	{
		FVector DropLocation = Interaction->GetActorLocation() + FVector(0, 0, 20);
		FTransform DropTransform(FQuat::Identity, DropLocation);

		// CDO(Class Default Object)에서 WeaponPickupClass 가져오기
		AMasterWeapon* WeaponCDO = Cast<AMasterWeapon>(DroppedWeaponClass->GetDefaultObject());
		if (WeaponCDO && WeaponCDO->WeaponPickupClass)
		{
			// WeaponPickupClass 스폰
			AInteraction* DroppedPickup = GetWorld()->SpawnActor<AInteraction>(WeaponCDO->WeaponPickupClass, DropTransform);
			if (DroppedPickup)
			{
				// Runtime 생성된 Interaction에 이벤트 바인딩
				DroppedPickup->OnInteractionExecuted.AddDynamic(this, &APlayer_Base::OnInteractionExecuted_Handler);

				// 저장된 탄약 상태 적용
				if (bHasSavedAmmo)
				{
					DroppedPickup->SavedAmmoState = DroppedAmmoState;
					DroppedPickup->bHasCustomState = true;

					UE_LOG(LogTemp, Log, TEXT("Dropped weapon pickup with saved ammo: CurrentAmmo=%d, MaxAmmo=%d"),
						DroppedAmmoState.CurrentAmmo, DroppedAmmoState.MaxAmmo);
				}

				UE_LOG(LogTemp, Log, TEXT("Dropped weapon pickup spawned and event bound: %s"), *DroppedPickup->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Not set DroppedWeaponClass"));
		}
	}

	// UI 업데이트 및 탄약 상태 복원 (ChildActor 초기화 대기)
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this, TargetSlot, Interaction]()
		{
			UChildActorComponent* TargetChild = (TargetSlot == EEquipmentSlot::Primary) ? PrimaryChild : HandgunChild;
			if (TargetChild)
			{
				if (AMasterWeapon* Weapon = Cast<AMasterWeapon>(TargetChild->GetChildActor()))
				{
					// ✅ 저장된 탄약 상태 복원
					if (Interaction && Interaction->bHasCustomState && Weapon->WeaponSystem)
					{
						Weapon->WeaponSystem->Weapon_Details.Weapon_Data.CurrentAmmo = Interaction->SavedAmmoState.CurrentAmmo;
						Weapon->WeaponSystem->Weapon_Details.Weapon_Data.MaxAmmo = Interaction->SavedAmmoState.MaxAmmo;
						Weapon->WeaponSystem->Weapon_Details.Weapon_Data.ClipAmmo = Interaction->SavedAmmoState.ClipAmmo;
						Weapon->WeaponSystem->Weapon_Details.Weapon_Data.DifferentAmmo = Interaction->SavedAmmoState.DifferentAmmo;
						Weapon->WeaponSystem->Weapon_Details.Weapon_Data.Ammo_Count = Interaction->SavedAmmoState.AmmoCount;

						UE_LOG(LogTemp, Log, TEXT("HandleWeaponPickup: Restored ammo state - CurrentAmmo: %d, MaxAmmo: %d"),
							Interaction->SavedAmmoState.CurrentAmmo, Interaction->SavedAmmoState.MaxAmmo);
					}

					if (Weapon->WeaponData)
					{
						AddWeaponUI(Weapon->WeaponData);
						if (LocomotionBP)
						{
							LocomotionBP->LeftHandIKOffset = Weapon->WeaponData->LeftHandIKOffset;
						}
						UE_LOG(LogTemp, Log, TEXT("HandleWeaponPickup: UI updated for %s"), *Weapon->GetName());
					}
				}
			}
		},
		0.1f,
		false
	);

	// 장착 애니메이션 재생
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && !MontagePath.IsEmpty())
	{
		UAnimMontage* EquipMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, *MontagePath));
		if (EquipMontage)
		{
			AnimInstance->Montage_Play(EquipMontage, 1.0f);

			FOnMontageEnded CompleteDelegate;
			CompleteDelegate.BindUObject(this, &APlayer_Base::OnMontageEnded);
			AnimInstance->Montage_SetEndDelegate(CompleteDelegate, EquipMontage);
		}
	}

	// Interaction Actor 제거
	Interaction->Destroy();

	UE_LOG(LogTemp, Log, TEXT("HandleWeaponPickup: Equipped %s successfully"), *WeaponClassPath);
}

void APlayer_Base::HandlePickup(class AInteraction* Interaction, const struct FInteractionContext& Context)
{
	if (!Interaction || !Context.InteractionData || !EquipmentSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("HandleWeaponPickup: Invalid parameters"));
		return;
	}

	
}
