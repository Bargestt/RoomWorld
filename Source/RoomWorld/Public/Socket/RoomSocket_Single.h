

#pragma once

#include "RoomSocket.h"
#include "RoomSocket_Single.generated.h"



/**
 * Direct one to one connection. More than one connection is not allowed
 * Ensures sockets reference each other
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (RoomWorld))
class ROOMWORLD_API URoomSocket_Single : public URoomSocket
{
	GENERATED_BODY()

public:
	URoomSocket_Single();

	virtual void SyncConnections() override;

	virtual bool SetConnection(URoomSocket* Connection, int32 Index) override;
	virtual bool AddConnection(URoomSocket* Connection) override;
	virtual bool ClearConnection(URoomSocket* Connection) override;
	virtual bool ClearConnectionAt(int32 Index) override;
};

