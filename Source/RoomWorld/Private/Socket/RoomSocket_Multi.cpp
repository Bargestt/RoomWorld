


#include "Socket/RoomSocket_Multi.h"
#include "Room.h"



URoomSocket_Multi::URoomSocket_Multi()
{
	
}


#if WITH_EDITOR
void URoomSocket_Multi::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(URoomSocket, ConnectedToHandles))
	{		
		for (int32 Index = 0; Index < ConnectedToHandles.Num(); Index++)
		{
			URoomSocket* NewConnection = ConnectedToHandles[Index] ? ConnectedToHandles[Index]->Socket : nullptr;


			//URoomSocket* CurrentConnection = ConnectedTo[Index];

			//if (NewConnection != CurrentConnection)
			{
				//SetConnection(NewConnection, Index);
			}
		}
	}
}
#endif




bool URoomSocket_Multi::SetConnection(URoomSocket* Connection, int32 Index)
{
	if (Connection == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("SetConnection: nullptr is not allowed when setting connection, use ClearConnection instead"));
		return false;
	}

	if (!ConnectedTo.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Error, TEXT("SetConnection: invalid index"));
		return false;
	}	

	if (Connection == ConnectedTo[Index] || 
		(ConnectedTo[Index] && !CanClearConnection(ConnectedTo[Index], Index)) || 
		!CanSetConnection(Connection, Index))
	{
		return false;
	}

	URoomSocket* OldConnection = ConnectedTo[Index];
	ConnectedTo[Index] = nullptr;
	OnConnectionLost(OldConnection, Index);

	ConnectedTo[Index] = Connection;
	OnConnectionGained(Connection, Index);

	SyncHandles();
	return true;
}

bool URoomSocket_Multi::AddConnection(URoomSocket* Connection)
{
	if (!Connection)
	{
		return false;
	}

	if (!CanSetConnection(Connection, INDEX_NONE))
	{
		return false;
	}

	int32 Index = ConnectedTo.Add(Connection);
	OnConnectionGained(Connection, Index);

	SyncHandles();
	return true;
}

bool URoomSocket_Multi::ClearConnection(URoomSocket* Connection)
{
	int32 Index = GetConnectionIndex(Connection);
	return (Index < 0) && ClearConnectionAt(Index);
}

bool URoomSocket_Multi::ClearConnectionAt(int32 Index)
{
	if (!ConnectedTo.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Error, TEXT("ClearConnectionAt: invalid index"));
		return false;
	}

	if (ConnectedTo[Index] == nullptr || !CanClearConnection(ConnectedTo[Index], Index))
	{
		return false;
	}

	URoomSocket* OldConnection = ConnectedTo[Index];	
	ConnectedTo[Index] = nullptr;
	OnConnectionLost(OldConnection, Index);

	SyncHandles();
	return true;
}
