


#include "Socket/RoomSocketHandle.h"

namespace RoomWorldCVars
{
	static int32 RoomSocketsVisibility = 1;
	FAutoConsoleVariableRef CVarShowRoomSockets(
		TEXT("ShowRoomSockets"),
		RoomSocketsVisibility,
		TEXT("Whether to show RoomSockets in Outliner\n")
		TEXT("0: Hidden, 1: Visible"),
		ECVF_Default);
}


ARoomSocketHandle::ARoomSocketHandle()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	bIsEditorOnlyActor = true;

#if WITH_EDITORONLY_DATA
	bListedInSceneOutliner = false;	
#endif //WITH_EDITORONLY_DATA
}

#if WITH_EDITOR

void ARoomSocketHandle::PostLoad()
{
	Super::PostLoad();
}

void ARoomSocketHandle::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	GEngine->OnActorMoved().AddUObject(this, &ThisClass::ActorMoved);
}

bool ARoomSocketHandle::EditorCanAttachTo(const AActor* InParent, FText& OutReason) const
{
	return Super::EditorCanAttachTo(InParent, OutReason) && (Socket && Socket->GetNode() == InParent);
}

bool ARoomSocketHandle::CanDeleteSelectedActor(FText& OutReason) const
{
	if (Socket && Socket->GetNode())
	{
		OutReason = FText::FromString(TEXT("Controlled by room"));
		return false;
	}

	return Super::CanDeleteSelectedActor(OutReason);
}

bool ARoomSocketHandle::IsListedInSceneOutliner() const
{	
	return Super::IsListedInSceneOutliner() || RoomWorldCVars::RoomSocketsVisibility != 0;
}

void ARoomSocketHandle::ActorMoved(AActor* Actor)
{
	if (Actor && Socket)
	{	
		const TArray<URoomSocket*>& Connections = Socket->ConnectedTo;
		for (int32 Index = 0; Index < Connections.Num(); Index++)
		{
			if (Connections[Index] && (Connections[Index]->Handle == Actor || Connections[Index]->GetNode() == Actor))
			{
				FEditorScriptExecutionGuard ScriptGuard;
				ReceiveConnectionMoved(Index);
			}
		}

		if (Actor == Socket->GetNode())
		{
			FEditorScriptExecutionGuard ScriptGuard;
			ReceiveNodeMoved();
		}
	}
}
#endif // WITH_EDITOR


void ARoomSocketHandle::SetSocket(URoomSocket* InSocket)
{
	Socket = InSocket;

#if WITH_EDITORONLY_DATA
	Class = Socket ? Socket->GetClass()->GetDisplayNameText().ToString() : TEXT("");
#endif
}

void ARoomSocketHandle::ToggleSocketVisibility()
{
	RoomWorldCVars::RoomSocketsVisibility = !RoomWorldCVars::RoomSocketsVisibility;
}
