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
#include "WeaponDataAsset.h"
#include "./Public/Weapon_AssultRifle.h"
#include "./Public/AWeapon_Handgun.h"
#include "./Public/Weapon/DA_Rifle.h"
#include "./Public/Weapon/DA_Pistol.h"
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
	bUseControllerRotationYaw = true;     // Disable yaw rotation
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

void ATPSTemplateCharacter::BeginPlay()
{
	Super::BeginPlay();

	// MEMO: When initialized in the constructor instead of BeginPlay, 
	// RifleData becomes NULL again
	// TODO: MasterWeapon���� �ʱ�ȭ ���ٶ� �̻������ ����
	

	if (Primary)
	{
		Primary->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("RifleHost_Socket"));
		TArray<USceneComponent*> AttachedChildren = Primary->GetAttachChildren();
		if (AttachedChildren.Num() > 0)
		{
			PrimaryChild = Cast<UChildActorComponent>(AttachedChildren[0]);
			if (PrimaryChild)
			{
				UE_LOG(LogTemp, Warning, TEXT("Successed PrimaryChild###2"));
			}
		}
	}
	if (Handgun)
	{
		Handgun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("PistolHost_Socket"));
		TArray<USceneComponent*> AttachedChildren = Handgun->GetAttachChildren();
		if (AttachedChildren.Num() > 0)
		{
			HandgunChild = Cast<UChildActorComponent>(AttachedChildren[0]);
			if (HandgunChild)
			{
				UE_LOG(LogTemp, Warning, TEXT("Successed HandgunChild###2"));
			}
		}
	}

	// Sequence 1
	WeaponSystem->CharacterRef = this;
	HealthComponent->CharacterRef = this;

	// Sequence 2
	CameraBoom->SocketOffset = FVector(0.0f, ShoulderYOffset, ShoulderZOffset);
	CameraBoom->TargetArmLength = TargetArmLengths.X;

	// Sequence 3
	// Create HUD Widget
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		UICrosshair = CreateWidget<UUserWidget>(PlayerController,
			LoadClass<UUserWidget>(nullptr, TEXT("/Game/ThirdPerson/Blueprints/W_ShooterHUD.W_ShooterHUD_C")));

		if (UICrosshair)
		{
			UICrosshair->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("Successfully created and added UICrosshair to viewport"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create UICrosshair widget"));
		}
	}
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
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::Look);

		// Switch Weapons
		EnhancedInputComponent->BindAction(SwitchWeaponsAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::SwitchWeapons);

		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::ShootFire);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::Reload);

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
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
    /* 무기 전환 시 타이밍 이슈 해결
     * 
     * 문제 원인:
     * 1. AddWeaponUI 함수가 무기 상태 변경(Weapon_State)과 동시에 호출됨
     * 2. Weapon_State에서 Child Actor 컴포넌트의 설정이 비동기적으로 이루어짐
     * 3. 그 결과 AddWeaponUI가 호출될 때 GetChildActor()가 아직 NULL을 반환
     * 
     * 해결 방법:
     * 1. AddWeaponUI 호출을 타이머를 사용해 약간 지연시킴 (0.1초)
     * 2. 이를 통해 Child Actor 컴포넌트가 완전히 초기화된 후 UI 업데이트
     * 3. BeginPlay에서는 정상적으로 초기화되었지만, 무기 전환 시에만 발생하던 문제 해결
     */

	if (!CanSwitchWeapon() || IsPrimaryEquip)
        return;

    bCanSwitchWeapon = false;
    IsPistolEquip = false;
    
    LocomotionBP->LeftHandIKOffset = WeaponSystem->RifleData->LeftHandIKOffset;

    WeaponSystem->Pistol_State(WeaponSystem->PistolData->WeaponClass, EAnimationState::Pistol, EWeaponState::Unequip, FName(""), FName("PistolHost_Socket"));
    IsPrimaryEquip = true;

    // 무기 상태 변경 후 약간의 지연을 두고 UI 업데이트
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        [this]()
        {
            AddWeaponUI(WeaponSystem->RifleData);
        },
        0.1f,  // Child Actor가 완전히 초기화되도록 지연
        false
    );

    WeaponSystem->Rifle_State(WeaponSystem->RifleData->WeaponClass, EAnimationState::RifleShotgun, EWeaponState::Equip, FName("Rifle_Socket"), FName(""));

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

	// 무기 상태 변경 후 약간의 지연을 두고 UI 업데이트
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			// Add Weapon UI
			AddWeaponUI(WeaponSystem->PistolData);
		},
		0.1f,  // 0.1초 지연
		false
	);

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

