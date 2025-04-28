#pragma once
#include "Actor.h"
#include "Delegates/DelegateCombination.h"

DECLARE_MULTICAST_DELEGATE(FOnGameInit);
DECLARE_MULTICAST_DELEGATE(FOnGameStart);
DECLARE_MULTICAST_DELEGATE(FOnGameEnd);


class AGameMode : public AActor
{
    DECLARE_CLASS(AGameMode, AActor)
public:
    AGameMode();
    virtual ~AGameMode();

    // 게임 모드 초기화
    virtual void InitGame();

    // 게임 시작
    virtual void StartMatch();

    // 게임 종료
    virtual void EndMatch();

    virtual void Tick(float DeltaTime) override;


public:
    FOnGameInit OnGameInit;
    FOnGameStart OnGameStart;
    FOnGameEnd OnGameEnd;
};
