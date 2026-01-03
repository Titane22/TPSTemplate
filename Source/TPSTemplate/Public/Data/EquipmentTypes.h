#pragma once

#include "CoreMinimal.h"
#include "Data/ItemData.h"
#include "EquipmentTypes.generated.h"

/**
 * Represents a single item slot in the equipment
 * Holds reference to ItemData and instance-specific properties
 */

class AEquipmentBase;

USTRUCT(BlueprintType)
struct FEquipmentSlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<AEquipmentBase> EquipmentClass;
	
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TSoftObjectPtr<UItemData> ItemData;
	
};
