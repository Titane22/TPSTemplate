// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Interaction.h"
#include "Weapon/InteractionComponent.h"

// Sets default values
AInteraction::AInteraction()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));

	RootComponent = DefaultSceneRoot;
}

// Called when the game starts or when spawned
void AInteraction::BeginPlay()
{
	Super::BeginPlay();
	
}

void AInteraction::Tick(float DeltaSeconds)
{
}

void AInteraction::Interact(AController* Interactor)
{
}

