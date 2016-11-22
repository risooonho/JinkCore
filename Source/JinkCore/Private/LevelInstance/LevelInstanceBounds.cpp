// Copyright 2015-2017 Piperift. All Rights Reserved.

#include "JinkCorePrivatePCH.h"
#include "LevelInstanceBounds.h"
#include "Components/BoxComponent.h"

// Default size of the box (scale)
static const FVector DefaultLevelSize = FVector(1000.f);

ALevelInstanceBounds::ALevelInstanceBounds(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UBoxComponent* BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent0"));
	RootComponent = BoxComponent;
	RootComponent->Mobility = EComponentMobility::Movable;
	RootComponent->RelativeScale3D = DefaultLevelSize;

	bAutoUpdateBounds = true;

	BoxComponent->bDrawOnlyIfSelected = true;
	BoxComponent->bUseAttachParentBound = false;
	BoxComponent->bUseEditorCompositing = true;
	BoxComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	BoxComponent->InitBoxExtent(FVector(0.5f, 0.5f, 0.5f));

	bCanBeDamaged = false;

#if WITH_EDITOR
	bLevelBoundsDirty = true;
	bUsingDefaultBounds = false;
#endif
}

void ALevelInstanceBounds::PostLoad()
{
	Super::PostLoad();
	//LevelInstance->BoundsActor = this;
}

FBox ALevelInstanceBounds::GetComponentsBoundingBox(bool bNonColliding) const
{
	checkf(RootComponent != nullptr, TEXT("LevelInstanceBounds actor with null root component: %s"), *GetPathNameSafe(this));
	FVector BoundsCenter = RootComponent->GetComponentLocation();
	FVector BoundsExtent = RootComponent->ComponentToWorld.GetScale3D() * 0.5f;
	return FBox(BoundsCenter - BoundsExtent,
		BoundsCenter + BoundsExtent);
}

FBox ALevelInstanceBounds::CalculateLevelBounds(ULevel* InLevel)
{
	FBox LevelBounds = FBox(0);

	if (InLevel)
	{
		// Iterate over all level actors
		for (int32 ActorIndex = 0; ActorIndex < InLevel->Actors.Num(); ++ActorIndex)
		{
			AActor* Actor = InLevel->Actors[ActorIndex];
			if (Actor && Actor->IsLevelBoundsRelevant())
			{
				// Sum up components bounding boxes
				FBox ActorBox = Actor->GetComponentsBoundingBox(true);
				if (ActorBox.IsValid)
				{
					LevelBounds += ActorBox;
				}
			}
		}
	}

	return LevelBounds;
}

#if WITH_EDITOR
void ALevelInstanceBounds::PostEditUndo()
{
	Super::PostEditUndo();

	MarkLevelBoundsDirty();
}

void ALevelInstanceBounds::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	MarkLevelBoundsDirty();
}

void ALevelInstanceBounds::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	MarkLevelBoundsDirty();
}

void ALevelInstanceBounds::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	if (!IsTemplate())
	{
		//LevelInstance->BoundsActor = this;
		SubscribeToUpdateEvents();
	}
}

void ALevelInstanceBounds::PostUnregisterAllComponents()
{
	if (!IsTemplate())
	{
		UnsubscribeFromUpdateEvents();
	}

	Super::PostUnregisterAllComponents();
}

void ALevelInstanceBounds::Tick(float DeltaTime)
{
	if (bLevelBoundsDirty)
	{
		UpdateLevelBounds();
		bLevelBoundsDirty = false;
	}
}

TStatId ALevelInstanceBounds::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ALevelInstanceBounds, STATGROUP_Tickables);
}

bool ALevelInstanceBounds::IsTickable() const
{
	if (GIsEditor && bAutoUpdateBounds && !IsTemplate())
	{
		UWorld* World = GetWorld();
		return (World && World->WorldType == EWorldType::Editor);
	}

	return false;
}

bool ALevelInstanceBounds::IsTickableInEditor() const
{
	return IsTickable();
}

void ALevelInstanceBounds::UpdateLevelBounds()
{
	FBox LevelBounds = CalculateLevelBounds(GetLevel());

    if (!LevelInstance.IsNull()) {
        ULevelInstance* LevelI = LevelInstance.LoadSynchronous();
        LevelI->Bounds = LevelBounds;
        LevelI->MarkPackageDirty();
    }

	if (LevelBounds.IsValid)
	{
		FVector LevelCenter = LevelBounds.GetCenter();
		FVector LevelSize = LevelBounds.GetSize();

		SetActorTransform(FTransform(FQuat::Identity, LevelCenter, LevelSize));
		bUsingDefaultBounds = false;
	}
	else
	{
		SetActorTransform(FTransform(FQuat::Identity, FVector::ZeroVector, DefaultLevelSize));
		bUsingDefaultBounds = true;
	}

	//BroadcastLevelBoundsUpdated();
}

void ALevelInstanceBounds::MarkLevelBoundsDirty()
{
	bLevelBoundsDirty = true;
}

bool ALevelInstanceBounds::IsUsingDefaultBounds() const
{
	return bUsingDefaultBounds;
}

void ALevelInstanceBounds::UpdateLevelBoundsImmediately()
{
	// This is used to get accurate bounds right when spawned.
	// This can't be done in PostActorCreated because the SpawnLocation interferes with the root component transform
	UpdateLevelBounds();
}

void ALevelInstanceBounds::OnLevelActorMoved(AActor* InActor)
{
	if (InActor->GetOuter() == GetOuter())
	{
		if (InActor == this)
		{
			//BroadcastLevelBoundsUpdated();
		}
		else
		{
			MarkLevelBoundsDirty();
		}
	}
}

void ALevelInstanceBounds::OnLevelActorAddedRemoved(AActor* InActor)
{
	if (InActor->GetOuter() == GetOuter())
	{
		MarkLevelBoundsDirty();
	}
}

void ALevelInstanceBounds::SubscribeToUpdateEvents()
{
	// Subscribe only in editor worlds
	if (!GetWorld()->IsGameWorld())
	{
		UnsubscribeFromUpdateEvents();

		OnLevelActorMovedDelegateHandle = GEngine->OnActorMoved().AddUObject(this, &ALevelInstanceBounds::OnLevelActorMoved);
		OnLevelActorDeletedDelegateHandle = GEngine->OnLevelActorDeleted().AddUObject(this, &ALevelInstanceBounds::OnLevelActorAddedRemoved);
		OnLevelActorAddedDelegateHandle = GEngine->OnLevelActorAdded().AddUObject(this, &ALevelInstanceBounds::OnLevelActorAddedRemoved);
	}
}

void ALevelInstanceBounds::UnsubscribeFromUpdateEvents()
{
	GEngine->OnActorMoved().Remove(OnLevelActorMovedDelegateHandle);
	GEngine->OnLevelActorDeleted().Remove(OnLevelActorDeletedDelegateHandle);
	GEngine->OnLevelActorAdded().Remove(OnLevelActorAddedDelegateHandle);
}


#endif // WITH_EDITOR