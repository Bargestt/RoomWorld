

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomSocketHandle.generated.h"

class URoomSocket;


/** 
 * Base editor only class for operating with sockets in the level viewport 
 * Removed from cooked builds
 */
UCLASS(Blueprintable, ClassGroup = (RoomWorld), hideCategories = (Transform,Replication,Collision,Input,LOD,Cooking))
class ROOMWORLD_API ARoomSocketHandle : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = Socket)
	FString Class;

	UPROPERTY(VisibleAnywhere, Instanced, Category = Socket)
	URoomSocket* Socket;	

	ARoomSocketHandle();

#if WITH_EDITOR
	virtual void PostLoad() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool ShouldExport() override { return true; }
	virtual bool EditorCanAttachTo(const AActor* InParent, FText& OutReason) const override;
	virtual bool CanDeleteSelectedActor(FText& OutReason) const;
	void ConnectionMoved(AActor* Actor);	
#endif // WITH_EDITOR

	virtual void SetSocket(URoomSocket* Socket);


	/** Called in editor when linked connection is moved */
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveConnectionMoved(int32 Index);

	/** Called when connection related to this actor is created  */
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveConnectionGained(URoomSocket* NewConnection, int32 Index);

	/** Called when connection related to this actor is destroyed  */
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveConnectionLost(URoomSocket* OldConnection, int32 Index);


public:


};


/*--------------------------------------------
	Promoted functions from URoomSocket
 *--------------------------------------------*/

#define SOCKET_FUNC_RetVal(FuncName, ...) \
	{ \
		if(URoomSocket* NodeSocket = GetSocket(NodeOwner)) \
		{ \
			return NodeSocket->FuncName(__VA_ARGS__); \
		} \
		return {}; \
	}


UCLASS(meta = (RestrictedToClasses = "RoomSocketHandle"))
class ROOMWORLD_API URoomSocketHandleFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "RoomWorld|Socket|Handle", meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static URoomSocket* GetSocket(ARoomSocketHandle* NodeOwner)
	{
		return NodeOwner ? NodeOwner->Socket : nullptr;
	}

	UFUNCTION(BlueprintPure, Category = "RoomWorld|Socket|Handle")
	static ARoomSocketHandle* GetSocketHandle(URoomSocket* Socket)
	{
		return Socket ? Socket->Handle : nullptr;
	}



	UFUNCTION(BlueprintPure, Category = "RoomWorld|Socket|Handle", meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static ARoomNode* GetNode(ARoomSocketHandle* NodeOwner)
	SOCKET_FUNC_RetVal(GetNode);

	UFUNCTION(BlueprintPure, Category = "RoomWorld|Socket|Handle", meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static ARoomNode* GetConnectedNode(ARoomSocketHandle* NodeOwner, int32 Index)
	SOCKET_FUNC_RetVal(GetNode);

	UFUNCTION(BlueprintPure, Category = "RoomWorld|Socket|Handle", meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static TArray<ARoomNode*> GetConnectedNodes(ARoomSocketHandle* NodeOwner)
	SOCKET_FUNC_RetVal(GetConnectedNodes);



	UFUNCTION(BlueprintPure, Category = "RoomWorld|Socket|Handle", meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static TArray<URoomSocket*> GetConnections(ARoomSocketHandle* NodeOwner)
	SOCKET_FUNC_RetVal(GetConnections);

	UFUNCTION(BlueprintPure, Category = "RoomWorld|Socket|Handle", meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static int32 GetNumConnections(ARoomSocketHandle* NodeOwner)
	SOCKET_FUNC_RetVal(GetNumConnections);

	UFUNCTION(BlueprintPure, Category = "RoomWorld|Socket|Handle", meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static bool HasConnections(ARoomSocketHandle* NodeOwner)
	SOCKET_FUNC_RetVal(HasConnections);

	UFUNCTION(BlueprintPure, Category = "RoomWorld|Socket|Handle", meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static URoomSocket* GetConnection(ARoomSocketHandle* NodeOwner, int32 Index)
	SOCKET_FUNC_RetVal(GetConnection, Index);

	UFUNCTION(BlueprintPure, Category = "RoomWorld|Socket|Handle", meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static bool IsConnectedTo(ARoomSocketHandle* NodeOwner, URoomSocket* Socket)
	SOCKET_FUNC_RetVal(IsConnectedTo, Socket);
};

#undef SOCKET_FUNC_RetVal
