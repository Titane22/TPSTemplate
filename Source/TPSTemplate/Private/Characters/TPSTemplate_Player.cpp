// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/TPSTemplate_Player.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "../WeaponSystem.h"
#include "../HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Animation/LocomotionAnimInstance.h"
#include "../WeaponDataAsset.h"
#include "../Public/Weapon_AssultRifle.h"
#include "../Public/AWeapon_Handgun.h"
#include "../Public/Weapon/DA_Rifle.h"
#include "../Public/Weapon/DA_Pistol.h"
#include "../Public/Widget/W_DynamicWeaponHUD.h"

void ATPSTemplate_Player::BeginPlay()
{
	Super::BeginPlay();
	// MEMO: When initialized in the constructor instead of BeginPlay, 
	// RifleData becomes NULL again
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

	if (CrouchCurve)
	{
		// TODO: Apply Crouch Timeline
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

	if (CrouchCurve)
	{
		// TODO: Apply Crouch Timeline
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindUFunction(this, FName("ShoulderCameraChange"));
		ShoulderCameraTimeline.AddInterpFloat(CrouchCurve, TimelineCallback);
		ShoulderCameraTimeline.SetPlayRate(4.0f);
	}

	// Sequence 1
	WeaponSystem->CharacterRef = this;
	HealthComponent->CharacterRef = this;

	// Sequence 2
	CameraBoom->SocketOffset = FVector(0.0f, ShoulderYOffset, ShoulderZOffset);
	CameraBoom->TargetArmLength = TargetArmLengths.X;

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

UUserWidget* ATPSTemplate_Player::AddWeaponUI(UWeaponDataAsset* WeaponData)
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

			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Current Ammo: %d"), CurrentWeapon->WeaponSystem->Weapon_Details.Weapon_Data.CurrentAmmo));
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

void ATPSTemplate_Player::SwitchWeapons()
{
}

void ATPSTemplate_Player::SwitchToPrimaryWeapon()
{
	/* ���� ��ȯ �� Ÿ�̹� �̽� �ذ�
	 *
	 * ���� ����:
	 * 1. AddWeaponUI �Լ��� ���� ���� ����(Weapon_State)�� ���ÿ� ȣ���
	 * 2. Weapon_State���� Child Actor ������Ʈ�� ������ �񵿱������� �̷����
	 * 3. �� ��� AddWeaponUI�� ȣ��� �� GetChildActor()�� ���� NULL�� ��ȯ
	 *
	 * �ذ� ���:
	 * 1. AddWeaponUI ȣ���� Ÿ�̸Ӹ� ����� �ణ ������Ŵ (0.1��)
	 * 2. �̸� ���� Child Actor ������Ʈ�� ������ �ʱ�ȭ�� �� UI ������Ʈ
	 * 3. BeginPlay������ ���������� �ʱ�ȭ�Ǿ�����, ���� ��ȯ �ÿ��� �߻��ϴ� ���� �ذ�
	 */

	if (!CanSwitchWeapon() || IsPrimaryEquip)
		return;

	if (IsPistolEquip)
	{
		AMasterWeapon* MasterWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
	}
	bCanSwitchWeapon = false;
	IsPistolEquip = false;

	LocomotionBP->LeftHandIKOffset = WeaponSystem->RifleData->LeftHandIKOffset;

	WeaponSystem->Pistol_State(WeaponSystem->PistolData->WeaponClass, EAnimationState::Pistol, EWeaponState::Unequip, FName(""), FName("PistolHost_Socket"));
	IsPrimaryEquip = true;

	// ���� ���� ���� �� �ణ�� ������ �ΰ� UI ������Ʈ
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			AddWeaponUI(WeaponSystem->RifleData);
		},
		0.1f,  // Child Actor�� ������ �ʱ�ȭ�ǵ��� ����
		false
	);

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

void ATPSTemplate_Player::SwitchToHandgunWeapon()
{
	if (!CanSwitchWeapon() || IsPistolEquip)
		return;

	bCanSwitchWeapon = false;
	IsPrimaryEquip = false;

	LocomotionBP->LeftHandIKOffset = WeaponSystem->PistolData->LeftHandIKOffset;

	WeaponSystem->Rifle_State(WeaponSystem->RifleData->WeaponClass, EAnimationState::RifleShotgun, EWeaponState::Unequip, FName(""), FName("RifleHost_Socket"));
	IsPistolEquip = true;

	// ���� ���� ���� �� �ణ�� ������ �ΰ� UI ������Ʈ
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			// Add Weapon UI
			AddWeaponUI(WeaponSystem->PistolData);
		},
		0.1f,  // 0.1�� ����
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

void ATPSTemplate_Player::ReadyToFire(AMasterWeapon* MasterWeapon, UWeaponDataAsset* CurrentWeaponDataAsset)
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

void ATPSTemplate_Player::ClearWeaponUI()
{
	if (!CurrentWeaponUI)
		return;
	CurrentWeaponUI->RemoveFromParent();
}
