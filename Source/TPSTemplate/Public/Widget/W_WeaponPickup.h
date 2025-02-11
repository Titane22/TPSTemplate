// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "W_WeaponPickup.generated.h"

/**
 * 
 */
UCLASS()
class TPSTEMPLATE_API UW_WeaponPickup : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Weapon", meta = (DisplayName = "SetWeaponBrush"))
	void SetWeaponBrush(UTexture2D* ToSetTexture);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Weapon", meta = (DisplayName = "SetWeaponName"))
	void SetToPickupWeaponName(const FString& ToSetWeaponName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UTextBlock* WeaponNameRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UImage* WeaponTextureRef;
};
