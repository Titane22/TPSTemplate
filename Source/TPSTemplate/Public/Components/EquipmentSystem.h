// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Library/AnimationState.h"
#include "EquipmentSystem.generated.h"

class AMasterWeapon;
class ATPSTemplateCharacter;
class UWeaponData;

UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	None        UMETA(DisplayName = "None"),      // 맨손 상태
	Primary     UMETA(DisplayName = "Primary"),
	Handgun     UMETA(DisplayName = "Handgun")
};

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Equip       UMETA(DisplayName = "Equip"),
	Unequip     UMETA(DisplayName = "Unequip")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSTEMPLATE_API UEquipmentSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UEquipmentSystem();

	// 최적화된 새 API: 한 번의 호출로 무기 전환
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SwitchToWeapon(EWeaponSlot TargetSlot);

	/**
	 * 새 무기 픽업 및 장착
	 * @param NewWeaponClass - 새로 장착할 무기 클래스
	 * @param TargetSlot - 장착할 슬롯 (Primary 또는 Handgun)
	 * @param OutDroppedWeaponClass - 드롭된 기존 무기 클래스 (출력)
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool PickupAndEquipWeapon(TSubclassOf<AMasterWeapon> NewWeaponClass, EWeaponSlot TargetSlot, TSubclassOf<AMasterWeapon>& OutDroppedWeaponClass);

	// 레거시 API (하위 호환성을 위해 유지)
	UFUNCTION()
	void SetWeaponState(TSubclassOf<AMasterWeapon> ToSetWeaponClass, EAnimationState ToSetAnimation, EWeaponState CurWeaponState, FName ToSetEquipSocketName, FName ToSetUnequipSocketName, EWeaponSlot WeaponSlot = EWeaponSlot::Primary);

	UFUNCTION()
	void EquipWeapon(FName SocketName, EWeaponSlot WeaponSlot);

	UFUNCTION()
	void UnequipWeapon(FName SocketName, EWeaponSlot WeaponSlot);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY()
	ATPSTemplateCharacter* CharacterRef;

	// Blueprint에서 무기 클래스를 설정하세요 (Data-Driven)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AMasterWeapon> PrimaryWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AMasterWeapon> HandgunWeaponClass;

	// 현재 장착된 무기 슬롯 (None = 맨손)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponSlot CurrentEquippedSlot;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AMasterWeapon> CurrentWeaponClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Animation", meta = (AllowPrivateAccess = "true"))
	EAnimationState AnimationState;
};
