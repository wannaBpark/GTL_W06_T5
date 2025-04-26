#pragma once
#include "Pawn.h"

class UInputComponent;
class AController;
class UStaticMeshComponent;
class ACharacter : public APawn
{
    DECLARE_CLASS(ACharacter, APawn)

public:
    ACharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;

    // === 이동 관련 ===
    void MoveForward(float Value);
    void MoveRight(float Value);
    void MoveLeft(float Value);
    void MoveBackward(float Value);

private:
    UPROPERTY
    (UStaticMeshComponent*, StaticMeshComponent, = nullptr);
};

