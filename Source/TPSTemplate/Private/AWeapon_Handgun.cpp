// Fill out your copyright notice in the Description page of Project Settings.


#include "AWeapon_Handgun.h"
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
}

void AAWeapon_Handgun::Fire()
{

}
