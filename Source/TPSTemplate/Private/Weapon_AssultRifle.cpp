// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon_AssultRifle.h"
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
}

void AWeapon_AssultRifle::Fire()
{
    
}