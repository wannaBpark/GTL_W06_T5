#include "LightComponent.h"
#include "UObject/Casts.h"

ULightComponentBase::ULightComponentBase()
{
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };
    CreateShadowMap();
}

ULightComponentBase::~ULightComponentBase()
{
  
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

HRESULT ULightComponentBase::CreateShadowMap()
{    
    FDepthStencilRHI NewResource;

    HRESULT hr = S_OK;

    D3D11_TEXTURE2D_DESC DepthStencilTextureDesc = {};
    DepthStencilTextureDesc.Width = ShadowMapWidth;
    DepthStencilTextureDesc.Height = ShadowMapHeight;
    DepthStencilTextureDesc.MipLevels = 1;
    DepthStencilTextureDesc.ArraySize = 1;
    DepthStencilTextureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    DepthStencilTextureDesc.SampleDesc.Count = 1;
    DepthStencilTextureDesc.SampleDesc.Quality = 0;
    DepthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    DepthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    DepthStencilTextureDesc.CPUAccessFlags = 0;
    DepthStencilTextureDesc.MiscFlags = 0;
    hr = FEngineLoop::GraphicDevice.Device->CreateTexture2D(&DepthStencilTextureDesc, nullptr, &NewResource.Texture2D);
    if (FAILED(hr))
    {
        return hr;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
    DepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DepthStencilViewDesc.Texture2D.MipSlice = 0;
    hr = FEngineLoop::GraphicDevice.Device->CreateDepthStencilView(NewResource.Texture2D, &DepthStencilViewDesc, &NewResource.DSV);
    if (FAILED(hr))
    {
        return hr;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC DepthStencilDesc = {};
    DepthStencilDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    DepthStencilDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    DepthStencilDesc.Texture2D.MostDetailedMip = 0;
    DepthStencilDesc.Texture2D.MipLevels = 1;
    hr = FEngineLoop::GraphicDevice.Device->CreateShaderResourceView(NewResource.Texture2D, &DepthStencilDesc, &NewResource.SRV);
    if (FAILED(hr))
    {
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
    for (auto ShadowMap : ShadowMaps)
    {
        DeviceContext->ClearDepthStencilView(ShadowMap.DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }
}

TArray<FDepthStencilRHI> ULightComponentBase::GetShadowMap()
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
