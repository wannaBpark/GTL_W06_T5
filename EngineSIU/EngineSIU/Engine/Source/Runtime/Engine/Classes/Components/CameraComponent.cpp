#include "CameraComponent.h"

#include <algorithm>

#include "Engine/Engine.h"
#include "UObject/Casts.h"
#include "World/World.h"

std::shared_ptr<UCameraComponent> UCameraComponent::DefaultCamera = std::make_shared<UCameraComponent>();

UObject* UCameraComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->ViewFOV = ViewFOV;
    NewComponent->NearClip = NearClip;
    NewComponent->FarClip = FarClip;
    
    return NewComponent;
}

void UCameraComponent::InitializeComponent()
{
    USceneComponent::InitializeComponent();

    FVector FL = GetWorldLocation();
    FVector TL = FL + GetForwardVector() * 10;
    
    LerpMovement(FL, TL, 0.8);
}

void UCameraComponent::TickComponent(float DeltaTime)
{
    USceneComponent::TickComponent(DeltaTime);
}

