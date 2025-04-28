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

protected:
    UPROPERTY
    (USceneComponent*, RootScene, = nullptr);

    UPROPERTY
    (UStaticMeshComponent*, BodyMesh, = nullptr);
};

