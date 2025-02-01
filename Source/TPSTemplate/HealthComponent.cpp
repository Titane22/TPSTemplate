// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "TPSTemplateCharacter.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	StartWithMaxHealth = true;
	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Sequence 1
	ATPSTemplateCharacter* Owner = Cast<ATPSTemplateCharacter>(GetOwner());
	if (!Owner)
		return;
	
	CharacterRef = Owner;

	// Sequence 2
	if (StartWithMaxHealth)
	{
		Health = MaxHealth;
	}
}

bool UHealthComponent::ApplyDamage(float Damage)
{
	Health = Health - Damage;

	if (Health <= 0)
	{
		// TODO: CharacterRef->StartRagdoll

		// TODO: CharacterRef->Die();

		return true;
	}
	else
	{
		return false;
	}
}

