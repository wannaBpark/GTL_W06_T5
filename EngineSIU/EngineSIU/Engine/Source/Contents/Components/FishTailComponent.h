#pragma once
#include "Components/StaticMeshComponent.h"

class UFishTailComponent : public UStaticMeshComponent
{
    DECLARE_CLASS(UFishTailComponent, UStaticMeshComponent)

public:
    UFishTailComponent();
    virtual ~UFishTailComponent() override = default;

    virtual void TickComponent(float DeltaTime) override;

    virtual void InitializeComponent() override;

    void SetFrequency(float InFrequency) { Frequency = InFrequency; }
    float GetFrequency() const { return Frequency; }

protected:
    float Frequency = 10.f;

    float ElapsedTime = 0.f;

    float MaxYaw = 20.f;
};
