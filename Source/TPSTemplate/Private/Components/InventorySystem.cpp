// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InventorySystem.h"
#include "Data/ItemData.h"

UInventorySystem::UInventorySystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventorySystem::BeginPlay()
{
	Super::BeginPlay();
	InitializeGrid();
}

//==============================================================================
// Public API
//==============================================================================

bool UInventorySystem::CanPlaceItem(UItemData* ItemData, int32 GridRow, int32 GridCol) const
{
	if (!ItemData)
	{
		return false;
	}

	// Check if item fits within grid bounds
	if (GridRow < 0 || GridCol < 0 ||
		GridRow + ItemData->GridHeight > RowCapacity ||
		GridCol + ItemData->GridWidth > ColCapacity)
	{
		return false;
	}

	// Check if all required cells are free
	for (int32 r = 0; r < ItemData->GridHeight; ++r)
	{
		for (int32 c = 0; c < ItemData->GridWidth; ++c)
		{
			int32 CheckRow = GridRow + r;
			int32 CheckCol = GridCol + c;
			int32 Index = GetGridIndex(CheckRow, CheckCol);

			// Cell is occupied if it has a valid GUID
			// TODO: 1개 이하일 때 ReplaceItem호출
			if (GridCells[Index].IsValid())
			{
				return false;
			}
		}
	}

	return true;
}

bool UInventorySystem::AddItem(UItemData* ItemData, int32 GridRow, int32 GridCol, int32 Quantity)
{
	if (!ItemData || Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySystem] AddItem failed: Invalid ItemData or Quantity"));
		return false;
	}

	// Check if item can be placed
	if (!CanPlaceItem(ItemData, GridRow, GridCol))
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySystem] AddItem failed: Cannot place item at (%d, %d)"), GridRow, GridCol);
		return false;
	}

	// Check weight limit
	float ItemWeight = ItemData->GetTotalWeight(Quantity);
	if (CurrentWeight + ItemWeight > MaxWeight)
	{
		// TODO: Allert Message
		UE_LOG(LogTemp, Warning, TEXT("[InventorySystem] AddItem failed: Weight limit exceeded"));
		return false;
	}

	// Create new item slot
	FItemSlot NewSlot(ItemData, Quantity);
	NewSlot.GridRow = GridRow;
	NewSlot.GridCol = GridCol;

	// Add to items array
	Items.Add(NewSlot);

	// Occupy grid cells
	OccupyGridCells(NewSlot);

	// Update weight
	CurrentWeight += ItemWeight;

	UE_LOG(LogTemp, Log, TEXT("[InventorySystem] Added item '%s' at (%d, %d), Weight: %.2f/%.2f"),
		*ItemData->ItemName.ToString(), GridRow, GridCol, CurrentWeight, MaxWeight);

	return true;
}

bool UInventorySystem::RemoveItem(FGuid InstanceId)
{
	// Find item by InstanceId
	int32 ItemIndex = Items.IndexOfByPredicate([&](const FItemSlot& Slot)
	{
		return Slot.InstanceId == InstanceId;
	});

	if (ItemIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySystem] RemoveItem failed: Item not found"));
		return false;
	}

	FItemSlot& Item = Items[ItemIndex];

	// Clear grid cells
	ClearGridCells(Item);

	// Update weight
	CurrentWeight -= Item.GetTotalWeight();

	// Remove from array
	Items.RemoveAt(ItemIndex);

	UE_LOG(LogTemp, Log, TEXT("[InventorySystem] Removed item, Weight: %.2f/%.2f"), CurrentWeight, MaxWeight);

	return true;
}

bool UInventorySystem::MoveItem(FGuid InstanceId, int32 GridRow, int32 GridCol)
{
	// Find the item to move
	FItemSlot* ItemToMove = FindItem(InstanceId);
	if (!ItemToMove)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySystem] MoveItem failed: Item not found"));
		return false;
	}

	UItemData* ItemData = ItemToMove->GetItemData();
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySystem] MoveItem failed: Invalid ItemData"));
		return false;
	}

	// Store old position for logging
	int32 OldRow = ItemToMove->GridRow;
	int32 OldCol = ItemToMove->GridCol;

	// Clear the old grid cells
	ClearGridCells(*ItemToMove);

	// Check if item can be placed at new position
	if (!CanPlaceItem(ItemData, GridRow, GridCol))
	{
		// Restore old grid cells if move failed
		OccupyGridCells(*ItemToMove);
		UE_LOG(LogTemp, Warning, TEXT("[InventorySystem] MoveItem failed: Cannot place item at (%d, %d)"), GridRow, GridCol);
		return false;
	}

	// Update item position
	ItemToMove->GridRow = GridRow;
	ItemToMove->GridCol = GridCol;
	UE_LOG(LogTemp, Warning, TEXT("Grid Row: %d, Grid Col: %d"), GridRow, GridCol);
	// Occupy new grid cells
	OccupyGridCells(*ItemToMove);

	UE_LOG(LogTemp, Log, TEXT("[InventorySystem] Moved item '%s' from (%d, %d) to (%d, %d)"),
		*ItemData->ItemName.ToString(), OldRow, OldCol, GridRow, GridCol);

	return true;
}

