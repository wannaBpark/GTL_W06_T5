#include "PointLightComponent.h"

#include "Math/JungleMath.h"
#include "UObject/Casts.h"
#include "EngineLoop.h"

UPointLightComponent::UPointLightComponent()
{
    PointLightInfo.Position = GetWorldLocation();
    PointLightInfo.Radius = 30.f;

    PointLightInfo.LightColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    PointLightInfo.Intensity = 1000.f;
    PointLightInfo.Type = ELightType::POINT_LIGHT;
    PointLightInfo.Attenuation = 20.0f;

    // ShadowMap 생성 오버라이딩 함수는 생성자 시점에서 불릴 수 없기에 Initialize()로 호출
    Initialize(); 
}

UPointLightComponent::~UPointLightComponent()
{
}

void UPointLightComponent::Initialize()
{
    CreateShadowMap();
    InitShadowDebugView();
}

HRESULT UPointLightComponent::CreateShadowMap()
{
    // Shadow Cube Map 생성
    // Texture2D : 기존 그대로
    // DepthStencilView : Texture2DArray로 생성
    // ShaderResourceView : Texture2DArray로 생성
    FDepthStencilRHI NewResource;

    HRESULT hr = S_OK;

    D3D11_TEXTURE2D_DESC CubeMapTextureDesc = {};
    CubeMapTextureDesc.Width = ShadowMapWidth;
    CubeMapTextureDesc.Height = ShadowMapHeight;
    CubeMapTextureDesc.MipLevels = 1;
    CubeMapTextureDesc.ArraySize = NUM_FACES;
    CubeMapTextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    CubeMapTextureDesc.SampleDesc.Count = 1;
    CubeMapTextureDesc.SampleDesc.Quality = 0;
    CubeMapTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    CubeMapTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    hr = FEngineLoop::GraphicDevice.Device->CreateTexture2D(&CubeMapTextureDesc, nullptr, &NewResource.Texture2D);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Shadow Cube Map texture!"));
        return hr;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC CubeMapDSVDesc = {};
    CubeMapDSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
    CubeMapDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    CubeMapDSVDesc.Texture2D.MipSlice = 0;
    CubeMapDSVDesc.Texture2DArray.FirstArraySlice = 0;
    CubeMapDSVDesc.Texture2DArray.ArraySize = NUM_FACES;  // DSV 생성시 모든 6면을 포함
    hr = FEngineLoop::GraphicDevice.Device->CreateDepthStencilView(NewResource.Texture2D, &CubeMapDSVDesc, &NewResource.DSV);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Shadow Cube Map DSV!"));
        return hr;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC CubeMapSRVDesc = {};
    CubeMapSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
    CubeMapSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    CubeMapSRVDesc.Texture2DArray.MostDetailedMip = 0;
    CubeMapSRVDesc.Texture2DArray.MipLevels = 1;
    CubeMapSRVDesc.Texture2DArray.FirstArraySlice = 0;
    CubeMapSRVDesc.Texture2DArray.ArraySize = NUM_FACES;
    hr = FEngineLoop::GraphicDevice.Device->CreateShaderResourceView(NewResource.Texture2D, &CubeMapSRVDesc, &NewResource.SRV);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Shadow Cube Map SRV!"));
        return hr;
    }

    ShadowMaps.Add(NewResource);

    return hr;
}

void UPointLightComponent::InitShadowDebugView()
{
    for (int i = 0; i < 6; ++i)
    {
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = ShadowMapWidth;
        texDesc.Height = ShadowMapHeight;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R32_FLOAT;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        ID3D11Texture2D* texture = nullptr;
        auto hr = FEngineLoop::GraphicDevice.Device->CreateTexture2D(&texDesc, nullptr, &texture);
        if (FAILED(hr))
        {
            UE_LOG(LogLevel::Error, TEXT("Failed to create shadow debug texture!"));
            return;
        }
        OutputTextures.Add(texture);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        ID3D11ShaderResourceView* srv = nullptr;
        hr = FEngineLoop::GraphicDevice.Device->CreateShaderResourceView(texture, &srvDesc, &srv);
        if (FAILED(hr))
        {
            UE_LOG(LogLevel::Error, TEXT("Failed to create shadow debug SRV!"));
            return;
        }
        OutputSRVs.Add(srv);
    }
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

        // CubeMap이므로 6개의 ShadowMap을 생성합니다. (함수 내부에서)
        constexpr int32 ShadowMapCreationCount = 6;
        CreateShadowMap();

        bDirtyFlag = false;
    }
    return ShadowMaps;
}

ID3D11ShaderResourceView* UPointLightComponent::GetSliceSRV(int SliceIndex)
{
    //return ShadowMaps[0].SRV;
    UINT subresource = D3D11CalcSubresource(0, SliceIndex, 1);
    FEngineLoop::GraphicDevice.DeviceContext->CopySubresourceRegion(
        OutputTextures[SliceIndex],
        0, 0, 0, 0,
        ShadowMaps[0].Texture2D,
        subresource,
        nullptr
    );
    return OutputSRVs[SliceIndex];
}
