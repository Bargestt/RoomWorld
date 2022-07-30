

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
 * Used in editor to notify execute construction event in room components and attached actors
 */
class ROOMWORLD_API IRoomConstructionNotifyInterface
{
	GENERATED_BODY()
public:

	/** Notify room actor constructed */
	UFUNCTION(BlueprintNativeEvent, Category = Room)
	void OnConstruct(class ARoomNode* RoomNode);

};
