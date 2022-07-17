


#include "RoomNode.h"
#include "RoomConstructionNotifyInterface.h"

#include <Logging/MessageLog.h>
#include <Logging/TokenizedMessage.h>
#include <Misc/UObjectToken.h>

#include "Socket/RoomSocket.h"
#include "Socket/RoomSocketHandle.h"
#include "RoomData.h"

#include "Socket/RoomSocket_Single.h"


#define LOCTEXT_NAMESPACE "RoomWorld"

ARoomNode::ARoomNode()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SocketClass = URoomSocket_Single::StaticClass();
	SocketHandleClass = ARoomSocketHandle::StaticClass();
}

void ARoomNode::OnConstruction(const FTransform& Transform)
{
	TArray<AActor*> Attached;
	GetAttachedActors(Attached);
	for (AActor* Actor : Attached)
	{
		if (ARoomSocketHandle* Handle = Cast<ARoomSocketHandle>(Actor))
		{
			if (Handle->Socket == nullptr || Sockets.FindKey(Handle->Socket) == nullptr)
			{
				Handle->Destroy();
			}
		}
	}
}

void ARoomNode::Destroyed()
{
	Super::Destroyed();

#if WITH_EDITOR
	TArray<AActor*> Attached;
	GetAttachedActors(Attached);

	for (AActor* Actor : Attached)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
#endif //WITH_EDITOR
}

void ARoomNode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TArray<AActor*> Attached;
	GetAttachedActors(Attached);

	for (AActor* Actor : Attached)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}		
	}

	Super::EndPlay(EndPlayReason);
}



void ARoomNode::InitializeSockets(const TMap<FName, FSocketData>& InSocketData)
{
	TMap<FName, URoomSocket*> OldSockets = Sockets;
	Sockets.Empty();

	if (SocketClass)
	{
		for (const auto& SocketData : InSocketData)
		{
			const FName& SocketName = SocketData.Key;
			const FSocketData& Data = SocketData.Value;

			URoomSocket* Socket = nullptr;
			OldSockets.RemoveAndCopyValue(SocketName, Socket);

			if (Socket == nullptr || Socket->GetClass() != SocketClass)
			{
				URoomSocket* NewSocket = NewObject<URoomSocket>(this, SocketClass, NAME_None);
				NewSocket->Handle = Socket ? Socket->Handle : nullptr;

				Socket = NewSocket;
			}

			Socket->Room = this;
			Socket->Initialize(SocketName, Data);

			Sockets.Add(SocketName, Socket);

#if WITH_EDITOR		
			ARoomSocketHandle* AddHandle = Socket->Handle;

			UClass* DesiredHandleClass = Socket->HandleClass ? Socket->HandleClass : SocketHandleClass;

			if (AddHandle && (
				AddHandle->GetAttachParentActor() != this ||
				AddHandle->GetClass() != DesiredHandleClass))
			{					
				AddHandle->Destroy();
			}

			if (DesiredHandleClass)
			{
				if (!IsValid(AddHandle))
				{
					FActorSpawnParameters Params;
					Params.OverrideLevel = GetLevel();
					//Params.Owner = this; // Not possible due to pesky notifies when deleting room
					Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					Params.ObjectFlags |= RF_NonPIEDuplicateTransient;

					AddHandle = GetWorld()->SpawnActor<ARoomSocketHandle>(DesiredHandleClass, Socket->GetRelativeTransform(), Params);
				}

				if (IsValid(AddHandle))
				{
					Socket->Handle = AddHandle;
					AddHandle->SetSocket(Socket);

					AddHandle->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
					AddHandle->SetActorRelativeTransform(Socket->GetRelativeTransform());
			
					FString Label = FString::Printf(TEXT("Socket:%s-%s"), *this->GetActorLabel(), *Socket->Name.ToString());
					if (AddHandle->GetActorLabel() != Label)
					{
						AddHandle->SetActorLabel(Label);
					}						
				}
				else
				{
					UE_LOG(LogRoomWorld, Error, TEXT("Failed to create socket handle for %s"), *GetName());
				}
			}
#endif //WITH_EDITOR	
			
		}
	}

	for (const auto& Socket : OldSockets)
	{
		URoomSocket* OldSocket = Socket.Value;
		if (OldSocket && OldSocket->Handle)
		{
			OldSocket->Handle->Destroy();
		}
	}
}


