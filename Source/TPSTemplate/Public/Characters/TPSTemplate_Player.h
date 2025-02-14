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
	
private:
	class UW_DivisionHUD* CurrentHUD;

protected:
	ATPSTemplate_Player();

	virtual void BeginPlay();

	UFUNCTION()
	UUserWidget* AddWeaponUI(UWeaponDataAsset* WeaponData);

	virtual void SwitchWeapons() override;

	virtual void SwitchToPrimaryWeapon() override;

	virtual void SwitchToHandgunWeapon() override;

	virtual void ReadyToFire(class AMasterWeapon* MasterWeapon, class UWeaponDataAsset* CurrentWeaponDataAsset);

	virtual void Interact() override;

	UFUNCTION()
	void ClearWeaponUI();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
