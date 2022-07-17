


#include "RoomWorldFunctionLibrary.h"
#include <LevelUtils.h>
#include <UObject/UnrealType.h>
#include <ProfilingDebugging/ScopedTimers.h>

TSoftObjectPtr<UWorld> URoomWorldFunctionLibrary::GetWorldPtrFromSoftObjectPath(const FSoftObjectPath& Path)
{
	return TSoftObjectPtr<UWorld>(Path);
}

const ULevel* URoomWorldFunctionLibrary::GetActorLevel(AActor* Actor)
{
	return Actor ? Actor->GetLevel() : nullptr;
}

const ULevel* URoomWorldFunctionLibrary::GetComponentLevel(UActorComponent* Component)
{
	return Component ? Component->GetComponentLevel() : nullptr;
}

FSoftObjectPath URoomWorldFunctionLibrary::GetStreamedLevelAssetPath(const ULevel* Level)
{
	// Copied from ULevelSequencePlayer::Initialize

	FString StreamedLevelAssetPath;

	// Construct the path to the level asset that the streamed level relates to
	ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel(Level);
	if (LevelStreaming)
	{
		// StreamedLevelPackage is a package name of the form /Game/Folder/MapName, not a full asset path
		FString StreamedLevelPackage = (LevelStreaming->PackageNameToLoad == NAME_None ? LevelStreaming->GetWorldAssetPackageFName() : LevelStreaming->PackageNameToLoad).ToString();

		int32 SlashPos = 0;
		if (StreamedLevelPackage.FindLastChar('/', SlashPos) && SlashPos < StreamedLevelPackage.Len() - 1)
		{
			// Construct the asset path by appending .MapName to the end for efficient comparison with FSoftObjectPath::GetAssetPathName
			const TCHAR* Pair[] = { *StreamedLevelPackage, &StreamedLevelPackage[SlashPos + 1] };
			StreamedLevelAssetPath = *FString::Join(Pair, TEXT("."));
		}
	}

	return FSoftObjectPath(StreamedLevelAssetPath);
}

TSoftObjectPtr<AActor> URoomWorldFunctionLibrary::ResolveSoftActorPtrUsingLevel(const TSoftObjectPtr<AActor>& Actor, const ULevel* Level)
{
	return TSoftObjectPtr<AActor>(ResolveSoftObjectPathUsingLevel(Actor.ToSoftObjectPath(), Level));
}

FSoftObjectPath URoomWorldFunctionLibrary::ResolveSoftObjectPathUsingLevel(const FSoftObjectPath& Object, const ULevel* Level)
{
	return ResolveSoftObjectPathUsingLevelPath(Object, Level, GetStreamedLevelAssetPath(Level).GetAssetPathName());
}

PRAGMA_DISABLE_OPTIMIZATION
FSoftObjectPath URoomWorldFunctionLibrary::ResolveSoftObjectPathUsingLevelPath(const FSoftObjectPath& Path, const ULevel* Level, const FName& StreamedLevelPath)
{
	FSoftObjectPath NewPath;

	if (Path.IsValid() && !Path.GetSubPathString().IsEmpty())
	{		
		//#TODO: Add handling expected level mismatch
		//if (StreamedLevelPath == NAME_None || StreamedLevelPath == Path.GetAssetPathName())

		// From FLevelSequenceBindingReference::Resolve
		UObject* ResolvedObject = FindObject<UObject>(Level->GetOuter(), *Path.GetSubPathString());
		NewPath = FSoftObjectPath(ResolvedObject);
	}

	return NewPath;
}


FTransform URoomWorldFunctionLibrary::CalculateBackToBackAttachTransform(const FTransform& LocalSocketRelativeTransform, const FTransform& TargetWorldTransform)
{
	return FTransform(LocalSocketRelativeTransform.GetRotation() * FRotator(0, 180, 0).Quaternion(), LocalSocketRelativeTransform.GetTranslation()).Inverse() * TargetWorldTransform;
}

void URoomWorldFunctionLibrary::ResolveObjectReferencesInObjectUsingLevel(UObject* Target, const ULevel* Level)
{
	if (!Target || !Level)
	{
		return;
	}	
	
	FAutoScopedDurationTimer Timer;

	FName LevelPath = GetStreamedLevelAssetPath(Level).GetAssetPathName();
	int32 NumResolved = 0;
	for (TPropertyValueIterator<FSoftObjectProperty> It(Target->GetClass(), Target); It; ++It)
	{
		FSoftObjectProperty* ObjectProp = CastField<FSoftObjectProperty>(It.Key());
		if (ObjectProp)
		{
			// Is modifying safe during iteration?
			FSoftObjectPtr* Value = const_cast<FSoftObjectPtr*>(ObjectProp->GetPropertyValuePtr(It.Value()));			
			*Value = ResolveSoftObjectPathUsingLevelPath(Value->ToSoftObjectPath(), Level, LevelPath);

			NumResolved++;
		}
	}

	UE_LOG(LogRoomWorld, Warning, TEXT("ResolveObjectReferencesInObjectUsingLevel: x%d in %f ms"), NumResolved, Timer.GetTime()*1000.0);
}
PRAGMA_ENABLE_OPTIMIZATION