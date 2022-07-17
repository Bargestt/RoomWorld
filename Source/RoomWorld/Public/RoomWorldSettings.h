// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RoomWorldSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "RoomWorld"))
class ROOMWORLD_API URoomWorldSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const URoomWorldSettings* Get() { return GetDefault<URoomWorldSettings>(); }
	
	URoomWorldSettings();

public:
	UPROPERTY(EditAnywhere, Config, Category = RoomWorld)
	FName DefaultSocketTag;

	/** Removes SocketTag from socket name when reading room sockets from level if SocketTag == DefaultSocketTag */
	UPROPERTY(EditAnywhere, Config, Category = RoomWorld)
	uint32 bCollapseDefaultSocketTag : 1;
};
