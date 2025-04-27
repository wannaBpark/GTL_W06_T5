#pragma once
#include "Classes/Actors/PlayerController.h"

class ASamplePlayerController : public APlayerController
{
    DECLARE_CLASS(ASamplePlayerController, APlayerController)
public:
    ASamplePlayerController();
    ~ASamplePlayerController();

protected:
    virtual void SetupInputComponent() override;

private:
    void Move(float X, float Y, float Z);
};
