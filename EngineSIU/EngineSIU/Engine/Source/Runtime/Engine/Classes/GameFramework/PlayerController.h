#pragma once
#include "Controller.h"
#include "InputCore/InputCoreTypes.h"

class UCameraComponent;
class UPlayerInput;
class APlayerController : public AController
{
    DECLARE_CLASS(APlayerController, AController)

public:
    APlayerController() = default;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    virtual void SetupInputComponent() override;
    // === 입력 기능 ===
    void AddYawInput(float Value);
    void AddPitchInput(float Value);
    bool IsInputKeyDown(EKeys::Type Key) const;

    UPROPERTY
    (UCameraComponent*, PlayerCameraManager, = nullptr)

protected:
    void SetupInputBindings();

    UPROPERTY
    (UPlayerInput*, PlayerInput, = nullptr)


};

