// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "CoverableActor.generated.h"

class UArrowComponent;
class UBoxComponent;

UENUM(BlueprintType)
enum class ECoverState : uint8 
{
	Low		UMETA(DisplayName = "Low"),
	High	UMETA(DisplayName = "High")
};

UCLASS()
class TPSTEMPLATE_API ACoverableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACoverableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* DefaultScene;

	// Arrow Component to visually indicate cover direction
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* ArrowComponentNorth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* ArrowComponentSouth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* ArrowComponentEast;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* ArrowComponentWest;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CoverMesh;

	// Box Collision to detect player and AI access
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* BoxCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	UWidgetComponent* CoverPromptWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> CoverPromptWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	bool bIsOccupied;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsOccupied();

	UFUNCTION(BlueprintCallable, Category = "Cover")
	void ShowCoverPrompt(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "Cover")
	void UpdateCoverPromptTransform(FVector Location, FRotator Rotaion);

};
