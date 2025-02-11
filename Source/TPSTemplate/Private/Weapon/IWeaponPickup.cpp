// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/IWeaponPickup.h"
#include "Widget/W_WeaponPickup.h"
#include "../MasterWeapon.h"
#include "./Weapon/InteractionComponent.h"
#include "./Weapon/Interactor.h"
#include "./Weapon/DA_Rifle.h"
#include "Weapon_AssultRifle.h"

AIWeaponPickup::AIWeaponPickup()
	:Super()
{
	PrimaryActorTick.bCanEverTick = true;
	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	WeaponClass = CreateDefaultSubobject<AWeapon_AssultRifle>(TEXT("WeaponClass"));

	Widget->SetupAttachment(RootComponent);
	WeaponMesh->SetupAttachment(RootComponent);

	Widget->SetUsingAbsoluteRotation(true);

	InteractionComponent->InteractionType = EInteractiveType::WeaponPickup;
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> RifleMesh(TEXT("/Game/Weapons/Rifle/Mesh/SK_Rifle"));
	if (RifleMesh.Succeeded() && WeaponMesh) 
	{
		WeaponMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
		WeaponMesh->SetSkeletalMesh(RifleMesh.Object);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetGenerateOverlapEvents(true);
	}
}

void AIWeaponPickup::BeginPlay()
{
	Super::BeginPlay();
	
	WidgetRef = Cast<UW_WeaponPickup>(Widget->GetWidget());
	if (!WidgetRef)
	{
		return;
	}

	WeaponData = NewObject<UDA_Rifle>();
	if (!WeaponData)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponData Is NULL"));
	}

	WidgetRef->SetWeaponBrush(WeaponData->WeaponUITexture);
	WidgetRef->SetToPickupWeaponName(WeaponData->WeaponName);
}

void AIWeaponPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetActorLocation(WeaponMesh->GetComponentLocation());
	Widget->SetVisibility(Marked);
}

void AIWeaponPickup::Interact(AController* Interactor)
{
	Destroy();
}
