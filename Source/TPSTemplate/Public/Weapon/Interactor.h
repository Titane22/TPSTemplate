// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Library/InteractiveType.h"
#include "Interactor.generated.h"

class APlayer_Base;
class ATPSTemplateCharacter;
class AInteraction;
class UInteractionComponent;

UENUM(BlueprintType)
enum class EInteractionMethod : uint8
{
	Camera          UMETA(DisplayName = "Camera"),
	BodyForward		UMETA(DisplayName = "BodyForward")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSTEMPLATE_API UInteractor : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractor();
	// Called when the game starts
	virtual void BeginPlay() override;

	float DetectionDistance = 350.0f;

	bool HasInteraction;

	bool InteractorActive = true;

	EInteractionMethod InteractionMethod = EInteractionMethod::Camera;

	AActor* InteractionActor;

	AInteraction* InteractionBP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	APlayer_Base* CharacterRef;

	void StopLastIntteraction();

	void StartInteraction(AController* Interactor);

	void Reset();
	
	EInteractiveType GetInteractionType();
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
