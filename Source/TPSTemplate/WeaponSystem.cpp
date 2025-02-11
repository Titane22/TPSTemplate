// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSystem.h"
#include "MasterWeapon.h"
#include "TPSTemplateCharacter.h"
#include "WeaponDataAsset.h"
#include "Public/Weapon/DA_Rifle.h"
#include "Public/Weapon/DA_Pistol.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

// Sets default values for this component's properties
UWeaponSystem::UWeaponSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}

void UWeaponSystem::Rifle_State(AMasterWeapon* toSetMasterWeapon, EAnimationState toSetAnimationState, EWeaponState curWeaponState, FName toSetEquipSocketName, FName toSetUnequipSocketName)
{
	// 현재 무기의 상태를 저장
	FWeapon_Details CurrentDetails;
	if (AMasterWeapon* CurrentWeapon = Cast<AMasterWeapon>(CharacterRef->PrimaryChild->GetChildActor()))
	{
		if (CurrentWeapon->WeaponSystem)
		{
			CurrentDetails = CurrentWeapon->WeaponSystem->Weapon_Details;
		}
	}

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

	// 새로 생성된 무기에 이전 상태를 복원
	if (AMasterWeapon* NewWeapon = Cast<AMasterWeapon>(CharacterRef->PrimaryChild->GetChildActor()))
	{
		if (NewWeapon->WeaponSystem)
		{
			NewWeapon->WeaponSystem->Weapon_Details = CurrentDetails;
		}
	}
}

void UWeaponSystem::Pistol_State(AMasterWeapon* toSetMasterWeapon, EAnimationState toSetAnimationState, EWeaponState curWeaponState, FName toSetEquipSocketName, FName toSetUnequipSocketName)
{
	// 현재 무기의 상태를 저장
	FWeapon_Details CurrentDetails;
	if (AMasterWeapon* CurrentWeapon = Cast<AMasterWeapon>(CharacterRef->HandgunChild->GetChildActor()))
	{
		if (CurrentWeapon->WeaponSystem)
		{
			CurrentDetails = CurrentWeapon->WeaponSystem->Weapon_Details;
		}
	}

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

	// 새로 생성된 무기에 이전 상태를 복원
	if (AMasterWeapon* NewWeapon = Cast<AMasterWeapon>(CharacterRef->HandgunChild->GetChildActor()))
	{
		if (NewWeapon->WeaponSystem)
		{
			NewWeapon->WeaponSystem->Weapon_Details = CurrentDetails;
		}
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

bool UWeaponSystem::FireCheck(int32 AmmoCount)
{
	if(Weapon_Details.Weapon_Data.CurrentAmmo == 0)
		return false;
	Weapon_Details.Weapon_Data.CurrentAmmo = Weapon_Details.Weapon_Data.CurrentAmmo - AmmoCount;
	return true;
}

void UWeaponSystem::FireFX(USoundBase* Sound, FVector Location, USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings)
{
	if (!Sound)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::FireFX Sound is null"));
		return;
	}

	UGameplayStatics::SpawnSoundAtLocation(
		this,           // World context object
		Sound,          // Sound to play
		Location,       // Location to play sound at
		FRotator::ZeroRotator,  // Rotation (default to zero)
		1.0f,          // Volume multiplier
		1.0f,          // Pitch multiplier
		0.0f,          // Start time
		AttenuationSettings,    // Attenuation settings
		ConcurrencySettings     // Concurrency settings
	);
}

void UWeaponSystem::EmptyFX(USoundBase* Sound)
{
	UGameplayStatics::PlaySoundAtLocation(
		this,
		Sound,
		CharacterRef->GetActorLocation()
	);
}

void UWeaponSystem::MuzzleVFX(UNiagaraSystem* SystemTemplate, USceneComponent* AttachToComponent)
{
	if (!SystemTemplate || !AttachToComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MuzzleVFX: SystemTemplate or AttachToComponent is null"));
		return;
	}

	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		SystemTemplate,           // 나이아가라 시스템 템플릿
		AttachToComponent,       // 부착할 컴포넌트
		NAME_None,               // 소켓 이름 (None으로 설정)
		FVector(0, 0, 0),        // 위치 오프셋
		FRotator(0, 0, 0),       // 회전 오프셋
		EAttachLocation::KeepRelativeOffset,  // 위치 타입
		true,                    // Auto Activate
		true,                    // Auto Destroy
		ENCPoolMethod::None,     // Pooling Method
		true                     // Pre Cull Check
	);

	if (!NiagaraComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MuzzleVFX: Failed to spawn niagara component"));
	}
}

