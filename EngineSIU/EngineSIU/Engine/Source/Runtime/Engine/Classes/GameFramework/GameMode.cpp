#include "GameMode.h"

AGameMode::AGameMode()
{
    OnGameInit.AddLambda([]() { UE_LOG(LogLevel::Display, TEXT("Game Initialized")); });
}

AGameMode::~AGameMode()
{
}

void AGameMode::InitGame()
{
    OnGameInit.Broadcast();
}

void AGameMode::StartMatch()
{
    OnGameStart.Broadcast();
}

void AGameMode::EndMatch()
{
    OnGameEnd.Broadcast();
}

void AGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);


}
