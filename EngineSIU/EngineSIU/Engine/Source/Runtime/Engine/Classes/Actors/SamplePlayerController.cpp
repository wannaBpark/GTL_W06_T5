#include "SamplePlayerController.h"

ASamplePlayerController::ASamplePlayerController()
{
    SetupInputComponent();
}

ASamplePlayerController::~ASamplePlayerController()
{
}

void ASamplePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // W Key to forward
    GetInputComponent()->BindAction(
        EKeys::W,
        EInputEvent::IE_Repeat,
        [this]() { Move(0.1f, 0, 0); }
    );
    GetInputComponent()->BindAction(
        EKeys::A,
        EInputEvent::IE_Repeat,
        [this]() { Move(0, -0.1f, 0); }
    );
    GetInputComponent()->BindAction(
        EKeys::S,
        EInputEvent::IE_Repeat,
        [this]() { Move(-0.1f, 0, 0); }
    );
    GetInputComponent()->BindAction(
        EKeys::D,
        EInputEvent::IE_Repeat,
        [this]() { Move(0, 0.1f, 0); }
    );
}

void ASamplePlayerController::Move(float X, float Y, float Z)
{
    // Sample movement function
    if (CurrentPossess) {
        FVector Location = CurrentPossess->GetActorLocation();
        Location.X += X;
        Location.Y += Y;
        Location.Z += Z;
        CurrentPossess->SetActorLocation(Location);
    }
}
