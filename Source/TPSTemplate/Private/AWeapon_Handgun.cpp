// Fill out your copyright notice in the Description page of Project Settings.


#include "AWeapon_Handgun.h"
#include "../WeaponSystem.h"
#include "./Weapon/DA_Pistol.h"
#include "UObject/ConstructorHelpers.h"

AAWeapon_Handgun::AAWeapon_Handgun()
{
    // 스켈레탈 메시 에셋 로드
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> HandgunMesh(TEXT("/Game/Weapons/Pistol/Mesh/SK_Handgun"));
    if (HandgunMesh.Succeeded() && WeaponMesh)  // WeaponMesh는 부모 클래스의 변수
    {
        WeaponMesh->SetSkeletalMesh(HandgunMesh.Object);
    }

    // 무기 타입 설정
    WeaponType = EAnimationState::Pistol;
    WeaponData = NewObject<UDA_Pistol>();
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
    WeaponSystem->RifleData = NewObject<UDA_Pistol>();

    WeaponSystem->CrosshairWidget = nullptr;
}

void AAWeapon_Handgun::Fire()
{

}
