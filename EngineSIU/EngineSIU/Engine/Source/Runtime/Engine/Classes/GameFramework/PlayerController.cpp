#include "PlayerController.h"
#include "Camera/CameraComponent.h"
#include "PlayerInput.h"

void APlayerController::BeginPlay()
{
    Super::BeginPlay();
}

void APlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    SetupInputBindings();
}

void APlayerController::AddYawInput(float Value)
{
}

void APlayerController::AddPitchInput(float Value)
{
}

bool APlayerController::IsInputKeyDown(EKeys::Type Key) const
{
    return false;
}

void APlayerController::SetupInputBindings()
{
    // Setup input bindings for player controller
}
