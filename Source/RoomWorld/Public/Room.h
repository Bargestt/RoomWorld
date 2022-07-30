

#pragma once

#include "CoreMinimal.h"
#include "RoomNode.h"
#include "Room.generated.h"

class ULevelStreamingDynamic;
class URoomStateControlComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRoomStateChangedDynamic, ARoom*, Room, ERoomState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRoomPlayerEventDynamic, ARoom*, Room, APlayerController*, Player);



UCLASS(Abstract, ClassGroup = (RoomWorld))
class ROOMWORLD_API ARoom : public ARoomNode
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category = Room)
	FRoomStateChangedDynamic OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = Room)
	FRoomPlayerEventDynamic OnPlayerEnter;

	UPROPERTY(BlueprintAssignable, Category = Room)
	FRoomPlayerEventDynamic OnPlayerLeave;
	

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Room)
	class URoomData* RoomData;

	/** Instantly change state when no level streaming created otherwise log error when room fails to create level streaming */
	UPROPERTY(EditDefaultsOnly, Category = Room)
	bool bAllowNoLevel;

private:
	UPROPERTY()
	ULevelStreamingDynamic* LevelStreaming;

	UPROPERTY()
	ERoomState DesiredState;


	// Waiting to finish begin play to run EnsureState
	uint8 bPendingEnsureState : 1;


public:	
	ARoom();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


public:
//~ Begin ARoomNode Interface
	virtual bool CanMove() const override;
	virtual const TMap<FName, FSocketData>& GetSocketData() const override;
//~ End ARoomNode Interface



	virtual ULevelStreamingDynamic* InitializeLevelStreaming();	

public:
	UFUNCTION(BlueprintCallable, Category = Room)
	void SetState(ERoomState State);	

	UFUNCTION(BlueprintCallable, Category = Room)
	ERoomState GetCurrentState() const;

	void EnsureState();

	/** Sets room data to load. Not allowed while room is loaded or shown  */
	UFUNCTION(BlueprintCallable, Category = Room)
	bool SetRoomData(URoomData* NewRoomData);

	/** Get level asset that will be loaded or currently loaded level */
	UFUNCTION(BlueprintCallable, Category = Room)
	TSoftObjectPtr<UWorld> GetLevelAsset() const;

	UFUNCTION(BlueprintCallable, Category = Room)
	ULevelStreamingDynamic* GetLevelStreaming() const;

	// Events
protected:		
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "StateChanged"), Category = "Room")
	void ReceiveStateChanged(ERoomState NewState);


	UFUNCTION()
	virtual void HandleLevelLoaded();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "LevelLoaded"), Category = "Room")
	void ReceiveLevelLoaded();

	UFUNCTION()
	virtual void HandleLevelUnloaded();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "LevelUnloaded"), Category = "Room")
	void ReceiveLevelUnloaded();


	UFUNCTION()
	virtual void HandleLevelShown();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "LevelShown"), Category = "Room")
	void ReceiveLevelShown();


	UFUNCTION()
	virtual void HandleLevelHidden();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "LevelHidden"), Category = "Room")
	void ReceiveLevelHidden();



	/*--------------------------------------------
	 	Visibility requests
	 *--------------------------------------------*/
protected:

	/** Keep room visible while at leas one is valid */
	UPROPERTY(VisibleAnywhere, Category = Room)
	TArray<TWeakObjectPtr<UObject>> VisibilityRequests;

public:

	/** 
	 * Request room to become visible. Room is kept visible while at leas one requester is valid 
	 * Automatically cleared If requested by Actor
	 */
	UFUNCTION(BlueprintCallable, Category = Room)
	void RequestVisible(UObject* Requester);

	UFUNCTION(BlueprintCallable, Category = Room)
	void ClearVisible(UObject* Requester);
	
	/** Remove all visibility requests and unload */
	UFUNCTION(BlueprintCallable, Category = Room)
	void ResetVisibilityRequests();

	/** Updates room visibility and removes invalid requesters */
	UFUNCTION(BlueprintCallable, Category = Room)
	void UpdateVisibilityRequests();

	UFUNCTION(BlueprintCallable, Category = Room)
	void GetVisibilityRequesters(TArray<UObject*>& Requesters) const;

protected:
	UFUNCTION()
	void RequesterDestroyed_Actor(AActor* Requester);



	/*--------------------------------------------
		Utility
	 *--------------------------------------------*/
public:

	template<typename Func>
	void ForEachActorInLevel(Func InFunc) const
	{
		if (LevelStreaming && LevelStreaming->GetLoadedLevel())
		{
			for (AActor* LevelActor : LevelStreaming->GetLoadedLevel()->Actors)
			{
				if (IsValid(LevelActor))
				{
					InFunc(LevelActor);
				}
			}
		}
	}
};

