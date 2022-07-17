

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RoomWorldFunctionLibrary.generated.h"

/**  */
USTRUCT(BlueprintType)
struct FSoftLevelPath
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowedClasses = "World"))
	FSoftObjectPath Path;	
};

/**
 * 
 */
UCLASS()
class ROOMWORLD_API URoomWorldFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintPure, Category = "Room World")
	static TSoftObjectPtr<UWorld> GetWorldPtrFromSoftObjectPath(const FSoftObjectPath& Path);

	UFUNCTION(BlueprintPure, Category = "Room World")
	static const ULevel* GetActorLevel(AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Room World")
	static const ULevel* GetComponentLevel(UActorComponent* Component);

	UFUNCTION(BlueprintPure, Category = "Room World")
	static FSoftObjectPath GetStreamedLevelAssetPath(const ULevel* Level);


	/** Attempts to locate Actor in level using passed in ActorPtr */
	UFUNCTION(BlueprintPure, Category = "Room World")
	static TSoftObjectPtr<AActor> ResolveSoftActorPtrUsingLevel(const TSoftObjectPtr<AActor>& Actor, const ULevel* Level);

	/** Attempts to locate object in level using passed in object path */
	UFUNCTION(BlueprintPure, Category = "Room World")
	static FSoftObjectPath ResolveSoftObjectPathUsingLevel(const FSoftObjectPath& Object, const ULevel* Level);


	static FSoftObjectPath ResolveSoftObjectPathUsingLevelPath(const FSoftObjectPath& Path, const ULevel* Level, const FName& StreamedLevelPath);


	/** Calculate world transform that aligns LocalSocket world transform with Target transform */
	UFUNCTION(BlueprintPure, Category = "Room World")
	FTransform CalculateBackToBackAttachTransform(const FTransform& LocalSocketRelativeTransform, const FTransform& TargetWorldTransform);

	/** 
	 * Fix up all references in object from pointing to actors in level asset to actors in specified level
	 */
	UFUNCTION(BlueprintCallable, Category = "Room World")
	static void ResolveObjectReferencesInObjectUsingLevel(UObject* Target, const ULevel* Level);


	
	UFUNCTION(BlueprintCallable, Category = "Room World")
	static FTransform CalcSliceTransformAtSplineOffset(USplineMeshComponent* Comp, const float Alpha);
};
