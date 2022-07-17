


#include "Components/RoomStateIconComponent.h"
#include "Room.h"

URoomStateIconComponent::URoomStateIconComponent()
{
	Sprite  = ConstructorHelpers::FObjectFinder<UTexture2D>(TEXT("Texture2D'/RoomWorld/Icons/Room.Room'")).Object;
	State_Unloaded = ConstructorHelpers::FObjectFinder<UTexture2D>(TEXT("Texture2D'/RoomWorld/Icons/Room_Unloaded.Room_Unloaded'")).Object;
	State_Loaded = ConstructorHelpers::FObjectFinder<UTexture2D>(TEXT("Texture2D'/RoomWorld/Icons/Room_Loaded.Room_Loaded'")).Object;
	State_Visible = ConstructorHelpers::FObjectFinder<UTexture2D>(TEXT("Texture2D'/RoomWorld/Icons/Room_Visible.Room_Visible'")).Object;
}

void URoomStateIconComponent::BeginPlay()
{
	Super::BeginPlay();
	
	ARoom* Room = GetOwner<ARoom>();
	if (IsValid(Room))
	{
		Room->OnStateChanged.AddDynamic(this, &ThisClass::StateChenged);
	}
}

void URoomStateIconComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ARoom* Room = GetOwner<ARoom>();
	if (IsValid(Room))
	{
		Room->OnStateChanged.RemoveAll(this);
	}
}

void URoomStateIconComponent::StateChenged(ARoom* Room, ERoomState NewState)
{
	switch (NewState)
	{
	case ERoomState::Unloaded:
		SetSprite(State_Unloaded);
		break;
	case ERoomState::Loaded:
		SetSprite(State_Loaded);
		break;
	case ERoomState::Visible:
		SetSprite(State_Visible);
		break;
	default:
		unimplemented();
		break;
	}
}
