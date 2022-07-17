

#pragma once

#include "CoreMinimal.h"
#include "Components/BillboardComponent.h"
#include "RoomTypes.h"
#include "RoomStateIconComponent.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintable, classGroup = (RoomWorld), meta = (BlueprintSpawnableComponent))
class ROOMWORLD_API URoomStateIconComponent : public UBillboardComponent
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Room)
	UTexture2D* State_Unloaded;

	UPROPERTY(EditDefaultsOnly, Category = Room)
	UTexture2D* State_Loaded;

	UPROPERTY(EditDefaultsOnly, Category = Room)
	UTexture2D* State_Visible;
	
public:
	URoomStateIconComponent();
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:	
	UFUNCTION()
	void StateChenged(class ARoom* Room, ERoomState NewState);
};
