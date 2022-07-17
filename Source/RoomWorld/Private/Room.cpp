


#include "Room.h"
#include "RoomListenerInterface.h"
#include "RoomConstructionNotifyInterface.h"

#include <Engine/LevelStreamingDynamic.h>

#include <Logging/MessageLog.h>
#include <Logging/TokenizedMessage.h>
#include <Misc/UObjectToken.h>
#include "RoomOwnedObjectInterface.h"

#include "Socket/RoomSocket.h"
#include "Socket/RoomSocketHandle.h"
#include "RoomData.h"

#include "Socket/RoomSocket_Single.h"



ARoom::ARoom()
{
	
}

void ARoom::OnConstruction(const FTransform& Transform)
{
	InitializeSockets(RoomData ? RoomData->SocketData : TMap<FName, FSocketData>());

	Super::OnConstruction(Transform);

	const TSet<UActorComponent*>& AllComponents = GetComponents();
	for (UActorComponent* Comp : AllComponents)
	{
		if (Comp && Comp->Implements<URoomConstructionNotifyInterface>())
		{
			IRoomConstructionNotifyInterface::Execute_OnConstruct(Comp, this);
		}
	}

	TArray<AActor*> Attached;
	GetAttachedActors(Attached);
	for (AActor* Actor : Attached)
	{
		if (IsValid(Actor) && Actor->Implements<URoomConstructionNotifyInterface>())
		{
			IRoomConstructionNotifyInterface::Execute_OnConstruct(Actor, this);
		}
	}
}

void ARoom::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ARoom::Destroyed()
{
	Super::Destroyed();
}

void ARoom::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetState(ERoomState::Unloaded);	
	ResetVisibilityRequests();

	if (LevelStreaming)
	{
		LevelStreaming->SetShouldBeVisible(false);
		LevelStreaming->SetShouldBeLoaded(false);
	}

	Super::EndPlay(EndPlayReason);
}


bool ARoom::CanMove() const
{
	return GetCurrentState() != ERoomState::Visible;
}

ULevelStreamingDynamic* ARoom::InitializeLevelStreaming()
{
	TSoftObjectPtr<UWorld> LevelAsset = RoomData ? TSoftObjectPtr<UWorld>(RoomData->LevelPath) : nullptr;
	FString LevelName = GetName() + TEXT("_Level");

	bool bSuccess;
	ULevelStreamingDynamic* LSD = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(this, LevelAsset, GetActorLocation(), GetActorRotation(), bSuccess, LevelName);
	if (LSD)
	{
		LSD->SetShouldBeLoaded(false);
		LSD->SetShouldBeVisible(false);
		LSD->bInitiallyLoaded = false;
		LSD->bInitiallyVisible = false;

		LSD->OnLevelShown.AddDynamic(this, &ThisClass::HandleLevelShown);
		LSD->OnLevelHidden.AddDynamic(this, &ThisClass::HandleLevelHidden);
		LSD->OnLevelLoaded.AddDynamic(this, &ThisClass::HandleLevelLoaded);
		LSD->OnLevelUnloaded.AddDynamic(this, &ThisClass::HandleLevelUnloaded);
	}

	return LSD;
}


void ARoom::SetState(ERoomState State)
{
	DesiredState = State;	
	EnsureState();
}

ERoomState ARoom::GetCurrentState() const
{
	if (LevelStreaming)
	{
		if (LevelStreaming->IsLevelVisible())
		{
			return ERoomState::Visible;
		}

		if (LevelStreaming->IsLevelLoaded())
		{
			return ERoomState::Loaded;
		}
	}

	return ERoomState::Unloaded;
}


void ARoom::EnsureState()
{
	if (!HasActorBegunPlay() && !IsActorBeginningPlay())
	{
		return;
	}

	ERoomState CurrentState = GetCurrentState();
	if (DesiredState != CurrentState)
	{	
		if (!LevelStreaming)
		{
			LevelStreaming = InitializeLevelStreaming();
		}

		if (LevelStreaming)
		{
			if (CurrentState != ERoomState::Visible)
			{
				LevelStreaming->LevelTransform.SetLocation(GetActorLocation());
				LevelStreaming->LevelTransform.SetRotation(GetActorQuat());
			}			

			switch (DesiredState)
			{
			case ERoomState::Unloaded:
				LevelStreaming->SetShouldBeLoaded(false);
				LevelStreaming->SetShouldBeVisible(false);
				break;
			case ERoomState::Loaded:
				LevelStreaming->SetShouldBeLoaded(true);
				LevelStreaming->SetShouldBeVisible(false);
				break;
			case ERoomState::Visible:
				LevelStreaming->SetShouldBeLoaded(true);
				LevelStreaming->SetShouldBeVisible(LevelStreaming->HasLoadedLevel()); // Ensure doesn't start before notifies are processed
				break;
			default:
				unimplemented();
				break;
			}
		}
		else
		{
			if (bAllowNoLevel)
			{
				ReceiveStateChanged(DesiredState);
				OnStateChanged.Broadcast(this, DesiredState);
			}
			else
			{
				FMessageLog("PIE").Error()
					->AddToken(FTextToken::Create(NSLOCTEXT("RoomWorld", "RoomName", "Room")))
					->AddToken(FUObjectToken::Create(this))
					->AddToken(FTextToken::Create(INVTEXT("Failed to create LevelStreaming object")));
			}
		}
	}
}


