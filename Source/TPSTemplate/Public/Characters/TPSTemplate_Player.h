// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../TPSTemplateCharacter.h"
#include "TPSTemplate_Player.generated.h"

/**
 * 
 */
UCLASS()
class TPSTEMPLATE_API ATPSTemplate_Player : public ATPSTemplateCharacter
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay();

	UFUNCTION()
	UUserWidget* AddWeaponUI(UWeaponDataAsset* WeaponData);

	virtual void SwitchWeapons() override;

	virtual void SwitchToPrimaryWeapon() override;

	virtual void SwitchToHandgunWeapon() override;

	virtual void ReadyToFire(class AMasterWeapon* MasterWeapon, class UWeaponDataAsset* CurrentWeaponDataAsset);

	UFUNCTION()
	void ClearWeaponUI();
};
