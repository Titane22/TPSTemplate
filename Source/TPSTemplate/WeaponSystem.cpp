// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSystem.h"
#include "MasterWeapon.h"
#include "TPSTemplateCharacter.h"

// Sets default values for this component's properties
UWeaponSystem::UWeaponSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UWeaponSystem::RifleEquip(FName SocketName)
{
	// 캐릭터 레퍼런스가 유효한지 확인
	if (CharacterRef == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::RifleEquip CharacterRef is Null"));
		return;
	}

	// Primary Child Component를 가져오고 캐스팅
	UChildActorComponent* PrimaryChild = Cast<UChildActorComponent>(CharacterRef->PrimaryChild);
	if (!PrimaryChild)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::RifleEquip PrimaryChild is Null"));
		return;
	}

	// Child Actor Class를 MasterWeapon으로 설정
	PrimaryChild->SetChildActorClass(MasterWeapon->GetClass());

	UWeaponSystem* TargetWeaponSystem = Cast<UWeaponSystem>(PrimaryChild->GetChildActor()->GetComponentByClass(this->GetClass()));
	if(!TargetWeaponSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::RifleEquip TargetWeaponSystem is Null"));
		return;
	}
	TargetWeaponSystem->CharacterRef = CharacterRef;
	
	PrimaryChild->AttachToComponent(
		CharacterRef->GetMesh(),
		FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget,    // Location Rule
			EAttachmentRule::SnapToTarget,    // Rotation Rule
			EAttachmentRule::SnapToTarget,    // Scale Rule
			true                              // Weld Simulated Bodies
		),
		SocketName
	);

	CharacterRef->CurrentAnimationState = AnimationState;
}

void UWeaponSystem::RifleUnequip(FName SocketName)
{

}

// Called when the game starts
void UWeaponSystem::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UWeaponSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

