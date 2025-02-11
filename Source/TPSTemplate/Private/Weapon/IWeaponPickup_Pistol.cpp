// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/IWeaponPickup_Pistol.h"
#include "Widget/W_WeaponPickup.h"
#include "./Weapon/IWeaponPickup_Pistol.h"
#include "./Weapon/DA_Pistol.h"

AIWeaponPickup_Pistol::AIWeaponPickup_Pistol()
	:Super()
{
	WeaponData = nullptr;
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> PistolMesh(TEXT("/Game/Weapons/Pistol/Mesh/SK_Pistol"));
	if (PistolMesh.Succeeded() && WeaponMesh)
	{
		WeaponMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
		WeaponMesh->SetSkeletalMesh(PistolMesh.Object);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetGenerateOverlapEvents(true);
		// Pawn만 무시하도록 설정
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	}
}

void AIWeaponPickup_Pistol::BeginPlay()
{
	Super::BeginPlay();
	
	WidgetRef = Cast<UW_WeaponPickup>(Widget->GetWidget());
	if (!WidgetRef)
	{
		return;
	}

	WeaponData = NewObject<UDA_Pistol>();
	if (!WeaponData)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponData Is NULL"));
		return;
	}
	
	WidgetRef->SetWeaponBrush(WeaponData->WeaponUITexture);
	WidgetRef->SetToPickupWeaponName(WeaponData->WeaponName);
}
