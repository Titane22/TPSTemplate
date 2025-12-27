// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Interaction.h"
#include "Components/WidgetComponent.h"
#include "IWeaponPickup.generated.h"


class AMasterWeapon;
class UWeaponData;
class UW_WeaponPickup;

/**
 * 
 */
UCLASS()
class TPSTEMPLATE_API AIWeaponPickup : public AInteraction
{
	GENERATED_BODY()
	
protected:
	AIWeaponPickup();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* Widget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* WeaponMesh;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void Interact(AController* Interactor);
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	AMasterWeapon* WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UWeaponData* WeaponData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UW_WeaponPickup* WidgetRef;
};
