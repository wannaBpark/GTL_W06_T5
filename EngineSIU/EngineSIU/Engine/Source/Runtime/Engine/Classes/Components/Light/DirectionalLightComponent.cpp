#include "DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "Math/JungleMath.h"
#include "Math/Rotator.h"
#include "Math/Quat.h"
#include "UObject/Casts.h"

UDirectionalLightComponent::UDirectionalLightComponent()
{

    DirectionalLightInfo.Direction = -GetUpVector();
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
    FRotator rotator = GetWorldRotation();
    FVector WorldDown= rotator.ToQuaternion().RotateVector(-GetUpVector());
    return WorldDown;  
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

void UDirectionalLightComponent::UpdateViewMatrix()
{
    // 1. 라이트의 방향 벡터 가져오기 (월드 공간 기준)
    const FRotator LightRotation = GetWorldRotation();
    FVector LightDirection = LightRotation.RotateVector(FVector::ForwardVector);
    LightDirection.Normalize();

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
    // Target을 원점으로, Eye를 Target에서 ViewForwardDirection의 반대 방향으로 설정
    // 즉, Eye에서 Target을 바라보는 방향이 ViewForwardDirection(LightDirection)이 되도록.
    const FVector TargetPosition = FVector::ZeroVector;
    // Eye = Target - ViewForwardDirection
    const FVector EyePosition = TargetPosition - ViewForwardDirection * 1.0f; // Eye는 LightDirection의 반대방향에 위치

    // 5. View Matrix 생성
    // 이제 CreateViewMatrix 내부에서 zAxis = normalize(Target - Eye) = normalize(ViewForwardDirection) = LightDirection 이 됨
    ViewMatrices[0] = JungleMath::CreateViewMatrix(EyePosition, TargetPosition, UpVector);
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
