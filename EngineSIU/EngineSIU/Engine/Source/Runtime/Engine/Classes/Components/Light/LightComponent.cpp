#include "LightComponent.h"
#include "UObject/Casts.h"

ULightComponentBase::ULightComponentBase()
{
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };

    ViewMatrices.SetNum(1);
    Initialize();    
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

HRESULT ULightComponentBase::CreateShadowMap()
{    
    FDepthStencilRHI NewResource;

    HRESULT hr = S_OK;

    D3D11_TEXTURE2D_DESC ShadowBufferDesc = {};
    ShadowBufferDesc.Width = ShadowMapWidth;
    ShadowBufferDesc.Height = ShadowMapHeight;
    ShadowBufferDesc.MipLevels = 1;
    ShadowBufferDesc.ArraySize = 1;
    ShadowBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    ShadowBufferDesc.SampleDesc.Count = 1;
    ShadowBufferDesc.SampleDesc.Quality = 0;
    ShadowBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    ShadowBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    ShadowBufferDesc.CPUAccessFlags = 0;
    ShadowBufferDesc.MiscFlags = 0;
    hr = FEngineLoop::GraphicDevice.Device->CreateTexture2D(&ShadowBufferDesc, nullptr, &NewResource.Texture2D);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create shadow map texture!"));
        return hr;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC ShadowDSVDesc = {};
    ShadowDSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
    ShadowDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    ShadowDSVDesc.Texture2D.MipSlice = 0;
    hr = FEngineLoop::GraphicDevice.Device->CreateDepthStencilView(NewResource.Texture2D, &ShadowDSVDesc, &NewResource.DSV);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create shadow map DSV!"));
        return hr;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC ShadowSRVDesc = {};
    ShadowSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
    ShadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ShadowSRVDesc.Texture2D.MostDetailedMip = 0;
    ShadowSRVDesc.Texture2D.MipLevels = 1;
    hr = FEngineLoop::GraphicDevice.Device->CreateShaderResourceView(NewResource.Texture2D, &ShadowSRVDesc, &NewResource.SRV);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create shadow map SRV!"));
        return hr;
    }

    ShadowMaps.Add(NewResource);

    return hr;
}

void ULightComponentBase::ReleaseShadowMap()
{
    for (auto ShadowMap : ShadowMaps)
    {
        ShadowMap.Release();
    }
}

void ULightComponentBase::ClearShadowMap(ID3D11DeviceContext* DeviceContext)
{
    for (const auto& ShadowMap : ShadowMaps)
    {
        DeviceContext->ClearDepthStencilView(ShadowMap.DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }
}

TArray<FDepthStencilRHI>& ULightComponentBase::GetShadowMap()
{
    // ShadowMap의 크기가 바뀐 경우 새로 생성합니다.
    if (bDirtyFlag)
    {
        if (HasShadowMap())
        {
            ReleaseShadowMap();
        }

        CreateShadowMap();

        bDirtyFlag = false;
    }
    return ShadowMaps;
}

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
