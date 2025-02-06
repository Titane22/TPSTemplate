// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Public/AnimationState.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "WeaponSystem.generated.h"

class USkeletalMesh;
class AMasterWeapon;
class ATPSTemplateCharacter;
class UWeaponDataAsset;
class UUserWidget;
class UNiagaraSystem;
class USceneComponent;

USTRUCT(BlueprintType)
struct FWeapon_Data
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 CurrentAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 MaxAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 ClipAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 DifferentAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 Ammo_Count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool ShortGun_Trace = false;
};

USTRUCT(BlueprintType)
struct FWeapon_Details
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	struct FWeapon_Data Weapon_Data = {
		/* CurrentAmmo */ 0,
		/* MaxAmmo */ 0,
		/* ClipAmmo */ 0,
		/* DifferentAmmo */ 0,
		/* Ammo_Count */ 0,
		/* ShortGun_Trace */ false
	};
};

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Equip       UMETA(DisplayName = "Equip"),
	Unequip     UMETA(DisplayName = "Unequip")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSTEMPLATE_API UWeaponSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponSystem();

	UFUNCTION()
	void Rifle_State(AMasterWeapon* toSetMasterWeapon, EAnimationState toSetAnimationState, EWeaponState curWeaponState, FName toSetEquipSocketName, FName toSetUnequipSocketName);

	UFUNCTION()
	void Pistol_State(AMasterWeapon* toSetMasterWeapon, EAnimationState toSetAnimationState, EWeaponState curWeaponState, FName toSetEquipSocketName, FName toSetUnequipSocketName);

	UFUNCTION()
	void RifleEquip(FName SocketName);

	UFUNCTION()
	void RifleUnequip(FName SocketName);

	UFUNCTION()
	void PistolEquip(FName SocketName);

	UFUNCTION()
	void PistolUnequip(FName SocketName);

	bool FireCheck(int32 AmmoCount);

	void FireFX(USoundBase* Sound, FVector Location, USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings);

	void MuzzleVFX(UNiagaraSystem* SystemTemplate, USceneComponent* AttachToComponent);

	void FireMontage(UAnimMontage* PistolAnim, UAnimMontage* RifleAnim);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USkeletalMesh* WeaponMesh;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	AMasterWeapon* MasterWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EAnimationState AnimationState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bIsDryAmmo;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UUserWidget* CrosshairWidget;

	UPROPERTY()
	FWeapon_Details Weapon_Details = {
		FWeapon_Data{
			/* CurrentAmmo */ 32,
			/* MaxAmmo */ 90,
			/* ClipAmmo */ 32,
			/* DifferentAmmo */ 90,
			/* Ammo_Count */ 1,
			/* ShortGun_Trace */ false
		}
	};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	ATPSTemplateCharacter* CharacterRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UWeaponDataAsset* PistolData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UWeaponDataAsset* RifleData;
};
