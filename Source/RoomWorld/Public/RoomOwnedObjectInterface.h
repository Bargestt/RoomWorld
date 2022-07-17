

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RoomOwnedObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URoomOwnedObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ROOMWORLD_API IRoomOwnedObjectInterface
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintNativeEvent, Category = Room)
	void SetOwningRoom(ARoom* Room);

	UFUNCTION(BlueprintNativeEvent, Category = Room)
	void GetOwningRoom(ARoom*& Room);
};
