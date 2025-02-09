// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPSTemplateGameMode.h"
#include "TPSTemplateCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATPSTemplateGameMode::ATPSTemplateGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_Player"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// Set default player controller class
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ShooterPlayerController"));
	if (PlayerControllerBPClass.Succeeded())
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;  
		UE_LOG(LogTemp, Warning, TEXT("PlayerController class set successfully"));
	}
}
