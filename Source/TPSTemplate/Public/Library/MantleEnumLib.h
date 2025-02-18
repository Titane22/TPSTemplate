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

UENUM(BlueprintType)
enum class EMantleAnimType : uint8
{
	UE5_Manny		UMETA(DisplayName = "UE5_Manny"),
	UE5_Quinn		UMETA(DisplayName = "UE5_Quinn"),
	UE4_Mannequin	UMETA(DisplayName = "UE4_Mannequin"),
	UE4_ESRPG		UMETA(DisplayName = "UE4_ESRPG"),
	Custom			UMETA(DisplayName = "Custom")
};

/**
 * 
 */
UCLASS()
class TPSTEMPLATE_API UMantleEnumLib : public UUserDefinedEnum
{
	GENERATED_BODY()
	
};
