// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/IWeaponPickup.h"
#include "IWeaponPickup_Pistol.generated.h"

/**
 * 
 */
UCLASS()
class TPSTEMPLATE_API AIWeaponPickup_Pistol : public AIWeaponPickup
{
	GENERATED_BODY()
protected:
	AIWeaponPickup_Pistol();

	virtual void BeginPlay() override;

};
