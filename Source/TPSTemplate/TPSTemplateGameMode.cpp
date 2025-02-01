// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPSTemplateGameMode.h"
#include "TPSTemplateCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATPSTemplateGameMode::ATPSTemplateGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
