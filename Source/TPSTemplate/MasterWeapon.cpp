// Fill out your copyright notice in the Description page of Project Settings.

#include "MasterWeapon.h"
#include "WeaponSystem.h"
#include "TPSTemplateCharacter.h"
#include "HealthComponent.h" 
#include "WeaponDataAsset.h"
#include "./Weapon/DA_Pistol.h"
#include "./Weapon/DA_Rifle.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMasterWeapon::AMasterWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Create Components
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    WeaponSystem = CreateDefaultSubobject<UWeaponSystem>(TEXT("WeaponSystem"));

    // Setting up the component hierarchy
    RootComponent = DefaultSceneRoot;
    WeaponMesh->SetupAttachment(DefaultSceneRoot);

    WeaponType = EAnimationState::Unarmed;
    bReloading = false;
    bAutoReload = false;

    WeaponSystem->MasterWeapon = this;
    WeaponSystem->AnimationState = EAnimationState::Unarmed;
    WeaponSystem->Weapon_Details = {
        FWeapon_Data{
            /* CurrentAmmo */ 32,
            /* MaxAmmo */ 90,
            /* ClipAmmo */ 32,
            /* DifferentAmmo */ 90,
            /* Ammo_Count */ 1,
            /* ShortGun_Trace */ false
        }
    };

    WeaponSystem->bIsDryAmmo = false;
    WeaponSystem->PistolData = NewObject<UDA_Pistol>();
    WeaponSystem->RifleData = NewObject<UDA_Rifle>();

    WeaponSystem->CrosshairWidget = nullptr;
}

// Called when the game starts or when spawned
void AMasterWeapon::BeginPlay()
{
	Super::BeginPlay();
	
    ATPSTemplateCharacter* Player = Cast<ATPSTemplateCharacter>(GetAttachParentActor());
    if (!Player)
        return;
    WeaponSystem->CharacterRef = Player;
}

bool AMasterWeapon::ApplyHit(const FHitResult HitResult, bool& ValidHit)
{
    AActor* HitActor = HitResult.GetActor();
    
    // Hit된 액터가 있고, 그 액터가 유효한지 확인
    if (!HitActor)
    {
        ValidHit = false;
        return false;
    }
    // Hit된 액터에서 ActorComponent를 찾음
    UActorComponent* HitReceiverComponent = HitActor->GetComponentByClass(UActorComponent::StaticClass());
    if (!HitReceiverComponent)
    {
        ATPSTemplateCharacter* Player = Cast<ATPSTemplateCharacter>(HitActor);
        if (!Player)
        {
            // Apply Damage Inner
            UGameplayStatics::ApplyDamage(
                Player,                    // Damaged Actor
                WeaponData->Damage,        // Base Damage
                GetInstigatorController(), // Event Instigator
                this,                      // Damage Causer
                nullptr                    // Damage Type Class
            );
            ValidHit = false;
            return false;
        }

        if (!Player->Dead)
        {
            // Apply Damage
            Player->GetHealthComponent()->ApplyDamage(WeaponData->Damage);
            // TODO: Play Sound 2D
            ValidHit = true;
            return ValidHit;
        }
        // Component가 있는지 확인
        ValidHit = false;
        return ValidHit;
    }
    // TODO: Play Sound 2D

    ValidHit = true;
    return false;
}

void AMasterWeapon::Fire()
{
}

void AMasterWeapon::Reload()
{
}

