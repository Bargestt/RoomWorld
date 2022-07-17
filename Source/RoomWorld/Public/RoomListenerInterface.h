

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RoomListenerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URoomListenerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ROOMWORLD_API IRoomListenerInterface
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintNativeEvent, Category = Room)
	void OnRoomLoaded(ARoom* Room);

	UFUNCTION(BlueprintNativeEvent, Category = Room)
	void OnRoomUnloaded(ARoom* Room);

	UFUNCTION(BlueprintNativeEvent, Category = Room)
	void OnRoomShown(ARoom* Room);

	UFUNCTION(BlueprintNativeEvent, Category = Room)
	void OnRoomHidden(ARoom* Room);
};