TArray<FName> ARoomNode::GetRoomSocketNames() const
{
	TArray<FName> Arr;
	Sockets.GenerateKeyArray(Arr);
	return Arr;
}

TArray<URoomSocket*> ARoomNode::GetRoomSockets() const
{
	TArray<URoomSocket*> Arr;
	Sockets.GenerateValueArray(Arr);
	return Arr;
}


bool ARoomNode::GetRoomSocket(FName Name, URoomSocket*& Socket) const
{
	URoomSocket* const* SocketPtr = Sockets.Find(Name);
	Socket = (SocketPtr != nullptr) ? *SocketPtr : nullptr;
	return (SocketPtr != nullptr);
}

bool ARoomNode::SnapWithNode(FName Socket, ARoomNode* TargetRoom, FName TargetSocket)
{
	if (!CanMove())
	{
		UE_LOG(LogRoomWorld, Error, TEXT("RoomNode %s snap with %s failed: Node can't move"), *GetNameSafe(this), *GetNameSafe(TargetRoom));
		return false;
	}

	if (TargetRoom == nullptr || TargetRoom == this)
	{
		UE_LOG(LogRoomWorld, Error, TEXT("RoomNode %s snap with %s failed: Invalid target"), *GetNameSafe(this), *GetNameSafe(TargetRoom));
		return false;
	}

	URoomSocket* LocalSocketData = nullptr;
	if (!GetRoomSocket(Socket, LocalSocketData))
	{
		UE_LOG(LogRoomWorld, Error, TEXT("RoomNode %s snap with %s failed: Socket '%s' does not exist in %s"), *GetNameSafe(this), *GetNameSafe(TargetRoom), *Socket.ToString(), *GetNameSafe(this));
		return false;
	}

	URoomSocket* TargetSocketData = nullptr;
	if (!TargetRoom->GetRoomSocket(TargetSocket, TargetSocketData))
	{
		UE_LOG(LogRoomWorld, Error, TEXT("RoomNode %s snap with %s failed: Socket '%s' does not exist in %s"), *GetNameSafe(this), *GetNameSafe(TargetRoom), *TargetSocket.ToString(), *GetNameSafe(TargetRoom));
		return false;
	}

	ensure(LocalSocketData);
	ensure(TargetSocketData);

	if (!LocalSocketData || !TargetSocketData)
	{
		UE_LOG(LogRoomWorld, Error, TEXT("RoomNode %s snap with %s failed: missing socket data"), *GetNameSafe(this), *GetNameSafe(TargetRoom));
		return false;
	}

	FTransform Result = FTransform((LocalSocketData->Rotation + FRotator(0, 180, 0)).Quaternion(), LocalSocketData->Location).Inverse() * TargetSocketData->GetTransform();
	SetActorTransform(Result);
	return true;

}

bool ARoomNode::GetConnectedNodes(TArray<ARoomNode*>& Nodes, TSubclassOf<ARoomNode> ClassFilter) const
{
	Nodes.Reset();
	for (const auto& Pair : Sockets)
	{
		URoomSocket* Socket = Pair.Value;
		if (Socket)
		{
			TArray<ARoomNode*> ConnectedNodes = Socket->GetConnectedNodes();
			for (ARoomNode* Node : ConnectedNodes)
			{
				if (!ClassFilter || Node->IsA(ClassFilter))
				{
					Nodes.AddUnique(Node);
				}
			}
		}
	}

	return Nodes.Num() > 0;
}

#undef LOCTEXT_NAMESPACE 