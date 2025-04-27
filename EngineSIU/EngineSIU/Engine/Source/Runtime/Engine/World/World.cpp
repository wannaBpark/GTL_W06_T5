#include "World.h"

#include "Actors/Cube.h"
#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Components/SkySphereComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Actors/HeightFogActor.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "UnrealEd/SceneManager.h"
#include "Actors/PointLightActor.h"
#include "Actors/SpotLightActor.h"


class UEditorEngine;

UWorld* UWorld::CreateWorld(UObject* InOuter, const EWorldType InWorldType, const FString& InWorldName)
{
    UWorld* NewWorld = FObjectFactory::ConstructObject<UWorld>(InOuter);
    NewWorld->WorldName = InWorldName;
    NewWorld->WorldType = InWorldType;
    NewWorld->InitializeNewWorld();

    return NewWorld;
}

void UWorld::InitializeNewWorld()
{
    ActiveLevel = FObjectFactory::ConstructObject<ULevel>(this);
    ActiveLevel->InitLevel(this);
}

void UWorld::InitializeLightScene()
{
    // (생략 가능: 여기는 LightScene 생성하는 테스트 코드)
}

UObject* UWorld::Duplicate(UObject* InOuter)
{
    UWorld* NewWorld = Cast<UWorld>(Super::Duplicate(InOuter));
    NewWorld->ActiveLevel = Cast<ULevel>(ActiveLevel->Duplicate(NewWorld));
    NewWorld->ActiveLevel->InitLevel(NewWorld);
    return NewWorld;
}

void UWorld::Tick(float DeltaTime)
{
    if (WorldType != EWorldType::Editor)
    {
        for (AActor* Actor : PendingBeginPlayActors)
        {
            Actor->BeginPlay();
        }
        PendingBeginPlayActors.Empty();
    }

    for (AActor* Actor : GetActiveLevel()->Actors)
    {
        Actor->UpdateOverlaps();
    }

    TArray<AActor*> ActorsCopy = GetActiveLevel()->Actors;

    for (AActor* Actor : ActorsCopy)
    {
        for (AActor* Other : ActorsCopy)
        {
            if (!Other || Other->IsActorBeingDestroyed())
            {
                continue;
            }

            if (Actor != Other)
            {
                if (Actor->IsOverlappingActor(Other))
                {
                    Actor->OnActorOverlap.Broadcast(Other);
                    Other->OnActorOverlap.Broadcast(Actor);
                }
            }
        }
    }
}

void UWorld::BeginPlay()  
{
    for (AActor* Actor : ActiveLevel->Actors)
    {
        if (Actor->GetWorld() == this)
        {
            Actor->BeginPlay();
            if (PendingBeginPlayActors.Contains(Actor))
            {
                PendingBeginPlayActors.Remove(Actor);
            }
        }
    }

}

void UWorld::Release()
{
    if (ActiveLevel)
    {
        ActiveLevel->Release();
        GUObjectArray.MarkRemoveObject(ActiveLevel);
        ActiveLevel = nullptr;
    }

    GUObjectArray.ProcessPendingDestroyObjects();
}

AActor* UWorld::SpawnActor(UClass* InClass, FName InActorName)
{
    if (!InClass)
    {
        UE_LOG(LogLevel::Error, TEXT("SpawnActor failed: ActorClass is null."));
        return nullptr;
    }

    if (InClass->IsChildOf<AActor>())
    {
        AActor* NewActor = Cast<AActor>(FObjectFactory::ConstructObject(InClass, this, InActorName));
        ActiveLevel->Actors.Add(NewActor);
        PendingBeginPlayActors.Add(NewActor);
        return NewActor;
    }

    UE_LOG(LogLevel::Error, TEXT("SpawnActor failed: Class '%s' is not derived from AActor."), *InClass->GetName());
    return nullptr;
}

bool UWorld::DestroyActor(AActor* ThisActor)
{
    if (ThisActor->GetWorld() == nullptr)
    {
        return false;
    }

    if (ThisActor->IsActorBeingDestroyed())
    {
        return true;
    }


    UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);

    if (EditorEngine->GetSelectedActor() == ThisActor)
    {
        EditorEngine->DeselectActor(ThisActor);
    }
    if (EditorEngine->GetSelectedComponent() && 
        
        
        
        
        
        ThisActor->GetComponentByFName<UActorComponent>(EditorEngine->GetSelectedComponent()->GetFName()))
    {
        EditorEngine->DeselectComponent(EditorEngine->GetSelectedComponent());
    }

    ThisActor->Destroyed();

  

    if (ThisActor->GetOwner())
    {
        ThisActor->SetOwner(nullptr);
    }

    ActiveLevel->Actors.Remove(ThisActor);

    GUObjectArray.MarkRemoveObject(ThisActor);
    return true;
}

UWorld* UWorld::GetWorld() const
{
    return const_cast<UWorld*>(this);
}