void ATPSTemplateCharacter::ClearWeaponUI()
{
	if (!CurrentWeaponUI)
		return;
	CurrentWeaponUI->RemoveFromParent();
}

UUserWidget* ATPSTemplateCharacter::AddWeaponUI(UWeaponDataAsset* WeaponData)
{
	if (!IsPlayerControlled())
		return nullptr;
	ClearWeaponUI();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		UUserWidget* WeaponUI = CreateWidget<UUserWidget>(PlayerController, WeaponSystem->RifleData->WeaponUI);
		if (WeaponUI)
		{
			UE_LOG(LogTemp, Warning, TEXT("Successed to create Dynamic Weapon HUD"));
			CurrentWeaponUI = Cast<UW_DynamicWeaponHUD>(WeaponUI);
			if (!CurrentWeaponUI)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to Initialize CurrentWeaponUI"));
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
			if (!CurrentWeapon)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to Initialize CurrentWeapon###2"));
				return nullptr;
			}

			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Current Ammo: %d"), CurrentWeapon->WeaponSystem->Weapon_Details.Weapon_Data.CurrentAmmo));
			CurrentWeaponUI->SetWeaponData(WeaponData->WeaponUITexture,
				WeaponData->WeaponName,
				CurrentWeapon->WeaponSystem->Weapon_Details.Weapon_Data.MaxAmmo,
				CurrentWeapon->WeaponSystem->Weapon_Details.Weapon_Data.CurrentAmmo);

			CurrentWeaponUI->AddToViewport();

			return CurrentWeaponUI;
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("Failed to create Dynamic Weapon HUD"));
	}
	
	return nullptr;
}

void ATPSTemplateCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// After Montage Ended
	if (!bInterrupted)
	{
		// Delay Montage
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				bCanSwitchWeapon = true;
			},
			0.25f,  // 0.25sec delay
			false
		);
	}
}

void ATPSTemplateCharacter::ShootFire(const FInputActionValue& Value)
{
	bFiring = Value.Get<bool>();
    HandleFiring();
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

bool ATPSTemplateCharacter::CanFire()
{
	
	bool bCanShoot = !IsSprint && !IsDodging && CanJump();
	if (bCanShoot || IsCrouch)
		return true;
	else
		return false;
}


void ATPSTemplateCharacter::StopFire()
{
    bFiring = false;
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
		AActor* CurrentWeaponActor = PrimaryChild->GetChildActor();
		AMasterWeapon* MasterWeapon = Cast<AMasterWeapon>(CurrentWeaponActor);
		UWeaponDataAsset* CurrentWeaponDataAsset = WeaponSystem->RifleData;
		ReadyToFire(MasterWeapon, CurrentWeaponDataAsset);
    }
	else if(IsPistolEquip)
	{
		AActor* CurrentWeaponActor = HandgunChild->GetChildActor();
		AMasterWeapon* MasterWeapon = Cast<AMasterWeapon>(CurrentWeaponActor);
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

	CurrentWeaponUI->UpdateAmmoCount(MasterWeapon->WeaponSystem->Weapon_Details.Weapon_Data.MaxAmmo,
		MasterWeapon->WeaponSystem->Weapon_Details.Weapon_Data.CurrentAmmo);

	// Process next shot based on fire mode
	float FireDelay = CurrentWeaponDataAsset->FireRate;
	EFireMode CurrentFireMode = CurrentWeaponDataAsset->FireMode;

	switch (CurrentFireMode)
	{
	case EFireMode::FullAuto:
		// Auto-fire is scheduled with a timer
		FTimerHandle TimerHandle;
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
	}
}
