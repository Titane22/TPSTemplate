// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineMantle.generated.h"

UCLASS()
class TPSTEMPLATE_API ATimelineMantle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATimelineMantle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* DefaultSceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UMantleSystem* MantleSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UCurveFloat* MantleCurve = nullptr;

	UFUNCTION()
	void MantleTimelineUpdate(float BlendIn);

	UFUNCTION()
	void MantleTimelineFinished();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UTimelineComponent* TimelineRef;
};