void UWeaponSystem::FireMontage(UAnimMontage* PistolAnim, UAnimMontage* RifleAnim)
{
	UAnimMontage* FireAnim = nullptr;

	switch (CharacterRef->CurrentAnimationState)
	{
	case EAnimationState::Pistol:
		FireAnim = PistolAnim;
		break;
	case EAnimationState::RifleShotgun:
		FireAnim = RifleAnim;
		break;
	}
	CharacterRef->GetMesh()->GetAnimInstance()->Montage_Play(FireAnim, 1.0f);
}

bool UWeaponSystem::CheckAmmo()
{
	bool bHasAmmo = Weapon_Details.Weapon_Data.MaxAmmo > 0;
	bool bCanReload = Weapon_Details.Weapon_Data.CurrentAmmo < Weapon_Details.Weapon_Data.ClipAmmo;
	return bHasAmmo && bCanReload;
}

float UWeaponSystem::ReloadMontage(UAnimMontage* PistolAnim, UAnimMontage* RifleAnim)
{
	UAnimMontage* ReloadAnim = nullptr;
	switch (CharacterRef->CurrentAnimationState)
	{
	case EAnimationState::Pistol:
		ReloadAnim = PistolAnim;
		break;
	case EAnimationState::RifleShotgun:
		ReloadAnim = RifleAnim;
		break;
	}

	return CharacterRef->GetMesh()->GetAnimInstance()->Montage_Play(ReloadAnim, 1.0f);
}

void UWeaponSystem::ReloadCheck()
{
	Weapon_Details.Weapon_Data;
	if (Weapon_Details.Weapon_Data.MaxAmmo > 0)
	{
		if (Weapon_Details.Weapon_Data.CurrentAmmo == 0)
		{
			if (Weapon_Details.Weapon_Data.MaxAmmo >= Weapon_Details.Weapon_Data.ClipAmmo)
			{
				Weapon_Details.Weapon_Data.CurrentAmmo = Weapon_Details.Weapon_Data.ClipAmmo;
				Weapon_Details.Weapon_Data.MaxAmmo = Weapon_Details.Weapon_Data.MaxAmmo - Weapon_Details.Weapon_Data.ClipAmmo;
			}
			else
			{
				Weapon_Details.Weapon_Data.CurrentAmmo = Weapon_Details.Weapon_Data.MaxAmmo;
				Weapon_Details.Weapon_Data.MaxAmmo = 0;
			}
		}
		else
		{
			if (Weapon_Details.Weapon_Data.MaxAmmo >= Weapon_Details.Weapon_Data.ClipAmmo)
			{
				Weapon_Details.Weapon_Data.MaxAmmo = Weapon_Details.Weapon_Data.MaxAmmo - (Weapon_Details.Weapon_Data.ClipAmmo - Weapon_Details.Weapon_Data.CurrentAmmo);
				Weapon_Details.Weapon_Data.CurrentAmmo = Weapon_Details.Weapon_Data.ClipAmmo;
			}
			else
			{
				int32 RemainAmmo = Weapon_Details.Weapon_Data.ClipAmmo - Weapon_Details.Weapon_Data.CurrentAmmo;
				if (RemainAmmo > Weapon_Details.Weapon_Data.MaxAmmo)
				{
					Weapon_Details.Weapon_Data.CurrentAmmo = Weapon_Details.Weapon_Data.CurrentAmmo + Weapon_Details.Weapon_Data.MaxAmmo;
					Weapon_Details.Weapon_Data.MaxAmmo = 0;
				}
				else
				{
					Weapon_Details.Weapon_Data.CurrentAmmo = Weapon_Details.Weapon_Data.CurrentAmmo + RemainAmmo;
					Weapon_Details.Weapon_Data.MaxAmmo = Weapon_Details.Weapon_Data.MaxAmmo - RemainAmmo;
				}
			}
		}
		FireCheck(0);
	}
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

