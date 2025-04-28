#pragma once
#include "Pawn.h"

class UInputComponent;
class AController;
class UStaticMeshComponent;
class UCameraComponent;
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

    // === 카메라 관련 ===
    UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
    UPROPERTY
    (USceneComponent*, RootScene, = nullptr);

    UPROPERTY
    (UStaticMeshComponent*, BodyMesh, = nullptr);

    //UPROPERTY
    //(USpringArmComponent*, CameraBoom, = nullptr);

    UPROPERTY
    (UCameraComponent*, FollowCamera, = nullptr);
};

