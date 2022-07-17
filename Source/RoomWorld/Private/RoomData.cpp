// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomData.h"
#include "RoomWorldSettings.h"


void URoomData::PostLoad()
{
	Super::PostLoad();
}

URoomData::URoomData()
{
	SocketTags.Add(URoomWorldSettings::Get()->DefaultSocketTag);
}


#if WITH_EDITOR

void URoomData::CollectLevelData()
{
	Messages.Empty();
	SocketData.Empty();

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
					if (!SocketData.Contains(SocketName))
					{
						SocketData.Add(SocketName, { Actor->GetActorTransform() });
					}
					else
					{
						Messages.Add(FString::Printf(TEXT("Error: Socket '%s' already exists. Actor %s"), *SocketNameString, *GetNameSafe(Actor)));
					}					
				}
			}
		}
	}

	MarkPackageDirty();
}

#endif //WITH_EDITOR