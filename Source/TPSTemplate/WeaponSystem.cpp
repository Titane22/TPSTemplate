// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSystem.h"
#include "MasterWeapon.h"
#include "TPSTemplateCharacter.h"
#include "WeaponDataAsset.h"
#include "Public/Weapon/DA_Rifle.h"
#include "Public/Weapon/DA_Pistol.h"

// Sets default values for this component's properties
UWeaponSystem::UWeaponSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}

void UWeaponSystem::Rifle_State(AMasterWeapon* toSetMasterWeapon, EAnimationState toSetAnimationState, EWeaponState curWeaponState,FName toSetEquipSocketName, FName toSetUnequipSocketName)
{
	MasterWeapon = toSetMasterWeapon;
	AnimationState = toSetAnimationState;

	switch (curWeaponState)
	{
	case EWeaponState::Equip:
		RifleEquip(toSetEquipSocketName);
		break;
	case EWeaponState::Unequip:
		RifleUnequip(toSetUnequipSocketName);
		break;
	}
}

void UWeaponSystem::Pistol_State(AMasterWeapon* toSetMasterWeapon, EAnimationState toSetAnimationState, EWeaponState curWeaponState, FName toSetEquipSocketName, FName toSetUnequipSocketName)
{
	MasterWeapon = toSetMasterWeapon;
	AnimationState = toSetAnimationState;

	switch (curWeaponState)
	{
	case EWeaponState::Equip:
		PistolEquip(toSetEquipSocketName);
		break;
	case EWeaponState::Unequip:
		PistolUnequip(toSetUnequipSocketName);
		break;
	}
}

void UWeaponSystem::RifleEquip(FName SocketName)
{
	// Check if CharacterRef is valid
	if (CharacterRef == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::RifleEquip CharacterRef is Null"));
		return;
	}

	// Get Primary Child Component and Cast
	if (!CharacterRef->PrimaryChild)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::RifleEquip PrimaryChild is Null"));
		return;
	}

	// Set Child Actor Class to MasterWeapon
	CharacterRef->PrimaryChild->SetChildActorClass(MasterWeapon->GetClass());

	UWeaponSystem* TargetWeaponSystem = Cast<UWeaponSystem>(CharacterRef->PrimaryChild->GetChildActor()->GetComponentByClass(this->GetClass()));
	if(!TargetWeaponSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::RifleEquip TargetWeaponSystem is Null"));
		return;
	}
	TargetWeaponSystem->CharacterRef = CharacterRef;
	
	CharacterRef->PrimaryChild->AttachToComponent(
		CharacterRef->GetMesh(),
		FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget,    // Location Rule
			EAttachmentRule::SnapToTarget,    // Rotation Rule
			EAttachmentRule::SnapToTarget,    // Scale Rule
			true                              // Weld Simulated Bodies
		),
		SocketName
	);

	CharacterRef->CurrentAnimationState = AnimationState;
}

void UWeaponSystem::RifleUnequip(FName SocketName)
{
	if (CharacterRef == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::RifleUnequip CharacterRef is Null"));
		return;
	}

	if (!CharacterRef->PrimaryChild)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::RifleUnequip PrimaryChild is Null"));
		return;
	}

	CharacterRef->PrimaryChild->SetChildActorClass(MasterWeapon->GetClass());

	CharacterRef->PrimaryChild->AttachToComponent(
		CharacterRef->GetMesh(),
		FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget,    // Location Rule
			EAttachmentRule::SnapToTarget,    // Rotation Rule
			EAttachmentRule::SnapToTarget,    // Scale Rule
			true                              // Weld Simulated Bodies
		),
		SocketName
	);

	CharacterRef->CurrentAnimationState = AnimationState;
}

void UWeaponSystem::PistolEquip(FName SocketName)
{
	if (CharacterRef == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::PistolEquip CharacterRef is Null"));
		return;
	}

	if (!CharacterRef->HandgunChild)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::PistolEquip HandgunChild is Null"));
		return;
	}

	// Child Actor Class를 MasterWeapon으로 설정
	CharacterRef->HandgunChild->SetChildActorClass(MasterWeapon->GetClass());

	UWeaponSystem* TargetWeaponSystem = Cast<UWeaponSystem>(CharacterRef->HandgunChild->GetChildActor()->GetComponentByClass(this->GetClass()));
	if (!TargetWeaponSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::PistolEquip TargetWeaponSystem is Null"));
		return;
	}
	TargetWeaponSystem->CharacterRef = CharacterRef;

	CharacterRef->HandgunChild->AttachToComponent(
		CharacterRef->GetMesh(),
		FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget,    // Location Rule
			EAttachmentRule::SnapToTarget,    // Rotation Rule
			EAttachmentRule::SnapToTarget,    // Scale Rule
			true                              // Weld Simulated Bodies
		),
		SocketName
	);

	CharacterRef->CurrentAnimationState = AnimationState;
}

void UWeaponSystem::PistolUnequip(FName SocketName)
{
	if (CharacterRef == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::PistolUnequip CharacterRef is Null"));
		return;
	}

	if (!CharacterRef->HandgunChild)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::PistolUnequip HandgunChild is Null"));
		return;
	}

	CharacterRef->HandgunChild->SetChildActorClass(MasterWeapon->GetClass());

	CharacterRef->HandgunChild->AttachToComponent(
		CharacterRef->GetMesh(),
		FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget,    // Location Rule
			EAttachmentRule::SnapToTarget,    // Rotation Rule
			EAttachmentRule::SnapToTarget,    // Scale Rule
			true                              // Weld Simulated Bodies
		),
		SocketName
	);

	CharacterRef->CurrentAnimationState = AnimationState;
}

// Called when the game starts
void UWeaponSystem::BeginPlay()
{
	Super::BeginPlay();

	// ...

	RifleData = NewObject<UDA_Rifle>();
	PistolData = NewObject<UDA_Pistol>();
}


// Called every frame
void UWeaponSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

