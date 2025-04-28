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

    ProceedLerp(DeltaTime);
}


void UCameraComponent::ProceedLerp(float DeltaTime)
{
    if (LerpDeltaVector > 0 ) // Lerp중
    {
        FVector FromLocation = GetWorldLocation();
        FVector ToLocation = FromLocation + LerpDeltaVector;

        FVector MoveLocation = FMath::FInterpTo(FromLocation, ToLocation, DeltaTime, LerpSpeed);
        SetWorldLocation(MoveLocation);

        //움직인만큼 빼주기
        LerpDeltaVector -= MoveLocation - FromLocation;
        LerpDeltaVector = FMath::Max(LerpDeltaVector, FVector::ZeroVector);
    }
}

void UCameraComponent::LerpMovement(FVector& FromLocation, FVector& ToLocation, float InLerpSpeed) //LerpSpeed = 0은 안움직이고 1은 바로이동
{
    LerpDeltaVector = ToLocation - FromLocation;

    LerpSpeed = InLerpSpeed;
}
