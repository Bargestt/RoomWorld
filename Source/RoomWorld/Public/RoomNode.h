

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomTypes.h"
#include "RoomNode.generated.h"


/** 
 * Base class for nodes in RoomWorld 
 * Allows snapping using node sockets
 */
UCLASS(Abstract, BlueprintType, ClassGroup = (RoomWorld))
class ROOMWORLD_API ARoomNode : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = Socket)
	TSubclassOf<class URoomSocket> SocketClass;

	/** Default handle class when socket does not provide override. Disables handles when null */
	UPROPERTY(EditDefaultsOnly, Category = Socket)
	TSubclassOf<class ARoomSocketHandle> SocketHandleClass;

	UPROPERTY(VisibleAnywhere, Instanced, NonPIEDuplicateTransient, Category = Socket)
	TMap<FName, URoomSocket*> Sockets;

public:	
	ARoomNode();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Destroyed() override;
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


public:
	UFUNCTION(BlueprintCallable, Category = Room)
	virtual bool CanMove() const { return true; }


	virtual void InitializeSockets(const TMap<FName, FSocketData>& SocketData);


public:

	UFUNCTION(BlueprintCallable, Category = Room)
	TArray<FName> GetRoomSocketNames() const;

	UFUNCTION(BlueprintCallable, Category = Room)
	TArray<URoomSocket*> GetRoomSockets() const;

	UFUNCTION(BlueprintCallable, Category = Room)
	bool GetRoomSocket(FName Name, URoomSocket*& Socket) const;


	/*--------------------------------------------
	 	Utility
	 *--------------------------------------------*/

public:

	/** Snaps room with target room aligning sockets back to back. Can be stobbed by CanMove() Returns true on success.  */
	UFUNCTION(BlueprintCallable, Category = Room)
	bool SnapWithNode(FName Socket, ARoomNode* TargetNode, FName TargetSocket);


	/** Collects connected nodes and filters by class. Returns true if at leas one node has passed filter */
	UFUNCTION(BlueprintCallable, Category = Room, meta = (DisplayName = "GetConnectedNodes", DynamicOutputParam = Nodes, DeterminesOutputType = ClassFilter))
	bool BP_GetConnectedNodes(TArray<ARoomNode*>& Nodes, TSubclassOf<ARoomNode> ClassFilter) { return GetConnectedNodes(Nodes, ClassFilter); }
	bool GetConnectedNodes(TArray<ARoomNode*>& Nodes, TSubclassOf<ARoomNode> ClassFilter) const;

};

