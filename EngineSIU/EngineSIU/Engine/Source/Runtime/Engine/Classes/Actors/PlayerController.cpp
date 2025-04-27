#include "PlayerController.h"

APlayerController::APlayerController()
{
    SetupInputComponent();
}

APlayerController::~APlayerController()
{
}

void APlayerController::BeginPlay()
{
}

void APlayerController::Tick(float DeltaTime)
{
    if (bHasPossessed) {
        ProcessInput(DeltaTime);
    }
}

void APlayerController::Destroyed()
{
}

void APlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

void APlayerController::Possess(AActor* InActor)
{
    CurrentPossess = InActor;
    bHasPossessed = true;
}

void APlayerController::UnPossess()
{
    CurrentPossess = nullptr;
    bHasPossessed = false;
}

void APlayerController::SetupInputComponent()
{
    // What is the correct parameter of ConstructObject?
    if (InputComponent == nullptr) {
        InputComponent = FObjectFactory::ConstructObject<UInputComponent>(this);
    }
}

void APlayerController::ProcessInput(float DeltaTime)
{
    if (InputComponent) InputComponent->ProcessInput(DeltaTime);
}
