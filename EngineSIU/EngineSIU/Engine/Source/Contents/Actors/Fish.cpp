
#include "Fish.h"

#include "Components/SphereComponent.h"
#include "Contents/Components/FishTailComponent.h"
#include "Engine/FObjLoader.h"

AFish::AFish()
{
    SphereComponent = AddComponent<USphereComponent>();
    SetRootComponent(SphereComponent);

    FishBody = AddComponent<UStaticMeshComponent>();
    FishBody->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Fish/Fish_Front.obj"));
    FishBody->SetRelativeLocation(FVector(0.f, -0.5f, 0.f));
    FishBody->SetupAttachment(SphereComponent);
    
    FishTail = AddComponent<UFishTailComponent>();
    FishTail->SetupAttachment(FishBody);
}

UObject* AFish::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    return NewActor;
}
