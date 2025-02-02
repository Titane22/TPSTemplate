// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/DA_Pistol.h"
#include "../Public/AWeapon_Handgun.h"

UDA_Pistol::UDA_Pistol()
	: Super()
{
    // Weapon Details
    WeaponName = TEXT("Pistol");
    WeaponType = EWeaponType::Pistol;
    WeaponClass = AAWeapon_Handgun::StaticClass()->GetDefaultObject<AMasterWeapon>();

    // Fire Mode Data
    FireMode = EFireMode::SemiAuto;
    BurstAmount = 1;
    FireRate = 0.1f;

    // Ballistics
    BulletSpread = 2.0f;
    Damage = 20.0f;
    MaxRange = 5000.0f;

    // Animation
    LeftHandIKOffset = FVector(0.0f, 0.0f, 0.0f);

    static ConstructorHelpers::FObjectFinder<UAnimMontage> BodyFireMontageAsset(TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Rifle/Montages/MM_Pistol_Fire_Montage"));
    static ConstructorHelpers::FObjectFinder<UAnimMontage> WeaponFireMontageAsset(TEXT("/Game/Weapons/Rifle/Animations/AM_Weap_Rifle_Fire"));
    static ConstructorHelpers::FObjectFinder<UAnimMontage> BodyReloadMontageAsset(TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Rifle/Montages/MM_Pistol_Reload_Montage"));
    static ConstructorHelpers::FObjectFinder<UAnimMontage> WeaponReloadMontageAsset(TEXT("/Game/Weapons/Rifle/Animations/AM_Weap_Rifle_Reload"));
    if (BodyFireMontageAsset.Succeeded())
    {
        BodyFireMontage = BodyFireMontageAsset.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load BodyFireMontage"));
    }
    if (WeaponFireMontageAsset.Succeeded())
    {
        WeaponFireMontage = WeaponFireMontageAsset.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load WeaponFireMontage"));
    }
    if (BodyReloadMontageAsset.Succeeded())
    {
        BodyReloadMontage = BodyReloadMontageAsset.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load BodyReloadMontage"));
    }
    if (WeaponReloadMontageAsset.Succeeded())
    {
        WeaponReloadMontage = WeaponReloadMontageAsset.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load WeaponReloadMontage"));
    }

    // UI
    static ConstructorHelpers::FObjectFinder<UTexture2D> WeaponUITextureAsset(TEXT("/Game/Weapons/Pistol/Textures/Pistol_UI"));
    if (WeaponUITextureAsset.Succeeded())
    {
        WeaponUITexture = WeaponUITextureAsset.Object;
        UE_LOG(LogTemp, Warning, TEXT("Succeeded to load WeaponUITexture"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load WeaponUITexture"));
    }

    /*static ConstructorHelpers::FClassFinder<UUserWidget> WeaponUIClass(TEXT("/Game/ThirdPerson/Blueprints/W_DynamicWeaponHUD.W_DynamicWeaponHUD_C"));
    if (WeaponUIClass.Succeeded())
    {
        WeaponUI = WeaponUIClass.Class;
        UE_LOG(LogTemp, Warning, TEXT("Succeeded to load WeaponUI Class"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load WeaponUI Class"));
    }*/

    // Audio
    static ConstructorHelpers::FObjectFinder<USoundBase> FireSoundAsset(TEXT("/Game/ThirdPerson/Audio/Sounds/Weapons/Pistol/MSS_Weapons_Pistol_Fire"));
    if (FireSoundAsset.Succeeded())
    {
        UE_LOG(LogTemp, Warning, TEXT("Succeeded to load FireSound"));
        FireSound = FireSoundAsset.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load FireSound"));
    }
}
