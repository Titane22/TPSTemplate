// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction.generated.h"

class UInteractionComponent;

UCLASS()
class TPSTEMPLATE_API AInteraction : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteraction();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	USceneComponent* DefaultSceneRoot;

	UInteractionComponent* InteractionComponent;
public:

	virtual void Interact(AController* Interactor);

	bool Marked;
};
