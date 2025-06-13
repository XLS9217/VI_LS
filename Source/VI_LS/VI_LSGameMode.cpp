// Copyright Epic Games, Inc. All Rights Reserved.

#include "VI_LSGameMode.h"
#include "VI_LSCharacter.h"
#include "UObject/ConstructorHelpers.h"

AVI_LSGameMode::AVI_LSGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
