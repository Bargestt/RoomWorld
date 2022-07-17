

#pragma once

#include "CoreMinimal.h"
#include "RoomSocket.generated.h"


class ARoomSocketHandle;

/**
 *
 */
UCLASS(BlueprintType, ClassGroup = (RoomWorld))
class ROOMWORLD_API URoomSocket : public UObject
	, public IRoomListenerInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Socket)
	ARoomNode* Room;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Socket)
	FName Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Socket)
	FVector Location;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Socket)
	FRotator Rotation;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Handle)
	TSubclassOf<ARoomSocketHandle> HandleClass;

	UPROPERTY()
	ARoomSocketHandle* Handle;


	UPROPERTY()
	TArray<URoomSocket*> ConnectedTo;

	UPROPERTY(EditInstanceOnly, Category = Socket, meta = (DisplayName = "ConnectedTo"))
	TArray<ARoomSocketHandle*> ConnectedToHandles;	

public:
	// Sets default values for this actor's properties
	URoomSocket();

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	virtual void Initialize(FName Name, const struct FSocketData& Data);


	/** Apply handles change to connections */
	virtual void SyncConnections()
	{

	}

	/** Ensure handles represent connections state */
	virtual void SyncHandles();


	/** Changes existing connection at specified index and returns true if connection was changed. Doesn't change other socket*/
	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	virtual bool SetConnection(URoomSocket* Connection, int32 Index)
	{
		unimplemented();
		return false;
	}

	/** Adds new connection */
	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	virtual bool AddConnection(URoomSocket* Connection)
	{
		unimplemented();
		return false;
	}

	/** Finds and clears connection. Returns true if connection was removed */
	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	virtual bool ClearConnection(URoomSocket* Connection)
	{
		unimplemented();
		return false;
	}

	/** Clears connection. Returns true if connection was removed */
	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	virtual bool ClearConnectionAt(int32 Index)
	{
		unimplemented();
		return false;
	}

	/** Additional check before setting connection. Uses INDEX_NONE when adding new connection */
	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	virtual bool CanSetConnection(URoomSocket* NewConnection, int32 Index) const
	{
		return true;
	}

	/** Additional check before clearing connection */
	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	virtual bool CanClearConnection(URoomSocket* CurrentConnection, int32 Index) const
	{
		return true;
	}

	/** Remove connections to nullptr */
	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	void RemoveEmptyConnections();


	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	ARoomNode* GetNode() const;

	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	ARoomNode* GetConnectedNode(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	TArray<ARoomNode*> GetConnectedNodes() const;


	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	FTransform GetRelativeTransform() const;

	/** In world space */
	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	FTransform GetTransform() const;


	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	TArray<URoomSocket*> GetConnections() const;

	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	int32 GetNumConnections() const;

	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	bool HasConnections() const;

	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	URoomSocket* GetConnection(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	int32 GetConnectionIndex(URoomSocket* Socket) const;

	UFUNCTION(BlueprintCallable, Category = "RoomWorld|Socket")
	bool IsConnectedTo(URoomSocket* Socket) const;	



protected:

	virtual void OnConnectionGained(URoomSocket* NewConnection, int32 Index);

	/** Called when connection related to this actor is created  */
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveConnectionGained(URoomSocket* NewConnection, int32 Index);



	virtual void OnConnectionLost(URoomSocket* OldConnection, int32 Index);

	/** Called when connection related to this actor is destroyed  */
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveConnectionLost(URoomSocket* OldConnection, int32 Index);
};
