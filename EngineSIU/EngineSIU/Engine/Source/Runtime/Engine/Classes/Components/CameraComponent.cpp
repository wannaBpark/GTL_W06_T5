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

    SetFInterpToSpeed(0.8f);
}

void UCameraComponent::TickComponent(float DeltaTime)
{
    USceneComponent::TickComponent(DeltaTime);

    FollowMainPlayer();
    
    ProceedFInterp(DeltaTime);
}

void UCameraComponent::FollowMainPlayer()
{
    FVector PlayerLocation = GEngine->ActiveWorld->GetMainPlayer()->GetActorLocation();
    
    FVector PlayerBackward = -GEngine->ActiveWorld->GetMainPlayer()->GetActorForwardVector();

    FVector CameraOffset = PlayerBackward * DistanceBehind + FVector(0, 0, CameraHeight);
    
    FVector MoveToLocation = FVector(PlayerLocation.X, PlayerLocation.Y, CameraZ) + CameraOffset;
    
    SetLocationWithFInterpTo(MoveToLocation);
}

void UCameraComponent::ProceedFInterp(float DeltaTime)
{
    FVector FromLocation = GetWorldLocation();

    //카메라 위치
    FVector MoveLocation = FMath::FInterpTo(FromLocation, FInterpTargetLocation, DeltaTime, FInterpToSpeed);

    FVector PlayerLocation = GEngine->ActiveWorld->GetMainPlayer()->GetActorLocation();
    PlayerLocation.Z = CameraZ + CameraZOffset;
    
    FRotator TargetRotation = FRotator::MakeLookAtRotation(MoveLocation, PlayerLocation);
    
    SetWorldLocation(MoveLocation);
    SetWorldRotation(TargetRotation);
}

void UCameraComponent::SetLocationWithFInterpTo(FVector& ToLocation) //LerpSpeed = 0은 안움직이고 1은 바로이동
{
    FInterpTargetLocation = ToLocation;
}

void UCameraComponent::SetFInterpToSpeed(float InSpeed)
{
    FInterpToSpeed = InSpeed;
}
