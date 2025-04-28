#include "PlayerController.h"

#include "UObject/UObjectIterator.h"

APlayerController::APlayerController()
{
    RootComponent = AddComponent<USceneComponent>();
    
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
    AActor::Tick(DeltaTime);
    
    if (bHasPossessed)
    {
        ProcessInput(DeltaTime);
    }
}

void APlayerController::ProcessInput(float DeltaTime) const
{
    if (InputComponent)
    {
        InputComponent->ProcessInput(DeltaTime);
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

    if (InputComponent)
    {
        InputComponent->SetPossess();
    }
}

void APlayerController::UnPossess()
{
    CurrentPossess = nullptr;
    bHasPossessed = false;

    if (InputComponent)
    {
        InputComponent->UnPossess();
    }
}

void APlayerController::SetupInputComponent()
{
    // What is the correct parameter of ConstructObject?
    if (InputComponent == nullptr) {
        InputComponent = FObjectFactory::ConstructObject<UInputComponent>(this);
    }
}

void APlayerController::BindAction(const FString& Key, const std::function<void(float)>& Callback)
{
    if (bHasPossessed && InputComponent)
    {
        InputComponent->BindAction(Key, Callback);
    }
}
