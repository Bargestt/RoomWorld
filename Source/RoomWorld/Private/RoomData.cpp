// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomData.h"
#include "RoomWorldSettings.h"


URoomData::URoomData()
{
	SocketTags.Add(URoomWorldSettings::Get()->DefaultSocketTag);
}

void URoomData::PostLoad()
{
	Super::PostLoad();
}


#if WITH_EDITOR

void URoomData::CollectLevelData()
{
	Messages.Empty();
	SocketData.Empty();
	BoundingBox = FBox();

	bool bImplementsCollector = GetClass()->IsFunctionImplementedInScript(TEXT("CollectSocketDataFromActor"));

	UWorld* WorldAsset = Cast<UWorld>(LevelPath.TryLoad());
	if (WorldAsset)
	{
		bool bCollapseDefaultTag = URoomWorldSettings::Get()->bCollapseDefaultSocketTag;
		FName DefaultSocket = URoomWorldSettings::Get()->DefaultSocketTag;

		for (AActor* Actor : WorldAsset->PersistentLevel->Actors)
		{
			if (!Actor)
			{
				continue;
			}

			if (Actor->IsLevelBoundsRelevant())
			{
				BoundingBox += Actor->CalculateComponentsBoundingBoxInLocalSpace();
			}

			// Collect socket data
			for (const FName& SocketTag : SocketTags)
			{
				if (Actor->ActorHasTag(SocketTag))
				{
					FString SocketNameString = Actor->GetActorLabel();
					if (!bCollapseDefaultTag || SocketTag != DefaultSocket)
					{
						SocketNameString = FString::Printf(TEXT("%s.%s"), *SocketTag.ToString(), *SocketNameString);
					}

					FName SocketName = *SocketNameString;

					const FSocketData* ExistingData = SocketData.Find(SocketName);
					if (ExistingData == nullptr)
					{
						FSocketData Data;
						Data.Transform = Actor->GetActorTransform();
						if (bImplementsCollector)
						{
							Data = CollectSocketDataFromActor(Actor);
						}						
						Data.SourceActor = Actor;

						SocketData.Add(SocketName, Data);
					}
					else
					{
						Messages.Add(FString::Printf(TEXT("Error: Failed to add '%s' from %s already created by %s"), *SocketNameString, *GetNameSafe(Actor), *ExistingData->SourceActor.GetSubPathString()));
					}					
				}
			}
		}
	}

	MarkPackageDirty();
}

#endif //WITH_EDITOR