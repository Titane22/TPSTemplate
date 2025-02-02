// Fill out your copyright notice in the Description page of Project Settings.


#include "AWeapon_Handgun.h"
#include "UObject/ConstructorHelpers.h"

AAWeapon_Handgun::AAWeapon_Handgun()
{
    // ���̷�Ż �޽� ���� �ε�
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> HandgunMesh(TEXT("/Game/Weapons/Pistol/Mesh/SK_Handgun"));
    if (HandgunMesh.Succeeded() && WeaponMesh)  // WeaponMesh�� �θ� Ŭ������ ����
    {
        WeaponMesh->SetSkeletalMesh(HandgunMesh.Object);
    }

    // ���� Ÿ�� ����
    WeaponType = EAnimationState::Pistol;
}

void AAWeapon_Handgun::Fire()
{

}
