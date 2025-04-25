#include "Pawn.h"
#include "Controller.h"
#include "Components/InputComponent.h"

void APawn::BeginPlay()
{
    Super::BeginPlay();
}

void APawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APawn::Destroyed()
{
    Super::Destroyed();
}

void APawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void APawn::PossessedBy(AController* NewController)
{
}

void APawn::UnPossessed()
{
}

void APawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
}

void APawn::AddMovementInput(FVector WorldDirection, float ScaleValue)
{
}

void APawn::AddControllerYawInput(float Value)
{
}

void APawn::AddControllerPitchInput(float Value)
{
}

FVector APawn::GetPawnViewLocation() const
{
    return FVector();
}

FRotator APawn::GetViewRotation() const
{
    return FRotator();
}
