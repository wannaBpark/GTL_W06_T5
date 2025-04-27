#pragma once
#include "SceneComponent.h"

class UCameraComponent : public USceneComponent
{
    DECLARE_CLASS(UCameraComponent, USceneComponent)
public:

    UCameraComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    void ProceedLerp(float DeltaTime);
    void LerpMovement(FVector& FromLocation, FVector& ToLocation, float DeltaTime);

    float ViewFOV = 90.0f;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;

    static std::shared_ptr<UCameraComponent> DefaultCamera; //아무 세팅 안된 기본카메라가 필요할때 쓰면 됨

private:

    FVector LerpMoveVector = FVector::ZeroVector;
    float LerpTime = 0.f;
};
