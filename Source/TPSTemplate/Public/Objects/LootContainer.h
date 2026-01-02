// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Interaction.h"
#include "LootContainer.generated.h"

/*
 * 루트 컨테이너
 * 
 */

class AShooterPlayerController;
class ULootingSystem;
class UInventorySystem;
struct FLootItemEntry;
class ULootTableData;

UCLASS()
class TPSTEMPLATE_API ALootContainer : public AInteraction
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALootContainer();

	virtual bool CanInteract_Implementation(AController* InstigatorRef) const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnLootOpened(AInteraction* Interaction, const FInteractionContext& Context);

	UFUNCTION(BlueprintCallable, Category = "Looting|UI")
	void OpenLootingUI(AShooterPlayerController* PC);
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BaseMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInventorySystem* InventorySystem = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	ULootingSystem* LootingSystem = nullptr;
	
};
