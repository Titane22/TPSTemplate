// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/TPSTemplateCharacter.h"
#include "TPSTemplate_Enemy_Base.generated.h"

/**
 * 
 */
UCLASS()
class TPSTEMPLATE_API ATPSTemplate_Enemy_Base : public ATPSTemplateCharacter
{
	GENERATED_BODY()

protected:
	ATPSTemplate_Enemy_Base();
	virtual void BeginPlay();
};
