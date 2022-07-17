

#pragma once

#include "RoomTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRoomWorld, Log, All);

UENUM(BlueprintType)
enum class ERoomState : uint8
{
	/** Hidden and Unloaded */
	Unloaded,
	/** Loaded but not shown */
	Loaded,
	/** Visible and Loaded */
	Visible
};


/**  */
USTRUCT(BlueprintType)
struct FSocketData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform;
	
};