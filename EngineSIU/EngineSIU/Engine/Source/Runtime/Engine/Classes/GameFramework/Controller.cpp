#include "Controller.h"
#include "Pawn.h"
#include "Components/InputComponent.h"

void AController::BeginPlay()
{
    Super::BeginPlay();
}

void AController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AController::Destroyed()
{
    Super::Destroyed();
}

void AController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void AController::Possess(APawn* InPawn)
{
}

void AController::UnPossess()
{
}

APawn* AController::GetPawn() const
{
    return nullptr;
}

void AController::SetPawn(APawn* InPawn)
{
}

void AController::SetupInputComponent()
{
}

FRotator AController::GetControlRotation() const
{
    return FRotator();
}

void AController::SetControlRotation(const FRotator& NewRotation)
{
}
