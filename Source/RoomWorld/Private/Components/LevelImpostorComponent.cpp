


#include "Components/LevelImpostorComponent.h"


ALevelImpostorActor::ALevelImpostorActor()
{
	Component = CreateDefaultSubobject<ULevelImpostorComponent>(TEXT("Comp"));
	RootComponent = Component;

}

void ALevelImpostorActor::Update()
{
	if (Component)
	{
		Component->Update(LevelPath);
	}
}


ULevelImpostorComponent::ULevelImpostorComponent()
{
	MinVolume = 0.1f;
	MaxNum = 0;

	bCollidingOnly = true;
	bVisibleOnly = true;
	
	SetStaticMesh(ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/EngineMeshes/Cube.Cube'")).Object);

	IgnoreActors.Add(ALevelImpostorActor::StaticClass());
}

void ULevelImpostorComponent::Update(FSoftObjectPath LevelPath)
{
	UWorld* Asset = Cast<UWorld>(LevelPath.TryLoad());

	ClearInstances();

	if (!Asset || !Asset->PersistentLevel)
	{
		return;
	}
	
	if (!GetStaticMesh())
	{
		return;
	}
	FVector MeshExtent = GetStaticMesh()->GetBounds().BoxExtent;

	typedef TTuple<FTransform, FVector> BoxData;
	TArray<BoxData> AllBoxes;

	for (AActor* Actor : Asset->PersistentLevel->Actors)
	{
		if (!Actor)
		{
			continue;
		}

		if (bVisibleOnly && Actor->IsHidden())
		{
			continue;
		}

		bool bIgnoreActor = false;
		for (auto& IgnoreClass : IgnoreActors)
		{
			if (IgnoreClass && Actor->IsA(IgnoreClass))
			{
				bIgnoreActor = true;
				break;
			}
		}

		if (bIgnoreActor)
		{
			continue;
		}

		if (bDetailed)
		{
			Actor->ForEachComponent<UPrimitiveComponent>(false, [&](const UPrimitiveComponent* InPrimComp)
			{				
				if (InPrimComp->IsRegistered() && (!bCollidingOnly || InPrimComp->IsCollisionEnabled() && (!bVisibleOnly || InPrimComp->IsVisible())))
				{
					FQuat Rotation = InPrimComp->GetComponentToWorld().GetRotation();
					FVector Center = InPrimComp->GetComponentToWorld().GetLocation();
					FVector Extent = InPrimComp->CalcLocalBounds().BoxExtent;

					AllBoxes.Add(MakeTuple(FTransform(Rotation, Center), Extent));
				}
			});
		}
		else
		{			
			FBox Box(ForceInit);

			Actor->ForEachComponent<UPrimitiveComponent>(false, [&](const UPrimitiveComponent* InPrimComp)
			{				
				if (InPrimComp->IsRegistered() && (!bCollidingOnly || InPrimComp->IsCollisionEnabled() && (!bVisibleOnly || InPrimComp->IsVisible())))
				{
					Box += InPrimComp->Bounds.GetBox();
				}
			});

			FVector Center;
			FVector Extent; 
			Box.GetCenterAndExtents(Center, Extent);
			
			AllBoxes.Add( MakeTuple(FTransform(FQuat::Identity, Center), Extent) );
		}
	}	

	if (MinVolume > 0)
	{	
		float Volume = MinVolume * 1000000.0f;
		AllBoxes.RemoveAll([Volume](const BoxData& Entry)
		{ 
			const FVector& Extent = Entry.Get<1>();
			return (Extent.X * Extent.Y * Extent.Z) < Volume;
		});
	}

	if (MaxVolume > 0)
	{
		float Volume = MaxVolume * 1000000.0f;
		AllBoxes.RemoveAll([Volume](const BoxData& Entry)
		{ 
			const FVector& Extent = Entry.Get<1>();
			return (Extent.X * Extent.Y * Extent.Z) > Volume;
		});
	}

	if (MaxNum > 0)
	{
		AllBoxes.Sort([](const BoxData& A, const BoxData& B)
		{ 
			const FVector& ExtentA = A.Get<1>();
			const FVector& ExtentB = B.Get<1>();
			return (ExtentA.X * ExtentA.Y * ExtentA.Z) > (ExtentB.X * ExtentB.Y * ExtentB.Z);
		});

		AllBoxes.SetNum(FMath::Min(AllBoxes.Num(), MaxNum), true);
	}
	

	TArray<FTransform> SelectedBoxes;
	for (const BoxData& Entry : AllBoxes)
	{
		FVector Scale = Entry.Get<1>() / MeshExtent;		

		if (!Scale.IsNearlyZero())
		{
			FTransform Tr = Entry.Get<0>();
			Tr.SetScale3D(Scale);

			SelectedBoxes.Add(Tr);
		}		
	}
	AddInstances(SelectedBoxes, false);
}
