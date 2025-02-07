// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../MasterWeapon.h"
#include "AnimationState.h"
#include "Weapon_AssultRifle.generated.h"

/**
 * 
 */
UCLASS()
class TPSTEMPLATE_API AWeapon_AssultRifle : public AMasterWeapon
{
	GENERATED_BODY()
public:
	AWeapon_AssultRifle();

	virtual void Fire() override;
	virtual void BeginPlay() override;
	virtual void Reload() override;
protected:
	USceneComponent* Muzzle;
private:
	void FireBullet(FHitResult Hit, bool bReturnHit);
	
	void FireFX();

	void RandPointInCircle(float Radius, float& PointX, float& PointY);
};
