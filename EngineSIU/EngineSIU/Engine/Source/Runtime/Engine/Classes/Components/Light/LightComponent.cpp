#include "LightComponent.h"
#include "UObject/Casts.h"

ULightComponentBase::ULightComponentBase()
{
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };

    ViewMatrices.SetNum(1);
}

ULightComponentBase::~ULightComponentBase()
{
  
}
void ULightComponentBase::Initialize()
{
    CreateShadowMap();
}

UObject* ULightComponentBase::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->AABB = AABB;

    return NewComponent;
}

void ULightComponentBase::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("AABB_Min"), AABB.min.ToString());
    OutProperties.Add(TEXT("AABB_Max"), AABB.max.ToString());
}

void ULightComponentBase::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("AABB_Min"));
    if (TempStr)
    {
        AABB.min.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("AABB_Max"));
    if (TempStr)
    {
        AABB.max.InitFromString(*TempStr);
    }
}

void ULightComponentBase::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

int ULightComponentBase::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    bool res = AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance);
    return res;
}

void ULightComponentBase::UpdateViewMatrix()
{
}

void ULightComponentBase::UpdateProjectionMatrix()
{
}

// TArray<FDepthStencilRHI> ULightComponentBase::GetShadowMap()
// {
//     // ShadowMap의 크기가 바뀐 경우 새로 생성합니다.
//     if (bDirtyFlag)
//     {
//         if (HasShadowMap())
//         {
//             ReleaseShadowMap();
//         }
//
//         CreateShadowMap();
//
//         bDirtyFlag = false;
//     }
//     return ShadowMaps;
// }

void ULightComponentBase::SetShadowMapSize(const uint32 InWidth, const uint32 InHeight)
{
    // 값이 다른 경우에만 Flag 적용
    if (InWidth == ShadowMapWidth && InHeight == ShadowMapHeight)
    {
        return;
    }

    ShadowMapWidth = InWidth;
    ShadowMapHeight = InHeight;
    bDirtyFlag = true;
}
