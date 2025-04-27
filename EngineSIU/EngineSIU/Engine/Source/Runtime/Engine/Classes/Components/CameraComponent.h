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

    float ViewFOV = 90.0f;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;

    static std::shared_ptr<UCameraComponent> DefaultCamera;
};
