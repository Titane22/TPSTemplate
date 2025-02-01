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
	bUseControllerRotationYaw = false;    // 좌우 회전 비활성화
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

	if (PrimaryChild)
	{
		PrimaryChild->SetChildActorClass(AWeapon_AssultRifle::StaticClass());
		PrimaryChild->SetupAttachment(Primary);
		UE_LOG(LogTemp, Warning, TEXT("Successed PrimaryChild"));
	}
	if (HandgunChild)
	{
		HandgunChild->SetChildActorClass(AAWeapon_Handgun::StaticClass());
		HandgunChild->SetupAttachment(Handgun);
		UE_LOG(LogTemp, Warning, TEXT("Successed HandgunChild"));
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

	// 네트워크 복제 설정
	// 멀티플레이어 동기화를 위한 설정
	SetReplicates(true);                                        // 액터 복제 활성화
	SetReplicateMovement(true);                                 // 움직임 복제 활성화

	ShoulderYOffset = 50.0f;
	ShoulderZOffset = 20.0f;
	TargetArmLengths = FVector(200.0f, 100.0f, 0.0f);
}

void ATPSTemplateCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 무기 소켓 부착 및 초기화
	if (Primary)
	{
		Primary->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("RifleHost_Socket"));
		UE_LOG(LogTemp, Warning, TEXT("Successed RifleHost_Socket"));
	}
	if (Handgun)
	{
		Handgun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("PistolHost_Socket"));
		UE_LOG(LogTemp, Warning, TEXT("Successed PistolHost_Socket"));
	}
	
	// Sequence 1
	WeaponSystem->CharacterRef = this;
	HealthComponent->CharacterRef = this;

	// Sequence 2
	CameraBoom->SocketOffset = FVector(0.0f, ShoulderYOffset, ShoulderZOffset);
	CameraBoom->TargetArmLength = TargetArmLengths.X;

	// Sequence 3
	// TODO: Create Widget
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

void ATPSTemplateCharacter::SwitchWeapons(const FInputActionValue& Value)
{

}
