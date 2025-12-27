// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/EquipmentSystem.h"
#include "Weapon/MasterWeapon.h"
#include "Characters/TPSTemplateCharacter.h"
#include "Data/WeaponData.h"
#include "Components/WeaponSystem.h"

// Sets default values for this component's properties
UEquipmentSystem::UEquipmentSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentEquippedSlot = EWeaponSlot::None;
}

void UEquipmentSystem::BeginPlay()
{
	Super::BeginPlay();

	// 무기 클래스는 Blueprint에서 설정됩니다 (Data-Driven)
	if (!PrimaryWeaponClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::BeginPlay - PrimaryWeaponClass is not assigned in Blueprint"));
	}
	if (!HandgunWeaponClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::BeginPlay - HandgunWeaponClass is not assigned in Blueprint"));
	}
}

void UEquipmentSystem::SetWeaponState(TSubclassOf<AMasterWeapon> ToSetWeaponClass, EAnimationState ToSetAnimation,
	EWeaponState CurWeaponState, FName ToSetEquipSocketName, FName ToSetUnequipSocketName, EWeaponSlot WeaponSlot)
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Error, TEXT("UEquipmentSystem::SetWeaponState - CharacterRef is null"));
		return;
	}

	CurrentWeaponClass = ToSetWeaponClass;
	AnimationState = ToSetAnimation;

	switch (CurWeaponState)
	{
	case EWeaponState::Equip:
		EquipWeapon(ToSetEquipSocketName, WeaponSlot);
		break;
	case EWeaponState::Unequip:
		UnequipWeapon(ToSetUnequipSocketName, WeaponSlot);
		break;
	}
}

void UEquipmentSystem::EquipWeapon(FName SocketName, EWeaponSlot WeaponSlot)
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::EquipWeapon CharacterRef is Null"));
		return;
	}

	// WeaponSlot에 따라 적절한 ChildActorComponent 선택
	UChildActorComponent* TargetChild = (WeaponSlot == EWeaponSlot::Primary)
		? CharacterRef->PrimaryChild
		: CharacterRef->HandgunChild;

	if (!TargetChild)
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::EquipWeapon TargetChild is Null"));
		return;
	}

	// 무기가 이미 존재하는지 확인 (BeginPlay에서 생성됨)
	if (!TargetChild->GetChildActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::EquipWeapon ChildActor is Null"));
		return;
	}

	// 소켓에 부착 (무기를 재생성하지 않고 이동만)
	TargetChild->AttachToComponent(
		CharacterRef->GetMesh(),
		FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			true
		),
		SocketName
	);

	CharacterRef->CurrentAnimationState = AnimationState;
}

void UEquipmentSystem::SwitchToWeapon(EWeaponSlot TargetSlot)
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Error, TEXT("EquipmentSystem::SwitchToWeapon - CharacterRef is null"));
		return;
	}

	// 이미 같은 무기를 들고 있으면 무시
	if (CurrentEquippedSlot == TargetSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("Already equipped %d"), (int32)TargetSlot);
		return;
	}

	// 1. 현재 장착된 무기를 등에 보관
	if (CurrentEquippedSlot != EWeaponSlot::None)
	{
		UChildActorComponent* CurrentChild = (CurrentEquippedSlot == EWeaponSlot::Primary)
			? CharacterRef->PrimaryChild
			: CharacterRef->HandgunChild;

		FName HolsterSocket = (CurrentEquippedSlot == EWeaponSlot::Primary)
			? FName("RifleHost_Socket")
			: FName("PistolHost_Socket");

		if (CurrentChild && CurrentChild->GetChildActor())
		{
			CurrentChild->AttachToComponent(
				CharacterRef->GetMesh(),
				FAttachmentTransformRules(
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					true
				),
				HolsterSocket
			);
		}
	}

	// 2. 새 무기를 손에 장착
	if (TargetSlot != EWeaponSlot::None)
	{
		UChildActorComponent* TargetChild = (TargetSlot == EWeaponSlot::Primary)
			? CharacterRef->PrimaryChild
			: CharacterRef->HandgunChild;

		FName HandSocket = (TargetSlot == EWeaponSlot::Primary)
			? FName("Rifle_Socket")
			: FName("Pistol_Socket");

		EAnimationState NewAnimState = (TargetSlot == EWeaponSlot::Primary)
			? EAnimationState::RifleShotgun
			: EAnimationState::Pistol;

		if (TargetChild && TargetChild->GetChildActor())
		{
			TargetChild->AttachToComponent(
				CharacterRef->GetMesh(),
				FAttachmentTransformRules(
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					true
				),
				HandSocket
			);

			CharacterRef->CurrentAnimationState = NewAnimState;
		}
	}
	else
	{
		// 맨손 상태
		CharacterRef->CurrentAnimationState = EAnimationState::Unarmed;
	}

	// 3. 상태 업데이트
	CurrentEquippedSlot = TargetSlot;

	UE_LOG(LogTemp, Log, TEXT("Switched to weapon slot: %d"), (int32)TargetSlot);
}

