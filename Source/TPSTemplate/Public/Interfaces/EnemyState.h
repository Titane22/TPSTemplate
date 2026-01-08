// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyState.generated.h"

class APlayer_Base;
class ATPSTemplate_Enemy_Base;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEnemyState : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TPSTEMPLATE_API IEnemyState
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, Category = "AI|Notify")
	void NotifyInvestigatingLocation(const FVector& Location);

	UFUNCTION(BlueprintNativeEvent, Category = "AI|Notify")
	void NotifyColleagueDeath(const ATPSTemplate_Enemy_Base* Colleague);

	UFUNCTION(BlueprintNativeEvent, Category = "AI|Notify")
	void NotifyFoundTarget(const APlayer_Base* Player);
};
