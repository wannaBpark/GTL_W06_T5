#include "PointLightComponent.h"

#include "Math/JungleMath.h"
#include "UObject/Casts.h"

UPointLightComponent::UPointLightComponent()
{
    PointLightInfo.Position = GetWorldLocation();
    PointLightInfo.Radius = 30.f;

    PointLightInfo.LightColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    PointLightInfo.Intensity = 1000.f;
    PointLightInfo.Type = ELightType::POINT_LIGHT;
    PointLightInfo.Attenuation = 20.0f;

    // CubeMap이므로 6개의 ShadowMap을 생성합니다.
    constexpr int32 ShadowMapCreationCount = 6;  
    for (int32 i = 0; i < ShadowMapCreationCount; ++i)  
    {  
       CreateShadowMap();  
    }
}

UPointLightComponent::~UPointLightComponent()
{
}

UObject* UPointLightComponent::Duplicate(UObject* InOuter)
{

    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->PointLightInfo = PointLightInfo;
    }
    return NewComponent;
}

void UPointLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Radius"), FString::Printf(TEXT("%f"), PointLightInfo.Radius));
    OutProperties.Add(TEXT("LightColor"), FString::Printf(TEXT("%s"), *PointLightInfo.LightColor.ToString()));
    OutProperties.Add(TEXT("Intensity"), FString::Printf(TEXT("%f"), PointLightInfo.Intensity));
    OutProperties.Add(TEXT("Type"), FString::Printf(TEXT("%d"), PointLightInfo.Type));
    OutProperties.Add(TEXT("Attenuation"), FString::Printf(TEXT("%f"), PointLightInfo.Attenuation));
    OutProperties.Add(TEXT("Position"), FString::Printf(TEXT("%s"), *PointLightInfo.Position.ToString()));
}

void UPointLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("Radius"));
    if (TempStr)
    {
        PointLightInfo.Radius = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightColor"));
    if (TempStr)
    {
        PointLightInfo.LightColor.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Intensity"));
    if (TempStr)
    {
        PointLightInfo.Intensity = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Type"));
    if (TempStr)
    {
        PointLightInfo.Type = FString::ToInt(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Attenuation"));
    if (TempStr)
    {
        PointLightInfo.Attenuation = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Position"));
    if (TempStr)
    {
        PointLightInfo.Position.InitFromString(*TempStr);
    }
    
}

const FPointLightInfo& UPointLightComponent::GetPointLightInfo() const
{
    return PointLightInfo;
}

void UPointLightComponent::SetPointLightInfo(const FPointLightInfo& InPointLightInfo)
{
    PointLightInfo = InPointLightInfo;
}


float UPointLightComponent::GetRadius() const
{
    return PointLightInfo.Radius;
}

void UPointLightComponent::SetRadius(float InRadius)
{
    PointLightInfo.Radius = InRadius;
}

FLinearColor UPointLightComponent::GetLightColor() const
{
    return PointLightInfo.LightColor;
}

void UPointLightComponent::SetLightColor(const FLinearColor& InColor)
{
    PointLightInfo.LightColor = InColor;
}


float UPointLightComponent::GetIntensity() const
{
    return PointLightInfo.Intensity;
}

void UPointLightComponent::SetIntensity(float InIntensity)
{
    PointLightInfo.Intensity = InIntensity;
}

int UPointLightComponent::GetType() const
{
    return PointLightInfo.Type;
}

void UPointLightComponent::SetType(int InType)
{
    PointLightInfo.Type = InType;
}

void UPointLightComponent::UpdateViewMatrix()
{
    FVector PointLightPos = GetWorldLocation();

    // ViewMatrices 배열의 크기가 6인지 확인하고, 아니면 조정합니다.
    // 생성자 등에서 미리 크기를 6으로 설정하는 것이 더 효율적일 수 있습니다.
    if (ViewMatrices.Num() != 6)
    {
        ViewMatrices.SetNum(6);
    }
    // 1. +X 면 (World Forward)
    // Target: 정면 / Up: 월드 위쪽 (Z+)
    ViewMatrices[0] = JungleMath::CreateViewMatrix(PointLightPos, PointLightPos + FVector::ForwardVector, FVector::UpVector);

    // 2. -X 면 (World Backward)
    // Target: 후면 / Up: 월드 위쪽 (Z+)
    ViewMatrices[1] = JungleMath::CreateViewMatrix(PointLightPos, PointLightPos - FVector::ForwardVector, FVector::UpVector);

    // 3. +Y 면 (World Right)
    // Target: 우측 / Up: 월드 위쪽 (Z+)
    ViewMatrices[2] = JungleMath::CreateViewMatrix(PointLightPos, PointLightPos + FVector::RightVector, FVector::UpVector);

    // 4. -Y 면 (World Left)
    // Target: 좌측 / Up: 월드 위쪽 (Z+)
    ViewMatrices[3] = JungleMath::CreateViewMatrix(PointLightPos, PointLightPos - FVector::RightVector, FVector::UpVector);

    // 5. +Z 면 (World Up)
    // Target: 위쪽 / Up: 월드 정면 (X+) -> 위를 볼 때, 화면 상단이 월드의 정면 방향이 되도록 설정
    // Up 벡터가 시선 방향(Z+)과 평행하면 안 되므로 다른 축 사용
    ViewMatrices[4] = JungleMath::CreateViewMatrix(PointLightPos, PointLightPos + FVector::UpVector, FVector::ForwardVector); // Up: World Forward (+X)

    // 6. -Z 면 (World Down)
    // Target: 아래쪽 / Up: 월드 정면 (X+) -> 아래를 볼 때, 화면 상단이 월드의 정면 방향이 되도록 설정
    // Up 벡터가 시선 방향(-Z)과 평행하면 안 되므로 다른 축 사용
    ViewMatrices[5] = JungleMath::CreateViewMatrix(PointLightPos, PointLightPos - FVector::UpVector, FVector::ForwardVector); // Up: World Forward (+X)
}

void UPointLightComponent::UpdateProjectionMatrix()
{
    // 1. 포인트 라이트용 파라미터 설정
    // FOV: 큐브맵 각 면은 90도 시야각을 가짐 (PI / 2 라디안)
    const float FieldOfViewRadians = PI / 2.0f; // 90 degrees in radians

    // Aspect Ratio: 큐브맵 각 면은 정사각형이므로 종횡비는 1.0
    const float CurrentAspectRatio = 1.0f;

    // Near Clip Plane 값 설정 (매우 작은 값 사용)
    const float NearClipPlane = NEAR_PLANE; // 또는 직접 상수 값 사용 (예: 0.01f)

    // Far Clip Plane 값 설정 (라이트의 감쇠 반경 사용)
    // GetRadius() 함수나 멤버 변수(PointLightInfo.Radius)를 통해 가져옴
    const float FarClipPlane = GetRadius();

    // 2. 원근 투영 행렬 생성
    ProjectionMatrix = JungleMath::CreateProjectionMatrix(
        FieldOfViewRadians,
        CurrentAspectRatio,
        NearClipPlane,
        FarClipPlane
    );
}


TArray<FDepthStencilRHI> UPointLightComponent::GetShadowMap()
{
    // ShadowMap의 크기가 바뀐 경우 새로 생성합니다.
    if (bDirtyFlag)
    {
        if (HasShadowMap())
        {
            ReleaseShadowMap();
        }

        // CubeMap이므로 6개의 ShadowMap을 생성합니다.
        constexpr int32 ShadowMapCreationCount = 6;
        for (int32 i = 0; i < ShadowMapCreationCount; ++i)
        {
            CreateShadowMap();
        }

        bDirtyFlag = false;
    }
    return ShadowMaps;
}