void UEquipmentSystem::UnequipWeapon(FName SocketName, EWeaponSlot WeaponSlot)
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::UnequipWeapon CharacterRef is Null"));
		return;
	}

	// WeaponSlot에 따라 적절한 ChildActorComponent 선택
	UChildActorComponent* TargetChild = (WeaponSlot == EWeaponSlot::Primary)
		? CharacterRef->PrimaryChild
		: CharacterRef->HandgunChild;

	if (!TargetChild)
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::UnequipWeapon TargetChild is Null"));
		return;
	}

	// 무기가 이미 존재하는지 확인
	if (!TargetChild->GetChildActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::UnequipWeapon ChildActor is Null"));
		return;
	}

	// 소켓에 부착 (무기를 재생성하지 않고 이동만)
	TargetChild->AttachToComponent(
		CharacterRef->GetMesh(),
		FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			true
		),
		SocketName
	);

	CharacterRef->CurrentAnimationState = AnimationState;
}
//
// void UEquipmentSystem::SwitchToWeapon(EWeaponSlot TargetSlot)
// {
// 	if (!CharacterRef)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("EquipmentSystem::SwitchToWeapon - CharacterRef is null"));
// 		return;
// 	}
//
// 	// 이미 같은 무기를 들고 있으면 무시
// 	if (CurrentEquippedSlot == TargetSlot)
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("Already equipped %d"), (int32)TargetSlot);
// 		return;
// 	}
//
// 	// 1. 현재 장착된 무기를 등에 보관
// 	if (CurrentEquippedSlot != EWeaponSlot::None)
// 	{
// 		UChildActorComponent* CurrentChild = (CurrentEquippedSlot == EWeaponSlot::Primary)
// 			? CharacterRef->PrimaryChild
// 			: CharacterRef->HandgunChild;
//
// 		FName HolsterSocket = (CurrentEquippedSlot == EWeaponSlot::Primary)
// 			? FName("RifleHost_Socket")
// 			: FName("PistolHost_Socket");
//
// 		if (CurrentChild && CurrentChild->GetChildActor())
// 		{
// 			CurrentChild->AttachToComponent(
// 				CharacterRef->GetMesh(),
// 				FAttachmentTransformRules(
// 					EAttachmentRule::SnapToTarget,
// 					EAttachmentRule::SnapToTarget,
// 					EAttachmentRule::SnapToTarget,
// 					true
// 				),
// 				HolsterSocket
// 			);
// 		}
// 	}
//
// 	// 2. 새 무기를 손에 장착
// 	if (TargetSlot != EWeaponSlot::None)
// 	{
// 		UChildActorComponent* TargetChild = (TargetSlot == EWeaponSlot::Primary)
// 			? CharacterRef->PrimaryChild
// 			: CharacterRef->HandgunChild;
//
// 		FName HandSocket = (TargetSlot == EWeaponSlot::Primary)
// 			? FName("Rifle_Socket")
// 			: FName("Pistol_Socket");
//
// 		EAnimationState NewAnimState = (TargetSlot == EWeaponSlot::Primary)
// 			? EAnimationState::RifleShotgun
// 			: EAnimationState::Pistol;
//
// 		if (TargetChild && TargetChild->GetChildActor())
// 		{
// 			TargetChild->AttachToComponent(
// 				CharacterRef->GetMesh(),
// 				FAttachmentTransformRules(
// 					EAttachmentRule::SnapToTarget,
// 					EAttachmentRule::SnapToTarget,
// 					EAttachmentRule::SnapToTarget,
// 					true
// 				),
// 				HandSocket
// 			);
//
// 			CharacterRef->CurrentAnimationState = NewAnimState;
// 		}
// 	}
// 	else
// 	{
// 		// 맨손 상태
// 		CharacterRef->CurrentAnimationState = EAnimationState::Unarmed;
// 	}
//
// 	// 3. 상태 업데이트
// 	CurrentEquippedSlot = TargetSlot;
//
// 	UE_LOG(LogTemp, Log, TEXT("Switched to weapon slot: %d"), (int32)TargetSlot);
// }
