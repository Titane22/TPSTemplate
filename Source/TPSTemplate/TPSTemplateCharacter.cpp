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
#include "./Public/Weapon/DA_Rifle.h"
#include "./Public/Weapon/DA_Pistol.h"
#include "Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ATPSTemplateCharacter

ATPSTemplateCharacter::ATPSTemplateCharacter()
{
	// Capsule Component 설정
	// 캐릭터의 기본 충돌 캡슐 크기 설정 (반지름, 높이)
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// 컨트롤러 회전 설정
	// 컨트롤러의 회전이 캐릭터에 직접 영향을 주지 않도록 설정
	bUseControllerRotationPitch = false;  // 상하 회전 비활성화
	bUseControllerRotationYaw = true;    // 좌우 회전 비활성화
	bUseControllerRotationRoll = false;   // 측면 회전 비활성화

	// Character Movement 설정
	// 캐릭터의 이동 및 회전 관련 속성 설정
	GetCharacterMovement()->bOrientRotationToMovement = true;     // 이동 방향으로 캐릭터 회전
	GetCharacterMovement()->bUseControllerDesiredRotation = false;// 컨트롤러 회전 사용 안함
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 540.0f);  // 회전 속도
	GetCharacterMovement()->JumpZVelocity = 400.f;               // 점프 세기
	GetCharacterMovement()->AirControl = 0.35f;                  // 공중 제어도
	GetCharacterMovement()->MaxWalkSpeed = 420.f;                // 최대 이동 속도
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;           // 최소 이동 속도
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f; // 정지시 감속도
	GetCharacterMovement()->MaxAcceleration = 1000.0f;           // 최대 가속도
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true; // 웅크린 상태에서 턱 이동 가능

	// Navigation 설정
	// AI 내비게이션 시스템을 위한 에이전트 속성 설정
	GetCharacterMovement()->NavAgentProps.AgentRadius = 42.0f;            // 에이전트 반경
	GetCharacterMovement()->NavAgentProps.AgentHeight = 192.0f;          // 에이전트 높이
	GetCharacterMovement()->NavAgentProps.AgentStepHeight = -1.0f;       // 에이전트 스텝 높이
	GetCharacterMovement()->NavAgentProps.NavWalkingSearchHeightScale = 0.5f;  // 경로 탐색 높이 스케일

	// 컴포넌트 생성
	// 게임플레이에 필요한 각종 컴포넌트 생성
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Primary = CreateDefaultSubobject<USceneComponent>(TEXT("Primary"));
	PrimaryChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("PrimaryChild"));
	Handgun = CreateDefaultSubobject<USceneComponent>(TEXT("Handgun"));
	HandgunChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("HandgunChild"));
	WeaponSystem = CreateDefaultSubobject<UWeaponSystem>(TEXT("WeaponSystem"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	// 컴포넌트 계층구조 설정
	// 각 컴포넌트의 부모-자식 관계 설정
	CameraBoom->SetupAttachment(RootComponent);
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Primary->SetupAttachment(RootComponent);
	Handgun->SetupAttachment(RootComponent);
	PrimaryChild->SetupAttachment(Primary);
	HandgunChild->SetupAttachment(Handgun);

	// 무기 클래스 설정
	if (PrimaryChild)
	{
		PrimaryChild->SetChildActorClass(AWeapon_AssultRifle::StaticClass());
	}
	if (HandgunChild)
	{
		HandgunChild->SetChildActorClass(AAWeapon_Handgun::StaticClass());
	}

	// Camera Boom 설정
	// 3인칭 카메라 시스템을 위한 스프링암 설정
	CameraBoom->TargetArmLength = 350.0f;                        // 카메라 거리
	CameraBoom->bUsePawnControlRotation = true;                  // 컨트롤러 기반 회전
	CameraBoom->SocketOffset = FVector(0.0f, 80.0f, 40.0f);      // 소켓 오프셋
	CameraBoom->bInheritRoll = false;                           // 롤 회전 상속 비활성화
	CameraBoom->bEnableCameraLag = true;                        // 카메라 지연 효과 활성화
	CameraBoom->bEnableCameraRotationLag = true;                // 카메라 회전 지연 활성화
	CameraBoom->CameraLagSpeed = 30.0f;                         // 카메라 지연 속도
	CameraBoom->CameraRotationLagSpeed = 30.0f;                 // 카메라 회전 지연 속도
	CameraBoom->CameraLagMaxDistance = 5.0f;                    // 최대 지연 거리
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f)); // 상대 위치
	CameraBoom->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f)); // 상대 회전
	CameraBoom->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));   // 상대 스케일

	// Follow Camera 설정
	// 실제 카메라 컴포넌트 설정
	FollowCamera->bUsePawnControlRotation = false;               // 폰 회전과 독립적으로 설정

	ShoulderYOffset = 50.0f;
	ShoulderZOffset = 20.0f;
	TargetArmLengths = FVector(200.0f, 100.0f, 0.0f);
}

void ATPSTemplateCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize UI widgets
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		// Initialize Current Weapon UI
		CurrentWeaponUI = CreateWidget<UUserWidget>(PlayerController, 
			LoadClass<UUserWidget>(nullptr, TEXT("/Game/ThirdPerson/Blueprints/W_ShooterHUD.W_ShooterHUD_C")));
		
		if (CurrentWeaponUI)
		{
			CurrentWeaponUI->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("Successfully created and added CurrentWeaponUI to viewport"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create CurrentWeaponUI widget"));
		}

		// Initialize Crosshair UI
		/*UICrosshair = CreateWidget<UUserWidget>(PlayerController, 
			LoadClass<UUserWidget>(nullptr, TEXT("/Game/ThirdPerson/Blueprints/WeaponBP/Crosshair/UI_Crosshair.UI_Crosshair_C")));
		
		if (UICrosshair)
		{
			UICrosshair->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("Successfully created and added UICrosshair to viewport"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create UICrosshair widget"));
		}*/
	}

	// MEMO: When initialized in the constructor instead of BeginPlay, 
	// RifleData becomes NULL again
	WeaponSystem->RifleData = NewObject<UDA_Rifle>();
	WeaponSystem->PistolData = NewObject<UDA_Pistol>();

	if (Primary)
	{
		Primary->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("RifleHost_Socket"));
		TArray<USceneComponent*> AttachedChildren = Primary->GetAttachChildren();
		if (AttachedChildren.Num() > 0)
		{
			PrimaryChild = Cast<UChildActorComponent>(AttachedChildren[0]);
			if (PrimaryChild)
			{
				UE_LOG(LogTemp, Warning, TEXT("Successed PrimaryChild"));
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
				UE_LOG(LogTemp, Warning, TEXT("Successed HandgunChild"));
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
		UUserWidget* ShooterHUD = CreateWidget<UUserWidget>(PlayerController, 
			LoadClass<UUserWidget>(nullptr, TEXT("/Game/ThirdPerson/Blueprints/W_ShooterHUD.W_ShooterHUD_C")));
		
		if (ShooterHUD)
		{
			ShooterHUD->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("Successfully created and added ShooterHUD to viewport"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create ShooterHUD widget"));
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
	
}

void ATPSTemplateCharacter::SwitchToPrimaryWeapon()
{
	if (!CanSwitchWeapon() && IsPrimaryEquip)
		return;

	bCanSwitchWeapon = false;
	IsPistolEquip = false;
	
	LocomotionBP->LeftHandIKOffset = WeaponSystem->RifleData->LeftHandIKOffset;

	WeaponSystem->Pistol_State(WeaponSystem->PistolData->WeaponClass, EAnimationState::Pistol, EWeaponState::Unequip, FName(""), FName("PistolHost_Socket"));
	IsPrimaryEquip = true;

	// TODO: Add Weapon UI
	//AddWeaponUI(WeaponSystem->RifleData);
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
		// 몽타주 재생
        AnimInstance->Montage_Play(RifleEquipMontage, 1.0f);
        
        // 몽타주 완료 델리게이트 바인딩
        FOnMontageEnded CompleteDelegate;
        CompleteDelegate.BindUObject(this, &ATPSTemplateCharacter::OnMontageEnded);
        AnimInstance->Montage_SetEndDelegate(CompleteDelegate, RifleEquipMontage);
	}
	
}

void ATPSTemplateCharacter::SwitchToHandgunWeapon()
{
	if (!CanSwitchWeapon() && IsPistolEquip)
		return;

	bCanSwitchWeapon = false;
	IsPrimaryEquip = false;

	LocomotionBP->LeftHandIKOffset = WeaponSystem->PistolData->LeftHandIKOffset;

	WeaponSystem->Rifle_State(WeaponSystem->RifleData->WeaponClass, EAnimationState::RifleShotgun, EWeaponState::Unequip, FName(""), FName("RifleHost_Socket"));
	IsPrimaryEquip = true;

	// TODO: Add Weapon UI
	//AddWeaponUI(WeaponSystem->RifleData);
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
		// 몽타주 재생
		AnimInstance->Montage_Play(PistolEquipMontage, 1.0f);

		// 몽타주 완료 델리게이트 바인딩
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

	/*if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		UUserWidget* WeaponUI = CreateWidget(nullptr, WeaponSystem->RifleData->WeaponUI);
		if (WeaponUI)
		{
			UE_LOG(LogTemp, Warning, TEXT("Successed to create Dynamic Weapon HUD"));
			CurrentWeaponUI = WeaponUI;
			if (IsPrimaryEquip)
			{
				CurrentWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
			}
			else
			{
				CurrentWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
			}
			if(!CurrentWeapon)
				UE_LOG(LogTemp, Warning, TEXT("Failed to Initialize CurrentWeapon"));
			
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("Failed to create Dynamic Weapon HUD"));
	}*/
	
	return nullptr;
}

void ATPSTemplateCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 몽타주 재생이 끝난 후의 처리
	if (!bInterrupted)
	{
		// 딜레이 후 몽타주 재생
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				bCanSwitchWeapon = true;
			},
			0.25f,  // 0.25초 딜레이
			false
		);
	}
}

void ATPSTemplateCharacter::ShootFire(const FInputActionValue& Value)
{
	bFiring = Value.Get<bool>();
    HandleFiring();
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
        AMasterWeapon* MasterWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
        if (MasterWeapon)
        {
            bCanFire = false;
            MasterWeapon->Fire();

            // 발사 모드에 따른 다음 발사 처리
            float FireDelay = WeaponSystem->RifleData->FireRate;
            EFireMode CurrentFireMode = WeaponSystem->RifleData->FireMode;

            switch (CurrentFireMode)
            {
                case EFireMode::FullAuto:
                    // 자동 발사는 타이머로 다음 발사 예약
					FTimerHandle TimerHandle;
					GetWorld()->GetTimerManager().SetTimer(
						TimerHandle,
                        [this]()
                        {
                            bCanFire = true;
                            HandleFiring(); // 재귀적으로 발사 처리
							GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Fire!!!!!!!!!!!!!!!!!!!!!!!!!"));
                        },
                        FireDelay,
                        false
                    );
                    break;
            }
        }
    }
}
