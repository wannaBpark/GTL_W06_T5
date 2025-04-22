#include "DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "Math/JungleMath.h"
#include "Math/Rotator.h"
#include "Math/Quat.h"
#include "UObject/Casts.h"

UDirectionalLightComponent::UDirectionalLightComponent()
{

    DirectionalLightInfo.Direction = GetDirection();
    DirectionalLightInfo.Intensity = 10.0f;

    DirectionalLightInfo.LightColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

UDirectionalLightComponent::~UDirectionalLightComponent()
{
}

UObject* UDirectionalLightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->DirectionalLightInfo = DirectionalLightInfo;
    }
    
    return NewComponent;
}

void UDirectionalLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("LightColor"), *DirectionalLightInfo.LightColor.ToString());
    OutProperties.Add(TEXT("Intensity"), FString::Printf(TEXT("%f"), DirectionalLightInfo.Intensity));
    OutProperties.Add(TEXT("Direction"), *DirectionalLightInfo.Direction.ToString());
}

void UDirectionalLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("LightColor"));
    if (TempStr)
    {
        DirectionalLightInfo.LightColor.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Intensity"));
    if (TempStr)
    {
        DirectionalLightInfo.Intensity = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Direction"));
    if (TempStr)
    {
        DirectionalLightInfo.Direction.InitFromString(*TempStr);
    }
}


FVector UDirectionalLightComponent::GetDirection()  
{
    // 컴포넌트의 월드 회전을 얻습니다.
    const FRotator WorldRotation = GetWorldRotation();

    // 컴포넌트의 로컬 전방 벡터 (+X)를 월드 공간으로 회전시킵니다.
    // 이것이 빛이 '오는' 방향입니다.
    const FVector LightComesFromDirection = WorldRotation.RotateVector(FVector::ForwardVector);

    // 셰이더 등에서는 보통 표면에서 '빛으로 향하는' 방향을 사용하므로,
    // 빛이 오는 방향 벡터를 반전시킵니다.
    FVector DirectionToLight = -LightComesFromDirection;

    // 정규화 (RotateVector는 보통 길이를 유지하지만, 안전하게)
    DirectionToLight.Normalize(); // 또는 GetSafeNormal() 사용

    return DirectionToLight;
}

float UDirectionalLightComponent::GetShadowNearPlane() const
{
    return ShadowNearPlane;
}

const FDirectionalLightInfo& UDirectionalLightComponent::GetDirectionalLightInfo() const
{
    return DirectionalLightInfo;
}

void UDirectionalLightComponent::SetDirectionalLightInfo(const FDirectionalLightInfo& InDirectionalLightInfo)
{
    DirectionalLightInfo = InDirectionalLightInfo;
}

float UDirectionalLightComponent::GetIntensity() const
{
    return DirectionalLightInfo.Intensity;
}

void UDirectionalLightComponent::SetIntensity(float InIntensity)
{
    DirectionalLightInfo.Intensity = InIntensity;
}

FLinearColor UDirectionalLightComponent::GetLightColor() const
{
    return DirectionalLightInfo.LightColor;
}

void UDirectionalLightComponent::SetLightColor(const FLinearColor& InColor)
{
    DirectionalLightInfo.LightColor = InColor;
}

void UDirectionalLightComponent::UpdateViewMatrix(FVector TargetPosition)
{
    // 1. 라이트의 방향 벡터 가져오기 (월드 공간 기준)
    FVector LightDirection = GetDirection();

    // 2. View Matrix의 'Forward' 방향 설정
    // View 공간의 +Z 축(Forward)이 월드 공간의 LightDirection과 일치하도록 설정
    const FVector ViewForwardDirection = LightDirection;

    // 3. View Matrix의 'Up' 벡터 결정 
    FVector UpVector = FVector::UpVector;
    const float DotThreshold = 1.0f - KINDA_SMALL_NUMBER;
    if (FMath::Abs(ViewForwardDirection.Dot(FVector::UpVector)) > DotThreshold)
    {
        UpVector = FVector::ForwardVector;
    }

    // 4. View Matrix의 'Eye'와 'Target' 결정
    const FVector EyePosition = TargetPosition - ViewForwardDirection * 500.0f; // Eye는 LightDirection의 반대방향에 위치

    // 5. View Matrix 생성
    // 이제 CreateViewMatrix 내부에서 zAxis = normalize(Target - Eye) = normalize(ViewForwardDirection) = LightDirection 이 됨
    ViewMatrices[0] = JungleMath::CreateViewMatrix(EyePosition, TargetPosition, UpVector);
}

void UDirectionalLightComponent::UpdateViewMatrix()
{
}

void UDirectionalLightComponent::UpdateProjectionMatrix()
{
    ProjectionMatrix = JungleMath::CreateOrthoProjectionMatrix(
        OrthoWidth,
        OrthoHeight,
        ShadowNearPlane,
        ShadowFarPlane
    );
}

float UDirectionalLightComponent::GetShadowFrustumWidth() const
{
    return OrthoWidth;
}
