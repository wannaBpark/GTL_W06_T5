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

    FVector FromL = GetRelativeLocation();
    FVector ToL = FromL + FVector(0, 0, 100);
    
    LerpMovement(FromL, ToL, 10);
}

void UCameraComponent::TickComponent(float DeltaTime)
{
    USceneComponent::TickComponent(DeltaTime);

    ProceedLerp(DeltaTime);
}

void UCameraComponent::ProceedLerp(float DeltaTime)
{
    if (LerpTime > 0) // Lerp중
    {
        float LerpDeltaTime = DeltaTime;

        //LerpTime이 0이 아닌 음수까지 가면 더 많이 움직이기 때문에 예외처리
        LerpDeltaTime = std::min(LerpTime, LerpDeltaTime);

        FVector DeltaLocation = LerpMoveVector / LerpTime * LerpDeltaTime;

        FVector CameraLocation = GetRelativeLocation();
        CameraLocation += DeltaLocation;
        SetRelativeLocation(CameraLocation);

        LerpMoveVector -= DeltaLocation;
        LerpTime -= LerpDeltaTime;
    }
}

void UCameraComponent::LerpMovement(FVector& FromLocation, FVector& ToLocation, float Time)
{
    LerpTime = Time;
    LerpMoveVector = ToLocation - FromLocation;
}
