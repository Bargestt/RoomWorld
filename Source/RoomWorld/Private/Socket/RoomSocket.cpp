


#include "Socket/RoomSocket.h"
#include "RoomNode.h"



URoomSocket::URoomSocket()
{
	
}

#if WITH_EDITOR
void URoomSocket::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(URoomSocket, ConnectedToHandles))
	{
		SyncConnections();
	}
}
#endif


void URoomSocket::Initialize(FName InName, const FSocketData& Data)
{
	Location = Data.Transform.GetLocation();
	Rotation = Data.Transform.GetRotation().Rotator();
	Name = InName;
}

void URoomSocket::SyncHandles()
{
#if WITH_EDITOR
	ConnectedToHandles.SetNum(ConnectedTo.Num());
	for (int32 Index = 0; Index < ConnectedToHandles.Num(); Index++)
	{
		ConnectedToHandles[Index] = ConnectedTo[Index] ? ConnectedTo[Index]->Handle : nullptr;
	}
#endif
}


void URoomSocket::RemoveEmptyConnections()
{
	ConnectedTo.Remove(nullptr);
	SyncHandles();
}

ARoomNode* URoomSocket::GetNode() const
{
	return Room;
}

ARoomNode* URoomSocket::GetConnectedNode(int32 Index) const
{
	URoomSocket* Connection = GetConnection(Index);
	return Connection ? Connection->GetNode() : nullptr;
}

TArray<ARoomNode*> URoomSocket::GetConnectedNodes() const
{
	TArray<ARoomNode*> Arr;

	for (int32 Index = 0; Index < GetNumConnections() ; Index++)
	{
		if (ARoomNode* ConnectedRoom = GetConnectedNode(Index))
		{
			Arr.Add(ConnectedRoom);
		}
	}

	return Arr;
}

FTransform URoomSocket::GetRelativeTransform() const
{
	return FTransform(Rotation, Location);
}

FTransform URoomSocket::GetTransform() const
{
	return Room ? GetRelativeTransform() * Room->GetActorTransform() : GetRelativeTransform();
}	

TArray<URoomSocket*> URoomSocket::GetConnections() const
{
	return ConnectedTo;
}

int32 URoomSocket::GetNumConnections() const
{
	return ConnectedTo.Num();
}

bool URoomSocket::HasConnections() const
{
	return ConnectedTo.Num() > 0;
}

URoomSocket* URoomSocket::GetConnection(int32 Index) const
{
	return ConnectedTo.IsValidIndex(Index) ? ConnectedTo[Index] : nullptr;
}

int32 URoomSocket::GetConnectionIndex(URoomSocket* Socket) const
{
	return Socket ? ConnectedTo.Find(Socket) : INDEX_NONE;
}

bool URoomSocket::IsConnectedTo(URoomSocket* Socket) const
{
	return Socket && ConnectedTo.Contains(Socket);
}

void URoomSocket::OnConnectionGained(URoomSocket* NewConnection, int32 Index)
{
	ReceiveConnectionGained(NewConnection, Index);

	if (Handle)
	{
		FEditorScriptExecutionGuard ScriptGuard;
		Handle->ReceiveConnectionGained(NewConnection, Index);
	}
}

void URoomSocket::OnConnectionLost(URoomSocket* OldConnection, int32 Index)
{
	ReceiveConnectionLost(OldConnection, Index);

	if (Handle)
	{
		FEditorScriptExecutionGuard ScriptGuard;
		Handle->ReceiveConnectionLost(OldConnection, Index);
	}
}
