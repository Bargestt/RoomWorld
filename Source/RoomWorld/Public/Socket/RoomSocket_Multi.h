

#pragma once

#include "RoomSocket.h"
#include "RoomSocket_Multi.generated.h"



/**
 * WIP
 */
UCLASS(Abstract, ClassGroup = (RoomWorld))
class ROOMWORLD_API URoomSocket_Multi : public URoomSocket
{
	GENERATED_BODY()

public:
	URoomSocket_Multi();

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR


	virtual bool SetConnection(URoomSocket* Connection, int32 Index) override;
	virtual bool AddConnection(URoomSocket* Connection) override;
	virtual bool ClearConnection(URoomSocket* Connection) override;
	virtual bool ClearConnectionAt(int32 Index) override;
};

