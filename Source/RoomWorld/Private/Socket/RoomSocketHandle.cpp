


#include "Socket/RoomSocketHandle.h"



ARoomSocketHandle::ARoomSocketHandle()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	bIsEditorOnlyActor = true;
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