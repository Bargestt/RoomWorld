

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RoomConstructionNotifyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URoomConstructionNotifyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ROOMWORLD_API IRoomConstructionNotifyInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, Category = Room)
	void OnConstruct(ARoom* Room);

};
