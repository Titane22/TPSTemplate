// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "MantleEnumLib.generated.h"

UENUM(BlueprintType)
enum class EMantleType : uint8
{
	HighMantle		UMETA(DisplayName = "HighMantle"),
	LowMantle		UMETA(DisplayName = "LowMantle"),
	FallingMantle	UMETA(DisplayName = "FallingMantle")
};

UENUM(BlueprintType)
enum class EMantleHandType : uint8
{
	LeftHand		UMETA(DisplayName = "LeftHand"),
	RightHand		UMETA(DisplayName = "RightHand")
};

/**
 * 
 */
UCLASS()
class TPSTEMPLATE_API UMantleEnumLib : public UUserDefinedEnum
{
	GENERATED_BODY()
	
};
