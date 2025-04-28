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
}

void UCameraComponent::TickComponent(float DeltaTime)
{
    USceneComponent::TickComponent(DeltaTime);

    ProceedFInterp(DeltaTime);
}


void UCameraComponent::ProceedFInterp(float DeltaTime)
{
    FVector FromLocation = GetWorldLocation();
  
    FVector MoveLocation = FMath::FInterpTo(FromLocation, FInterpTargetLocation, DeltaTime, FInterpToSpeed);
    SetWorldLocation(MoveLocation);
}

void UCameraComponent::SetLocationWithFInterpTo(FVector& ToLocation) //LerpSpeed = 0은 안움직이고 1은 바로이동
{
    FInterpTargetLocation = ToLocation;
}

void UCameraComponent::SetFInterpToSpeed(float InSpeed)
{
    FInterpToSpeed = InSpeed;
}
