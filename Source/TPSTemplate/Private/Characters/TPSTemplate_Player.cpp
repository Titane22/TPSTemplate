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
#include "../Public/Widget/W_DivisionHUD.h"
#include "Library/InteractiveType.h"
#include "Weapon/Interactor.h"
#include "Weapon/IWeaponPickup.h"

ATPSTemplate_Player::ATPSTemplate_Player()
	:Super()
{

}

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

		CurrentHUD = CreateWidget<UW_DivisionHUD>(PlayerController,
			LoadClass<UW_DivisionHUD>(nullptr, TEXT("/Game/ThirdPerson/Blueprints/W_DivisionHUD.W_DivisionHUD_C")));
		if (CurrentHUD)
		{
			Cast<UUserWidget>(CurrentHUD)->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("Successfully created and added CurrentHUD to viewport"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create CurrentHUD widget"));
		}
	}

}

void ATPSTemplate_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent); 
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Triggered, this, &ATPSTemplateCharacter::Interact);
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
	/* 무기 전환 시 타이밍 이슈 해결
	 *
	 * 문제 상황:
	 * 1. AddWeaponUI 함수와 무기 상태 변경(Weapon_State)이 동시에 호출됨
	 * 2. Weapon_State에서 Child Actor 컴포넌트의 생성이 비동기적으로 이루어짐
	 * 3. 그 결과 AddWeaponUI가 호출될 때 GetChildActor()가 아직 NULL을 반환
	 *
	 * 해결 방안:
	 * 1. AddWeaponUI 호출을 타이머로 약간의 지연시킴 (0.1초)
	 * 2. 이를 통해 Child Actor 컴포넌트가 완전히 초기화된 후 UI 업데이트
	 * 3. BeginPlay에서는 정상적으로 초기화되었으나, 무기 전환 시에만 발생하는 문제 해결
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

	// 무기 전환 시 타이머로 약간의 지연시킴 (0.1초)
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			AddWeaponUI(WeaponSystem->RifleData);
		},
		0.1f,  // Child Actor 초기화될 때까지 대기
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

	// 무기 전환 시 타이머로 약간의 지연시킴 (0.1초)
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			// Add Weapon UI
			AddWeaponUI(WeaponSystem->PistolData);
		},
		0.1f,  // 0.1초 대기
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

void ATPSTemplate_Player::Interact()
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
				UE_LOG(LogTemp, Warning, TEXT("ATPSTemplate_Player::Interact()::AnimInstance is NULL"));
				return;
			}

			switch (PickupToWeapon->WeaponData->WeaponType)
			{
			case EWeaponType::Pistol:
				MasterWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
				if (!MasterWeapon)
				{
					UE_LOG(LogTemp, Warning, TEXT("Interact()::EWeaponType::Pistol::MasterWeapon Is NULL"));
					return;
				}
				
				GetWorld()->SpawnActor<AActor>(
					MasterWeapon->WeaponPickupClass,
					InteractionSpawnTransform
				);

				if (IsPrimaryEquip)
				{
					IsPrimaryEquip = false;
					WeaponSystem->Rifle_State(
						WeaponSystem->RifleData->WeaponClass,
						EAnimationState::Unarmed,
						EWeaponState::Unequip,
						FName(""),
						FName("RifleHost_Socket")
					);
				}
				HandgunChild->SetChildActorClass(PickupToWeapon->WeaponData->WeaponClass->StaticClass());
				WeaponSystem->PistolData = PickupToWeapon->WeaponData;
				IsPistolEquip = true;

				AddWeaponUI(WeaponSystem->PistolData);
				LocomotionBP->LeftHandIKOffset = WeaponSystem->PistolData->LeftHandIKOffset;
				WeaponSystem->Pistol_State(
					WeaponSystem->PistolData->WeaponClass,
					EAnimationState::Pistol,
					EWeaponState::Equip,
					FName("Pistol_Socket"),
					FName("")
				);

				if (AnimInstance)
				{
					UAnimMontage* PistolEquipMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Pistol/Montages/MM_Pistol_Equip2")));
					// Play Montage
					AnimInstance->Montage_Play(PistolEquipMontage, 1.0f);

					// Set Montage End Delegate
					FOnMontageEnded CompleteDelegate;
					CompleteDelegate.BindUObject(this, &ATPSTemplateCharacter::OnMontageEnded);
					AnimInstance->Montage_SetEndDelegate(CompleteDelegate, PistolEquipMontage);

					InteractorComponent->StartInteraction(GetController());
				}
				break;
			case EWeaponType::RifleAndShotgun:
				MasterWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
				if (!MasterWeapon)
				{
					UE_LOG(LogTemp, Warning, TEXT("Interact()::EWeaponType::RifleAndShotgun::MasterWeapon Is NULL"));
					return;
				}
				if (!MasterWeapon)
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("MasterWeapon is NULL"));
				else if(!MasterWeapon->WeaponPickupClass)
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("WeaponPickupClass is NULL"));

				GetWorld()->SpawnActor<AActor>(
					MasterWeapon->WeaponPickupClass,
					InteractionSpawnTransform
				);
				if (IsPistolEquip)
				{
					IsPistolEquip = false;
					WeaponSystem->Pistol_State(
						WeaponSystem->PistolData->WeaponClass,
						EAnimationState::Unarmed,
						EWeaponState::Unequip,
						FName(""),
						FName("PistolHost_Socket")
					);
				}
				HandgunChild->SetChildActorClass(PickupToWeapon->WeaponData->WeaponClass->StaticClass());
				WeaponSystem->RifleData = PickupToWeapon->WeaponData;
				IsPrimaryEquip = true;

				AddWeaponUI(WeaponSystem->RifleData);
				LocomotionBP->LeftHandIKOffset = WeaponSystem->RifleData->LeftHandIKOffset;
				WeaponSystem->Rifle_State(
					WeaponSystem->RifleData->WeaponClass,
					EAnimationState::RifleShotgun,
					EWeaponState::Equip,
					FName("Rifle_Socket"),
					FName("")
				);

				if (AnimInstance) 
				{
					UAnimMontage* RifleEquipMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Rifle/Montages/MM_Rifle_Equip1")));
					// Play Montage
					AnimInstance->Montage_Play(RifleEquipMontage, 1.0f);

					// Set Montage End Delegate
					FOnMontageEnded CompleteDelegate;
					CompleteDelegate.BindUObject(this, &ATPSTemplateCharacter::OnMontageEnded);
					AnimInstance->Montage_SetEndDelegate(CompleteDelegate, RifleEquipMontage);

					InteractorComponent->StartInteraction(GetController());
				}
				
				break;
			}
		}
		break;
	}
}

void ATPSTemplate_Player::ClearWeaponUI()
{
	if (!CurrentWeaponUI)
		return;
	CurrentWeaponUI->RemoveFromParent();
}
