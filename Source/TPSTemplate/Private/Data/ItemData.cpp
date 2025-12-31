// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/ItemData.h"

UItemData::UItemData()
{
	// Basic Information
	ItemName = FText::FromString(TEXT("Item"));
	ItemDescription = FText::FromString(TEXT(""));
	ItemIcon = nullptr;

	// Stack System
	bStackable = true;
	MaxStackSize = 1;

	// Properties
	Rarity = EItemRarity::Common;
	Weight = 0.0f;
	SellValue = 0;
	BuyValue = 0;
}

float UItemData::GetTotalWeight(int32 Quantity) const
{
	return Weight * FMath::Max(0, Quantity);
}

int32 UItemData::GetTotalSellValue(int32 Quantity) const
{
	return SellValue * FMath::Max(0, Quantity);
}

bool UItemData::IsValid() const
{
	return !ItemName.IsEmpty();
}
