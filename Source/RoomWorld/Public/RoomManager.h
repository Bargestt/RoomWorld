// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomManager.generated.h"

UCLASS()
class ROOMWORLD_API ARoomManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<ARoomNode*> AllNodes;
	
public:	
	ARoomManager();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PreInitializeComponents() override;
protected:
	virtual void BeginPlay() override;

public:
#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif //WITH_EDITOR
	


public:
	virtual void RegisterNode(ARoomNode* Node);

	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveRegisterNode(ARoomNode* Node);
	

	/*--------------------------------------------
	 	Utility
	 *--------------------------------------------*/
public:
	UFUNCTION(BlueprintCallable, Category = "Room World", meta = (DeterminesOutputType = "NodeClass", DynamicOutputParam = "OutNodes"))
	void GetAllNodesOfClass(TSubclassOf<ARoomNode> NodeClass, TArray<ARoomNode*>& OutNodes);

};