bool ARoom::SetRoomData(URoomData* NewRoomData)
{
	if (LevelStreaming && (LevelStreaming->ShouldBeLoaded() || LevelStreaming->ShouldBeVisible()))
	{
		return false;
	}

	LevelStreaming = nullptr;
	RoomData = NewRoomData;
	InitializeSockets(RoomData->SocketData);

	return true;
}

TSoftObjectPtr<UWorld> ARoom::GetLevelAsset() const
{
	if (LevelStreaming)
	{
		return LevelStreaming->GetWorldAsset();
	}

	return RoomData ? TSoftObjectPtr<UWorld>(RoomData->LevelPath) : nullptr;
}

ULevelStreamingDynamic* ARoom::GetLevelStreaming() const
{
	return LevelStreaming;
}

void ARoom::HandleLevelLoaded()
{
	for (AActor* LevelActor : LevelStreaming->GetLoadedLevel()->Actors)
	{
		if (LevelActor && LevelActor->Implements<URoomOwnedObjectInterface>())
		{
			IRoomOwnedObjectInterface::Execute_SetOwningRoom(LevelActor, this);
		}
	}

	ReceiveLevelLoaded();

	for (auto& Pair : Sockets)
	{
		if (URoomSocket* Socket = Pair.Value)
		{
			IRoomListenerInterface::Execute_OnRoomLoaded(Socket, this);
		}
	}
	
	ReceiveStateChanged(GetCurrentState());
	OnStateChanged.Broadcast(this, GetCurrentState());

	EnsureState();
}

void ARoom::HandleLevelUnloaded()
{
	ReceiveLevelUnloaded();
	for (auto& Pair : Sockets)
	{
		if (URoomSocket* Socket = Pair.Value)
		{
			IRoomListenerInterface::Execute_OnRoomUnloaded(Socket, this);
		}
	}

	ReceiveStateChanged(GetCurrentState());
	OnStateChanged.Broadcast(this, GetCurrentState());
}

void ARoom::HandleLevelShown()
{
	ReceiveLevelShown();
	for (auto& Pair : Sockets)
	{
		if (URoomSocket* Socket = Pair.Value)
		{
			IRoomListenerInterface::Execute_OnRoomShown(Socket, this);
		}
	}

	ReceiveStateChanged(GetCurrentState());
	OnStateChanged.Broadcast(this, GetCurrentState());
}

void ARoom::HandleLevelHidden()
{
	ReceiveLevelHidden();
	for (auto& Pair : Sockets)
	{
		if (URoomSocket* Socket = Pair.Value)
		{
			IRoomListenerInterface::Execute_OnRoomHidden(Socket, this);
		}
	}
	
	ReceiveStateChanged(GetCurrentState());
	OnStateChanged.Broadcast(this, GetCurrentState());
}


/*--------------------------------------------
 	Visibility requests
 *--------------------------------------------*/

void ARoom::RequestVisible(UObject* Requester)
{
	if (Requester)
	{
		VisibilityRequests.RemoveAllSwap([](const auto& Entry) { return !Entry.IsValid(); });

		if (!VisibilityRequests.Contains(Requester))
		{
			VisibilityRequests.Add(Requester);

			if (AActor* ActorRequester = Cast<AActor>(Requester))
			{
				ActorRequester->OnDestroyed.AddDynamic(this, &ThisClass::RequesterDestroyed_Actor);
			}

			UpdateVisibilityRequests();
		}
	}
}

void ARoom::ClearVisible(UObject* Requester)
{
	if (VisibilityRequests.Remove(Requester) > 0)
	{
		if (AActor* ActorRequester = Cast<AActor>(Requester))
		{
			ActorRequester->OnDestroyed.RemoveDynamic(this, &ThisClass::RequesterDestroyed_Actor);
		}
		UpdateVisibilityRequests();
	}
}

void ARoom::ResetVisibilityRequests()
{
	for (const auto& Requester : VisibilityRequests)
	{
		if (AActor* ActorRequester = Cast<AActor>(Requester))
		{
			ActorRequester->OnDestroyed.RemoveDynamic(this, &ThisClass::RequesterDestroyed_Actor);
		}
	}
	VisibilityRequests.Empty();
	UpdateVisibilityRequests();
}

void ARoom::UpdateVisibilityRequests()
{
	VisibilityRequests.RemoveAllSwap([](const auto& Entry) { return !Entry.IsValid(); });
	SetState((VisibilityRequests.Num() > 0) ? ERoomState::Visible : ERoomState::Unloaded);
}

void ARoom::GetVisibilityRequesters(TArray<UObject*>& Requesters) const
{	
	Requesters.Reset(VisibilityRequests.Num());
	for (const auto& Requester : VisibilityRequests)
	{
		Requesters.Add(Requester.Get());
	}
}

void ARoom::RequesterDestroyed_Actor(AActor* Requester)
{
	UpdateVisibilityRequests();
}


