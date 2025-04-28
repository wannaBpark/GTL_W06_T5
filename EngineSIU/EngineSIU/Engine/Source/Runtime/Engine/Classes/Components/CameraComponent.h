#pragma once
#include "SceneComponent.h"

class UCameraComponent : public USceneComponent
{
public:
    DECLARE_CLASS(UCameraComponent, USceneComponent)

    UCameraComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    void FollowMainPlayer();

    float ViewFOV = 90.0f;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;

    float DistanceBehind = 10.f;
    float CameraHeight = 20.f;
    float CameraZ = 0.f; //바닥에 닿을때마다 바닥 Z로 업데이트
    float CameraZOffset = 5.f; //너무 아래 보기때문에 조금 위를 향해서 보는 변수

    static std::shared_ptr<UCameraComponent> DefaultCamera; //아무 세팅 안된 기본카메라가 필요할때 쓰면 됨

    void SetLocationWithFInterpTo(FVector& ToLocation);
    void SetFInterpToSpeed(float InSpeed);

private:
    void ProceedFInterp(float DeltaTime);
    
    FVector FInterpTargetLocation = FVector::ZeroVector;
    float FInterpToSpeed = 0.8f;
};
