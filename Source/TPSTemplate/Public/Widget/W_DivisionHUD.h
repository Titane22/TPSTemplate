// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "W_DivisionHUD.generated.h"

/**
 * 
 */
UCLASS()
class TPSTEMPLATE_API UW_DivisionHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Weapon", meta = (DisplayName = "SetWeaponData"))
	void SetWeaponData(UTexture2D* Texture, const FString& WeaponName, int32 MaxAmmo, int32 CurrentClip);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Weapon", meta = (DisplayName = "UpdateAmmoCount"))
	void UpdateAmmoCount(int32 MaxAmmo, int32 CurrentClip);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Weapon", meta = (DisplayName = "ParseAmmoCount"))
	FString ParseAmmoCount(int32 AmmoCount);
};
