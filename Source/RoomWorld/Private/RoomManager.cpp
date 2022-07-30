// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomManager.h"
#include "RoomNode.h"

#include <Misc/UObjectToken.h>
#include <Logging/MessageLog.h>
#include <EngineUtils.h>



ARoomManager::ARoomManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARoomManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ARoomManager::PreInitializeComponents()
{
	if (ULevel* Level = GetLevel())
	{
		for (AActor* LevelActor : Level->Actors)
		{
			if (IsValid(LevelActor))
			{
				if (ARoomNode* Node = Cast<ARoomNode>(LevelActor))
				{
					RegisterNode(Node);					
				}
#if !UE_BUILD_SHIPPING
				else if (ARoomManager* OtherManager = Cast<ARoomManager>(LevelActor))
				{
					if (OtherManager != this)
					{
						FMessageLog("MapCheck").Error()
							->AddToken(FUObjectToken::Create(this))
							->AddToken(FTextToken::Create(NSLOCTEXT("Actor", "MapCheck_Message_ExtraManagers", "Only one RoomManager allowed per Level")));
					}
				}
#endif //!UE_BUILD_SHIPPING
			}
		}

	}

	Super::PreInitializeComponents();	
}

void ARoomManager::BeginPlay()
{
	Super::BeginPlay();	
}

#if WITH_EDITOR
void ARoomManager::CheckForErrors()
{
	if (ULevel* Level = GetLevel())
	{
		int32 NumManagers = 0;
		for (AActor* LevelActor : Level->Actors)
		{
			if (IsValid(LevelActor) && LevelActor->IsA<ARoomManager>())
			{
				NumManagers++;
			}
		}

		if (NumManagers > 1)
		{
			FMessageLog("MapCheck").Error()
				->AddToken(FUObjectToken::Create(this))
				->AddToken(FTextToken::Create(NSLOCTEXT("Actor", "MapCheck_Message_ExtraManagers", "Only one RoomManager allowed per Level")));
		}
	}
}
#endif //WITH_EDITOR




void ARoomManager::RegisterNode(ARoomNode* Node)
{
	Node->Manager = this;
	ReceiveRegisterNode(Node);

	// Node could be destroyed during register
	if (IsValid(Node))
	{
		AllNodes.Add(Node);
	}
}

void ARoomManager::GetAllNodesOfClass(TSubclassOf<ARoomNode> NodeClass, TArray<ARoomNode*>& OutNodes)
{
	OutNodes.Reset();

	for (ARoomNode* Node : AllNodes)
	{
		if (IsValid(Node) && (!NodeClass || Node->IsA(NodeClass)))
		{
			OutNodes.Add(Node);
		}
	}
}