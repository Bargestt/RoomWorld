


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

	NodeColor = FLinearColor::Transparent;
}

void ARoomNode::PostLoad()
{
	Super::PostLoad();

	RebuildSockets();

	ForEachAttachedActors([this](AActor* Actor) 
	{ 
		if (ARoomSocketHandle* Handle = Cast<ARoomSocketHandle>(Actor))
		{
			if (Handle->Socket == nullptr || Sockets.FindKey(Handle->Socket) == nullptr)
			{
				Handle->Destroy();
			}
		}

		return true; 
	});
}

void ARoomNode::PostInitializeComponents()
{
	InitialPosition = GetActorTransform();

	Super::PostInitializeComponents();
}	

void ARoomNode::OnConstruction(const FTransform& Transform)
{
	if (NodeColor == FLinearColor::Transparent)
	{
		NodeColor = FLinearColor::MakeRandomColor();
	}

	RebuildSockets();

	ForEachComponent<UActorComponent>(false, [this](UActorComponent* Comp)
	{
		if (IsValid(Comp) && Comp->Implements<URoomConstructionNotifyInterface>())
		{
			IRoomConstructionNotifyInterface::Execute_OnConstruct(Comp, this);
		}
	});

	ForEachAttachedActors([this](AActor* Actor) 
	{ 
		if (ARoomSocketHandle* Handle = Cast<ARoomSocketHandle>(Actor))
		{
			if (Handle->Socket == nullptr || Sockets.FindKey(Handle->Socket) == nullptr)
			{
				Handle->Destroy();
			}
		}

		if (IsValid(Actor) && Actor->Implements<URoomConstructionNotifyInterface>())
		{
			IRoomConstructionNotifyInterface::Execute_OnConstruct(Actor, this);
		}

		return true; 
	});

	Super::OnConstruction(Transform);
}

void ARoomNode::Destroyed()
{
	Super::Destroyed();

	ForEachAttachedActors([this](AActor* Actor) 
	{ 
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}	

		return true; 
	});
}



const TMap<FName, FSocketData>& ARoomNode::GetSocketData() const
{
	static const TMap<FName, FSocketData> Empty;
	return Empty;
}

void ARoomNode::RebuildSockets()
{
	const TMap<FName, FSocketData>& SocketData = GetSocketData();

	TMap<FName, URoomSocket*> OldSockets = Sockets;
	Sockets.Empty();

	if (SocketClass)
	{
		for (const auto& Pair : SocketData)
		{
			const FName& SocketName = Pair.Key;
			const FSocketData& Data = Pair.Value;

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


void ARoomNode::OnSnapped(FName Socket, ARoomNode* OtherNode, FName OtherSocket)
{
	ReceiveWasSnapped(Socket, OtherNode, OtherSocket);
}

bool ARoomNode::SnapWithNode(FName Socket, ARoomNode* TargetNode, FName TargetSocket)
{
#if WITH_EDITOR
	if (GetWorld() && GetWorld()->IsPreviewWorld() && !bCanMoveInEditor)
	{
		return false;
	}
#endif // WITH_EDITOR


	if (!CanMove())
	{
		UE_LOG(LogRoomWorld, Error, TEXT("RoomNode %s snap with %s failed: Node can't move"), *GetNameSafe(this), *GetNameSafe(TargetNode));
		return false;
	}

	if (TargetNode == nullptr || TargetNode == this)
	{
		UE_LOG(LogRoomWorld, Error, TEXT("RoomNode %s snap with %s failed: Invalid target"), *GetNameSafe(this), *GetNameSafe(TargetNode));
		return false;
	}

	URoomSocket* LocalSocketData = nullptr;
	if (!GetRoomSocket(Socket, LocalSocketData))
	{
		UE_LOG(LogRoomWorld, Error, TEXT("RoomNode %s snap with %s failed: Socket '%s' does not exist in %s"), *GetNameSafe(this), *GetNameSafe(TargetNode), *Socket.ToString(), *GetNameSafe(this));
		return false;
	}

	URoomSocket* TargetSocketData = nullptr;
	if (!TargetNode->GetRoomSocket(TargetSocket, TargetSocketData))
	{
		UE_LOG(LogRoomWorld, Error, TEXT("RoomNode %s snap with %s failed: Socket '%s' does not exist in %s"), *GetNameSafe(this), *GetNameSafe(TargetNode), *TargetSocket.ToString(), *GetNameSafe(TargetNode));
		return false;
	}

	ensure(LocalSocketData);
	ensure(TargetSocketData);

	if (!LocalSocketData || !TargetSocketData)
	{
		UE_LOG(LogRoomWorld, Error, TEXT("RoomNode %s snap with %s failed: missing socket data"), *GetNameSafe(this), *GetNameSafe(TargetNode));
		return false;
	}

	FTransform Result = FTransform((LocalSocketData->Rotation + FRotator(0, 180, 0)).Quaternion(), LocalSocketData->Location).Inverse() * TargetSocketData->GetTransform();
	SetActorTransform(Result);
	OnSnapped(Socket, TargetNode, TargetSocket);

	return true;

}

void ARoomNode::SnapAllConnectedNodes()
{
	ForEachSocket([this](URoomSocket* Socket)
	{
		for (URoomSocket* ConnectedSocket : Socket->ConnectedTo)
		{
			ARoomNode* ConnectedNode = ConnectedSocket ? ConnectedSocket->GetNode() : nullptr;
			if (ConnectedNode && ConnectedNode->CanMove())
			{
				ConnectedNode->SnapWithNode(ConnectedSocket->Name, this, Socket->Name);
			}
		}
	});
}

bool ARoomNode::GetConnectedNodes(TArray<ARoomNode*>& Nodes, TSubclassOf<ARoomNode> ClassFilter) const
{
	Nodes.Reset();
	ForEachSocket([&Nodes, &ClassFilter](URoomSocket* Socket)
	{
		TArray<ARoomNode*> ConnectedNodes = Socket->GetConnectedNodes();
		for (ARoomNode* Node : ConnectedNodes)
		{
			if (!ClassFilter || Node->IsA(ClassFilter))
			{
				Nodes.AddUnique(Node);
			}
		}
	});	

	return Nodes.Num() > 0;
}

#undef LOCTEXT_NAMESPACE 