FItemSlot* UInventorySystem::FindItem(FGuid InstanceId)
{
	return Items.FindByPredicate([&](const FItemSlot& Slot)
	{
		return Slot.InstanceId == InstanceId;
	});
}

bool UInventorySystem::FindItemCopy(FGuid InstanceId, FItemSlot& OutItem)
{
	FItemSlot* Item = FindItem(InstanceId);
	if (Item)
	{
		OutItem = *Item;
		return true;
	}
	return false;
}

bool UInventorySystem::FindEmptySpot(UItemData* ItemData, int32& OutRow, int32& OutCol)
{
	if (!ItemData)
	{
		return false;
	}

	// Try to find an empty spot by scanning the grid
	for (int32 Row = 0; Row <= RowCapacity - ItemData->GridHeight; ++Row)
	{
		for (int32 Col = 0; Col <= ColCapacity - ItemData->GridWidth; ++Col)
		{
			if (CanPlaceItem(ItemData, Row, Col))
			{
				OutRow = Row;
				OutCol = Col;
				return true;
			}
		}
	}

	return false;
}

//==============================================================================
// Helper Functions
//==============================================================================

void UInventorySystem::InitializeGrid()
{
	int32 TotalCells = RowCapacity * ColCapacity;
	GridCells.SetNum(TotalCells);

	// Initialize all cells with invalid GUIDs (empty)
	for (int32 i = 0; i < TotalCells; ++i)
	{
		GridCells[i].Invalidate();
	}

	CurrentWeight = 0.0f;
	Items.Empty();

	UE_LOG(LogTemp, Log, TEXT("[InventorySystem] Grid initialized: %dx%d (%d cells)"), RowCapacity, ColCapacity, TotalCells);
}

int32 UInventorySystem::GetGridIndex(int32 Row, int32 Col) const
{
	return Row * ColCapacity + Col;
}

bool UInventorySystem::IsValidGridPosition(int32 Row, int32 Col) const
{
	return Row >= 0 && Row < RowCapacity && Col >= 0 && Col < ColCapacity;
}

FItemSlot* UInventorySystem::GetItemAtCell(int32 Row, int32 Col)
{
	if (!IsValidGridPosition(Row, Col))
	{
		return nullptr;
	}

	int32 Index = GetGridIndex(Row, Col);
	FGuid ItemId = GridCells[Index];

	if (!ItemId.IsValid())
	{
		return nullptr;
	}

	return FindItem(ItemId);
}

bool UInventorySystem::GetItemAtCellCopy(int32 Row, int32 Col, FItemSlot& OutItem)
{
	FItemSlot* Item = GetItemAtCell(Row, Col);
	if (Item)
	{
		OutItem = *Item;
		return true;
	}
	return false;
}

void UInventorySystem::OccupyGridCells(const FItemSlot& Item)
{
	if (!Item.IsPlacedInGrid())
	{
		return;
	}

	UItemData* Data = Item.GetItemData();
	if (!Data)
	{
		return;
	}

	// Mark all cells occupied by this item
	for (int32 r = 0; r < Data->GridHeight; ++r)
	{
		for (int32 c = 0; c < Data->GridWidth; ++c)
		{
			int32 Row = Item.GridRow + r;
			int32 Col = Item.GridCol + c;
			int32 Index = GetGridIndex(Row, Col);

			GridCells[Index] = Item.InstanceId;
		}
	}
}

void UInventorySystem::ClearGridCells(const FItemSlot& Item)
{
	if (!Item.IsPlacedInGrid())
	{
		return;
	}

	UItemData* Data = Item.GetItemData();
	if (!Data)
	{
		return;
	}

	// Clear all cells occupied by this item
	for (int32 r = 0; r < Data->GridHeight; ++r)
	{
		for (int32 c = 0; c < Data->GridWidth; ++c)
		{
			int32 Row = Item.GridRow + r;
			int32 Col = Item.GridCol + c;
			int32 Index = GetGridIndex(Row, Col);

			GridCells[Index].Invalidate();
		}
	}
}

void UInventorySystem::RecalculateWeight()
{
	CurrentWeight = 0.0f;

	for (const FItemSlot& Item : Items)
	{
		CurrentWeight += Item.GetTotalWeight();
	}
}

//==============================================================================
// Testing / Debug
//==============================================================================

void UInventorySystem::AddTestItems(UItemData* RifleData, UItemData* PistolData)
{
	if (!RifleData || !PistolData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySystem] AddTestItems failed: Invalid item data"));
		return;
	}

	// Add Pistol at (0, 0)
	bool bPistolAdded = AddItem(PistolData, 0, 0, 1);
	if (bPistolAdded)
	{
		UE_LOG(LogTemp, Log, TEXT("[InventorySystem] Test: Added Pistol at (0, 0)"));
	}

	// Add Rifle at (1, 2)
	bool bRifleAdded = AddItem(RifleData, 1, 2, 1);
	if (bRifleAdded)
	{
		UE_LOG(LogTemp, Log, TEXT("[InventorySystem] Test: Added Rifle at (1, 2)"));
	}

	UE_LOG(LogTemp, Log, TEXT("[InventorySystem] Test items added. Total items: %d, Weight: %.2f/%.2f"),
		Items.Num(), CurrentWeight, MaxWeight);
}
