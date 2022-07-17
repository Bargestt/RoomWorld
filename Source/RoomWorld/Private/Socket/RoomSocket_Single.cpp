


#include "Socket/RoomSocket_Single.h"
#include "Room.h"



URoomSocket_Single::URoomSocket_Single()
{
	ConnectedTo.SetNum(1);
	ConnectedToHandles.SetNum(1);
}

void URoomSocket_Single::SyncConnections()
{
	ConnectedToHandles.SetNum(1);
	SetConnection(ConnectedToHandles[0] ? ConnectedToHandles[0]->Socket : nullptr, 0);
}

bool URoomSocket_Single::SetConnection(URoomSocket* NewConnection, int32 Index)
{	
	if (!ensure(ConnectedTo.Num() == 1))
	{
		ConnectedTo.SetNum(1);
	}

	URoomSocket*& Connection = ConnectedTo[0];
	if (NewConnection == Connection || NewConnection == this)
	{
		return false;
	}

	URoomSocket* OldConnection = Connection;

	Connection = nullptr;
	OnConnectionLost(OldConnection, 0);
	if (OldConnection && OldConnection->IsConnectedTo(this))
	{
		OldConnection->ClearConnection(this);
	}

	Connection = NewConnection;
	OnConnectionGained(Connection, 0);

	if (Connection && !Connection->IsConnectedTo(this))
	{
		Connection->AddConnection(this);
	}

	SyncHandles();
	return true;
}

bool URoomSocket_Single::AddConnection(URoomSocket* Connection)
{
	return SetConnection(Connection, 0);
}

bool URoomSocket_Single::ClearConnection(URoomSocket* Connection)
{
	if (!IsConnectedTo(Connection))
	{
		return false;
	}

	return SetConnection(nullptr, 0);
}

bool URoomSocket_Single::ClearConnectionAt(int32 Index)
{
	return SetConnection(nullptr, 0);
}



