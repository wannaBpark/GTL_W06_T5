#pragma once
#include "GameFramework/Character.h"

class AEnemyCharacter : public ACharacter
{

    DECLARE_CLASS(AEnemyCharacter, ACharacter)

public:
    AEnemyCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;


};

