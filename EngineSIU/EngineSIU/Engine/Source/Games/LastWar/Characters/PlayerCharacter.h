#pragma once
#include "GameFramework/Character.h"

class UCameraComponent;
class UInputComponent;
class APlayerCharacter : public ACharacter
{
    DECLARE_CLASS(APlayerCharacter, ACharacter)

public:
    APlayerCharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    // === 이동 관련 ===
    void MoveForward(float Value);
    void MoveRight(float Value);

    // === 카메라 관련 ===
    UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
    UPROPERTY
    (UCameraComponent*, FollowCamera, = nullptr);
};

