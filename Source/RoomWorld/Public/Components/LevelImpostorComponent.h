

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "LevelImpostorComponent.generated.h"


UCLASS(NotBlueprintable, classGroup = (RoomWorld))
class ROOMWORLD_API ALevelImpostorActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Level Impostor")
	bool bUpdate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Impostor", meta = (AllowedClasses = "World"))
	FSoftObjectPath LevelPath;

	UPROPERTY(VisibleAnywhere, Category = "Level Impostor")
	class ULevelImpostorComponent* Component;

	

public:
	ALevelImpostorActor();

	void Update();

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);

		if (PropertyChangedEvent.GetPropertyName() == TEXT("bUpdate"))
		{
			bUpdate = false;
			Update();
		}
	}
#endif // WITH_EDITOR
};


/**
 * 
 */
UCLASS(BlueprintType, ClassGroup = (RoomWorld), meta = (BlueprintSpawnableComponent))
class ROOMWORLD_API ULevelImpostorComponent : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()
	
public:

	/** In m^3 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Impostor")
	float MinVolume;

	/** In m^3 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Impostor")
	float MaxVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Impostor")
	int32 MaxNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Impostor")
	bool bDetailed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Impostor")
	bool bCollidingOnly;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Impostor")
	bool bVisibleOnly;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Impostor")
	TArray<TSubclassOf<AActor>> IgnoreActors;

public:
	ULevelImpostorComponent();

	UFUNCTION(BlueprintCallable, Category = "Level Impostor")
	void Update(FSoftObjectPath LevelPath);
};
