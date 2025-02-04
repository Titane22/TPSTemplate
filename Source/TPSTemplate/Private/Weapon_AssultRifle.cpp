// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon_AssultRifle.h"
#include "./Weapon/DA_Rifle.h"
#include "../WeaponSystem.h"
#include "./Weapon/DA_Rifle.h"
#include "UObject/ConstructorHelpers.h"

AWeapon_AssultRifle::AWeapon_AssultRifle()
{
    // 스켈레탈 메시 에셋 로드
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> RifleMesh(TEXT("/Game/Weapons/Rifle/Mesh/SK_Rifle"));
    if (RifleMesh.Succeeded() && WeaponMesh)  // WeaponMesh는 부모 클래스의 변수
    {
        WeaponMesh->SetSkeletalMesh(RifleMesh.Object);
    }

    // 무기 타입 설정
    WeaponType = EAnimationState::RifleShotgun;
    WeaponData = NewObject<UDA_Rifle>();
    bReloading = false;
    bAutoReload = true;

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
    WeaponSystem->PistolData = nullptr;
    WeaponSystem->RifleData = NewObject<UDA_Rifle>();

    WeaponSystem->CrosshairWidget = nullptr;
}

void AWeapon_AssultRifle::Fire()
{
    
}