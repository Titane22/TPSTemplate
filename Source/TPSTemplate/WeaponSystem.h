// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Public/AnimationState.h"
#include "WeaponSystem.generated.h"

class USkeletalMesh;
class AMasterWeapon;
class ATPSTemplateCharacter;
class UWeaponDataAsset;
class UUserWidget;

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

	// 서버에서 실행될 Rifle State 함수
	/*UFUNCTION(Server, Reliable)
	void Rifle_State(AMasterWeapon* InMasterWeapon, 
					EAnimationState InAnimState, 
					EWeaponState InWeaponState,
					FName InEquipSocketName, 
					FName InUnEquipSocketName);*/

	// 클라이언트에서 실행될 Rifle State 함수
	/*UFUNCTION(Client, Reliable)
	void Rifle_State_Client(AMasterWeapon* InMasterWeapon, 
						  EAnimationState InAnimState, 
						  EWeaponState InWeaponState,
						  FName InEquipSocketName, 
						  FName InUnEquipSocketName);*/

	// 서버에서 실행될 Pistol State 함수
	/*UFUNCTION(Server, Reliable)
	void Pistol_State(AMasterWeapon* InMasterWeapon, 
					 EAnimationState InAnimState, 
					 EWeaponState InWeaponState,
					 FName InEquipSocketName, 
					 FName InUnEquipSocketName);*/

	// 클라이언트에서 실행될 Pistol State 함수
	/*UFUNCTION(Client, Reliable)
	void Pistol_State_Client(AMasterWeapon* InMasterWeapon, 
							 EAnimationState InAnimState, 
							 EWeaponState InWeaponState,
							 FName InEquipSocketName, 
							 FName InUnEquipSocketName);*/

	UFUNCTION()
	void RifleEquip(FName SocketName);

	UFUNCTION()
	void RifleUnequip(FName SocketName);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USkeletalMesh* WeaponMesh;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	AMasterWeapon* MasterWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EAnimationState AnimationState;

	// TODO: Weapon Detail

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bIsDryAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UWeaponDataAsset* PistolData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UWeaponDataAsset* RifleData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UUserWidget* CrosshairWidget;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	ATPSTemplateCharacter* CharacterRef;
};
