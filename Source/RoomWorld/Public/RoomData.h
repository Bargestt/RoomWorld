// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoomTypes.h"
#include "RoomData.generated.h"



/**
 * 
 */
UCLASS(Blueprintable, ClassGroup = (RoomWorld))
class ROOMWORLD_API URoomData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Room, meta = (AllowedClasses = "World"))
	FSoftObjectPath LevelPath;

	/** Room bounds in room space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Room)
	FBox BoundingBox;

	/** 
	 * Collect sockets from level using these tags. 
	 * Sockets are named using actor labels: {SocketTag}.{ActorLabel}
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Room)
	TArray<FName> SocketTags;

	/** Used to attach rooms to each other */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Room)
	TMap<FName, FSocketData> SocketData;

#if WITH_EDITOR
	UPROPERTY(VisibleAnywhere, Transient, Category = Room)
	TArray<FString> Messages;
#endif //WITH_EDITOR

public:
	URoomData();
	void PostLoad() override;

#if WITH_EDITOR

	UFUNCTION(BlueprintCallable, Category = Room)
	void CollectLevelData();

#endif //WITH_EDITOR

	
	UFUNCTION(BlueprintImplementableEvent)
	FSocketData CollectSocketDataFromActor(AActor* Actor) const;

	UFUNCTION(BlueprintCallable, Category = Room)
	TSoftObjectPtr<UWorld> GetSoftLevelPtr() const { return TSoftObjectPtr<UWorld>(LevelPath); }
};
