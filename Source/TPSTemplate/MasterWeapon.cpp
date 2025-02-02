// Fill out your copyright notice in the Description page of Project Settings.

#include "MasterWeapon.h"
#include "WeaponSystem.h"
#include "TPSTemplateCharacter.h"
#include "HealthComponent.h" 
#include "WeaponDataAsset.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMasterWeapon::AMasterWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // 컴포넌트 생성
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    WeaponSystem = CreateDefaultSubobject<UWeaponSystem>(TEXT("WeaponSystem"));

    // 컴포넌트 계층 구조 설정
    RootComponent = DefaultSceneRoot;
    WeaponMesh->SetupAttachment(DefaultSceneRoot);
}

// Called when the game starts or when spawned
void AMasterWeapon::BeginPlay()
{
	Super::BeginPlay();
	
    ATPSTemplateCharacter* Player = Cast<ATPSTemplateCharacter>(GetAttachParentActor());
    if (!Player)
        return;
    // TODO: WeaponSystem->Ch = Player;
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
    // 기본 구현 - 자식 클래스에서 오버라이드 가능
}

