// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Public/AnimationState.h"
#include "MasterWeapon.generated.h"

class UWeaponDataAsset;
class ATPSTemplateCharacter; 
class ATPSTemplate_Player;
class AIWeaponPickup;

UCLASS()
class TPSTEMPLATE_API AMasterWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMasterWeapon();

	virtual void Fire();
	virtual void Reload();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// 컴포넌트들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWeaponSystem* WeaponSystem;

	// Interaction Weapon Pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AIWeaponPickup> WeaponPickupClass;  

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EAnimationState WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class UWeaponDataAsset* WeaponData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bReloading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bAutoReload;

	// Hit 처리 함수
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool ApplyHit(const FHitResult HitResult, bool& ValidHit);

private:
	// 컴포넌트 초기화 함수
	void InitializeComponents();
};
