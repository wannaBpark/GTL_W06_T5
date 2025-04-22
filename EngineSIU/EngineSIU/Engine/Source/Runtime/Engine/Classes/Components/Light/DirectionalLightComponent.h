#pragma once
#include "LightComponent.h"
#include "UObject/ObjectMacros.h"

class UDirectionalLightComponent : public ULightComponentBase
{
    DECLARE_CLASS(UDirectionalLightComponent, ULightComponentBase)

public:
    UDirectionalLightComponent();
    virtual ~UDirectionalLightComponent() override;

    virtual UObject* Duplicate(UObject* InOuter) override;
    
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    FVector GetDirection();
    float GetShadowNearPlane() const;

    const FDirectionalLightInfo& GetDirectionalLightInfo() const;
    void SetDirectionalLightInfo(const FDirectionalLightInfo& InDirectionalLightInfo);

    float GetIntensity() const;
    void SetIntensity(float InIntensity);

    FLinearColor GetLightColor() const;
    void SetLightColor(const FLinearColor& InColor);

    void UpdateViewMatrix(FVector TargetPosition);
    void UpdateViewMatrix() override;
    void UpdateProjectionMatrix() override;
    float GetShadowFrustumWidth() const;

private:
    FDirectionalLightInfo DirectionalLightInfo;

    // --- 직교 투영 파라미터 ---
    // 직교 투영 볼륨의 월드 단위 너비 (섀도우 영역)
    float OrthoWidth = 100.0f;

    // 직교 투영 볼륨의 월드 단위 높이 (섀도우 영역)
    float OrthoHeight = 100.0f;

    // 섀도우 계산을 위한 라이트 시점의 Near Plane (음수 가능)
    float ShadowNearPlane = 1.0f;

    // 섀도우 계산을 위한 라이트 시점의 Far Plane
    float ShadowFarPlane = 1000.0f;
};

