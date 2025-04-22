#include "TileLightCullingPass.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "UnrealClient.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"
#include "UObject/Casts.h"
#include "Engine/EditorEngine.h"
#include "Components/Light/LightComponent.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "UObject/UObjectIterator.h"

#define SAFE_RELEASE(p) if (p) { (p)->Release(); (p) = nullptr; }

#define PRINTDEBUG FALSE

FTileLightCullingPass::FTileLightCullingPass()
{
}

FTileLightCullingPass::~FTileLightCullingPass()
{
}

void FTileLightCullingPass::ResizeTiles(const UINT InWidth, const UINT InHeight)
{
    TILE_COUNT_X = (InWidth + TILE_SIZE - 1) / TILE_SIZE;
    TILE_COUNT_Y = (InHeight + TILE_SIZE - 1) / TILE_SIZE;
    TILE_COUNT = TILE_COUNT_X * TILE_COUNT_Y;
    SHADER_ENTITY_TILE_BUCKET_COUNT = MAX_LIGHTS_PER_TILE / 32;
}

void FTileLightCullingPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManage;

    ResizeTiles(Graphics->ScreenWidth, Graphics->ScreenHeight); // 시작은 전체 크기
    // 한 타일이 가질 수 있는 조명 ID 목록을 비트마스크로 표현한 총 슬롯 수

    CreateShader();
    CreateViews();
    CreateBuffers(Graphics->ScreenWidth, Graphics->ScreenHeight); // 시작은 전체 크기
}

void FTileLightCullingPass::PrepareRenderArr()
{
    for (const auto iter : TObjectRange<ULightComponentBase>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(iter))
            {
                PointLights.Add(PointLight);
            }
            else if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(iter))
            {
                SpotLight->GetDirection();
                SpotLights.Add(SpotLight);
            }
            iter->UpdateViewMatrix();
            iter->UpdateProjectionMatrix();
        }
    }

    CreatePointLightBufferGPU();
    CreateSpotLightBufferGPU();
}

void FTileLightCullingPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    DepthSRV = Viewport->GetViewportResource()->GetDepthStencil(
        EResourceType::ERT_Debug
    )->SRV;
    ComputeShader = ShaderManager->GetComputeShaderByKey(L"TileLightCullingComputeShader");
    UpdateTileLightConstantBuffer(Viewport);
    Dispatch(Viewport);
}

void FTileLightCullingPass::Dispatch(const std::shared_ptr<FEditorViewportClient>& Viewport) const
{
    // 한 스레드 그룹(groupSizeX, groupSizeY)은 16x16픽셀 영역처리
    const UINT GroupSizeX = (Viewport->GetD3DViewport().Width  + TILE_SIZE - 1) / TILE_SIZE;
    const UINT GroupSizeY = (Viewport->GetD3DViewport().Height + TILE_SIZE - 1) / TILE_SIZE;

    Graphics->DeviceContext->CSSetConstantBuffers(0, 1, &TileLightConstantBuffer);

    // 1. SRV (전역 Light 정보) 바인딩
    if (PointLightBufferSRV)
    {
        Graphics->DeviceContext->CSSetShaderResources(0, 1, &PointLightBufferSRV);                  // register(t0)
    }
    if (SpotLightBufferSRV)
    {
        Graphics->DeviceContext->CSSetShaderResources(2, 1, &SpotLightBufferSRV);                  // register(t0)
    }
    if (DepthSRV)
    {
        Graphics->DeviceContext->CSSetShaderResources(1, 1, &DepthSRV);                  // register(t1)
    }

    // 2. UAV 바인딩
    Graphics->DeviceContext->CSSetUnorderedAccessViews(0, 1, &PerTilePointLightIndexMaskBufferUAV, nullptr);    // register(u0)
	Graphics->DeviceContext->CSSetUnorderedAccessViews(1, 1, &PerTileSpotLightIndexMaskBufferUAV, nullptr);     // register(u1)
	Graphics->DeviceContext->CSSetUnorderedAccessViews(3, 1, &DebugHeatmapUAV, nullptr);                        // register(u3)

    // 3. 셰이더 바인딩
    Graphics->DeviceContext->CSSetShader(ComputeShader, nullptr, 0);

    // 4. 디스패치
    Graphics->DeviceContext->Dispatch(GroupSizeX, GroupSizeY, 1);

    // 5-1. UAV 바인딩 해제 (다른 렌더패스에서 사용하기 위함)
    ID3D11UnorderedAccessView* NullUAV = nullptr;
    Graphics->DeviceContext->CSSetUnorderedAccessViews(0, 1, &NullUAV, nullptr);
	Graphics->DeviceContext->CSSetUnorderedAccessViews(1, 1, &NullUAV, nullptr);
	Graphics->DeviceContext->CSSetUnorderedAccessViews(3, 1, &NullUAV, nullptr);

    // 5-2. SRV 해제
    ID3D11ShaderResourceView* NullSRVs[2] = { nullptr, nullptr };
    Graphics->DeviceContext->CSSetShaderResources(0, 2, NullSRVs);

    // 5-3. 상수버퍼 해제
    ID3D11Buffer* NullBuffer[1] = { nullptr };
    Graphics->DeviceContext->CSSetConstantBuffers(0, 1, NullBuffer);
}

void FTileLightCullingPass::ClearRenderArr()
{
    ClearUAVs();

    PointLights.Empty();
    SpotLights.Empty();
}

void FTileLightCullingPass::CreateShader()
{
    // Compute Shader 생성
    HRESULT hr = ShaderManager->AddComputeShader(L"TileLightCullingComputeShader", L"Shaders/TileLightCullingComputeShader.hlsl", "mainCS");
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to Compile Compute Shader!"));
    }
    ComputeShader = ShaderManager->GetComputeShaderByKey(L"TileLightCullingComputeShader");

}

void FTileLightCullingPass::CreatePointLightBufferGPU()
{
    if (PointLights.Num() == 0)
    {
        return;
    }

    TArray<FPointLightGPU> Lights;

    for (UPointLightComponent* LightComp : PointLights)
    {
        if (!LightComp) continue;
        FPointLightGPU LightData;
        LightData = {
            .Position = LightComp->GetWorldLocation(),
            .Radius = LightComp->GetRadius(),
            .Direction = LightComp->GetUpVector(),
            .Padding = 0.0f
        };
        Lights.Add(LightData);
    }

    D3D11_BUFFER_DESC Desc = {};
    Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    Desc.ByteWidth = sizeof(FPointLightGPU) * Lights.Num();
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.StructureByteStride = sizeof(FPointLightGPU);
    Desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = Lights.GetData();

    SAFE_RELEASE(PointLightBuffer)
    SAFE_RELEASE(PointLightBufferSRV)

    HRESULT hr = Graphics->Device->CreateBuffer(&Desc, &InitData, &PointLightBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Light Structured Buffer!"));
        return;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
    SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    SrvDesc.Buffer.FirstElement = 0;
    SrvDesc.Buffer.NumElements = Lights.Num();

    hr = Graphics->Device->CreateShaderResourceView(PointLightBuffer, &SrvDesc, &PointLightBufferSRV);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Light Buffer SRV!"));
    }
}

void FTileLightCullingPass::CreateSpotLightBufferGPU()
{
    if (SpotLights.Num() == 0)
        return;

    TArray<FSpotLightGPU> Lights;

    for (USpotLightComponent* LightComp : SpotLights)
    {
        if (!LightComp) continue;
        FSpotLightGPU LightData;
        LightData = {
            .Position = LightComp->GetWorldLocation(),
            .Radius = LightComp->GetRadius(),
            .Direction = LightComp->GetDirection(),
            .Angle = LightComp->GetOuterDegree(),
        };
        Lights.Add(LightData);
    }

    D3D11_BUFFER_DESC Desc = {};
    Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    Desc.ByteWidth = sizeof(FSpotLightGPU) * Lights.Num();
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.StructureByteStride = sizeof(FSpotLightGPU);
    Desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = Lights.GetData();

    SAFE_RELEASE(SpotLightBuffer)
    SAFE_RELEASE(SpotLightBufferSRV)

    HRESULT hr = Graphics->Device->CreateBuffer(&Desc, &InitData, &SpotLightBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Light Structured Buffer!"));
        return;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
    SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    SrvDesc.Buffer.FirstElement = 0;
    SrvDesc.Buffer.NumElements = Lights.Num();

    hr = Graphics->Device->CreateShaderResourceView(SpotLightBuffer, &SrvDesc, &SpotLightBufferSRV);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Light Buffer SRV!"));
    }
}

void FTileLightCullingPass::CreateViews()
{
    // 2. entityTiles UAV buffer (tile mask 결과용)
    D3D11_BUFFER_DESC tileBufferDesc = {};
    tileBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    tileBufferDesc.ByteWidth = sizeof(uint32) * TILE_COUNT * SHADER_ENTITY_TILE_BUCKET_COUNT;
    tileBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    tileBufferDesc.StructureByteStride = sizeof(uint32);
    tileBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_UNORDERED_ACCESS_VIEW_DESC tileUAVDesc = {};
    tileUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    tileUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
    tileUAVDesc.Buffer.FirstElement = 0;
    tileUAVDesc.Buffer.NumElements = TILE_COUNT * SHADER_ENTITY_TILE_BUCKET_COUNT;

    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
    SRVDesc.Buffer.FirstElement = 0;
    SRVDesc.Buffer.NumElements = TILE_COUNT * SHADER_ENTITY_TILE_BUCKET_COUNT;

    // PointLight Buffer
    HRESULT hr = Graphics->Device->CreateBuffer(&tileBufferDesc, nullptr, &PerTilePointLightIndexMaskBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Tile UAV Buffer!"));
    }

    hr = Graphics->Device->CreateUnorderedAccessView(PerTilePointLightIndexMaskBuffer, &tileUAVDesc, &PerTilePointLightIndexMaskBufferUAV);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Tile UAV!"));
    }

    hr = Graphics->Device->CreateShaderResourceView(PerTilePointLightIndexMaskBuffer, &SRVDesc, &PerTilePointLightIndexMaskBufferSRV);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Tile SRV!"));
    }

    // SpotLight Buffer
	hr = Graphics->Device->CreateBuffer(&tileBufferDesc, nullptr, &PerTileSpotLightIndexMaskBuffer);
	if (FAILED(hr))
	{
		UE_LOG(LogLevel::Error, TEXT("Failed to create Tile SpotLight UAV Buffer!"));
	}

	hr = Graphics->Device->CreateUnorderedAccessView(PerTileSpotLightIndexMaskBuffer, &tileUAVDesc, &PerTileSpotLightIndexMaskBufferUAV);
	if (FAILED(hr))
	{
		UE_LOG(LogLevel::Error, TEXT("Failed to create Tile Spot UAV!"));
	}

    hr = Graphics->Device->CreateShaderResourceView(PerTileSpotLightIndexMaskBuffer, &SRVDesc, &PerTileSpotLightIndexMaskBufferSRV);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Tile Spot SRV!"));
    }
}

void FTileLightCullingPass::CreateBuffers(uint32 InWidth, uint32 InHeight)
{
    // 3. Debug heatmap 텍스처 + UAV (디버깅용)
    D3D11_TEXTURE2D_DESC HeatMapDesc = {};
    HeatMapDesc.Width = InWidth;
    HeatMapDesc.Height = InHeight;
    HeatMapDesc.MipLevels = 1;
    HeatMapDesc.ArraySize = 1;
    HeatMapDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // R32G32B32A32_FLOAT
    HeatMapDesc.SampleDesc.Count = 1;
    HeatMapDesc.Usage = D3D11_USAGE_DEFAULT;
    HeatMapDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = Graphics->Device->CreateTexture2D(&HeatMapDesc, nullptr, &DebugHeatmapTexture);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Heatmap Texture!"));
    }

    D3D11_UNORDERED_ACCESS_VIEW_DESC DebugUAVDesc = {};
    DebugUAVDesc.Format = HeatMapDesc.Format;
    DebugUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

    hr = Graphics->Device->CreateUnorderedAccessView(DebugHeatmapTexture, &DebugUAVDesc, &DebugHeatmapUAV);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Heatmap UAV!"));
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Format = HeatMapDesc.Format;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MostDetailedMip = 0;
    SRVDesc.Texture2D.MipLevels = 1;

    hr = Graphics->Device->CreateShaderResourceView(DebugHeatmapTexture, &SRVDesc, &DebugHeatmapSRV);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Heatmap SRV!"));
    }

    //// 4. TileLight Culling 설정용 ConstantBuffer (2.5D 켜기/끄기 등)
    D3D11_BUFFER_DESC CBDesc = {};
    CBDesc.Usage = D3D11_USAGE_DYNAMIC;
    CBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    CBDesc.ByteWidth = (sizeof(TileLightCullSettings) + 0xf) & 0xfffffff0; // struct 정의 필요
    CBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = Graphics->Device->CreateBuffer(&CBDesc, nullptr, &TileLightConstantBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create TileLight Constant Buffer!"));
    }
}

void FTileLightCullingPass::Release()
{
    // Compute Shader Release는 ShaderManager에서 관리
    SAFE_RELEASE(PerTilePointLightIndexMaskBuffer)
    SAFE_RELEASE(PerTilePointLightIndexMaskBufferUAV)
    SAFE_RELEASE(PerTilePointLightIndexMaskBufferSRV)

	SAFE_RELEASE(PerTileSpotLightIndexMaskBuffer)
	SAFE_RELEASE(PerTileSpotLightIndexMaskBufferUAV)
    SAFE_RELEASE(PerTileSpotLightIndexMaskBufferSRV)
    
    SAFE_RELEASE(DebugHeatmapTexture)
    SAFE_RELEASE(DebugHeatmapUAV)
    SAFE_RELEASE(DebugHeatmapSRV)

    SAFE_RELEASE(TileLightConstantBuffer)
    
    SAFE_RELEASE(PointLightBuffer)
    SAFE_RELEASE(PointLightBufferSRV)

    SAFE_RELEASE(SpotLightBuffer)
    SAFE_RELEASE(SpotLightBufferSRV)
}

void FTileLightCullingPass::ClearUAVs() const
{
    // UAV 초기화용 zero값
    constexpr UINT ClearColor[4] = { 0, 0, 0, 0 };

    // 1. 타일 마스크 초기화
	Graphics->DeviceContext->ClearUnorderedAccessViewUint(PerTilePointLightIndexMaskBufferUAV, ClearColor);
	Graphics->DeviceContext->ClearUnorderedAccessViewUint(PerTileSpotLightIndexMaskBufferUAV, ClearColor);

    // 2. 히트맵 초기화
    constexpr float ClearColorF[4] = { 0, 0, 0, 0 };
    Graphics->DeviceContext->ClearUnorderedAccessViewFloat(DebugHeatmapUAV, ClearColorF);
}

void FTileLightCullingPass::UpdateTileLightConstantBuffer(const std::shared_ptr<FEditorViewportClient>& Viewport) const
{
    // 1. Constant Buffer 업데이트
    TileLightCullSettings Settings;
    Settings.ScreenSize[0] = static_cast<uint32>(Viewport->GetD3DViewport().Width);
    Settings.ScreenSize[1] = static_cast<uint32>(Viewport->GetD3DViewport().Height);
    Settings.TileSize[0] = TILE_SIZE;
    Settings.TileSize[1] = TILE_SIZE;
    Settings.NearZ = Viewport->NearClip;
    Settings.FarZ = Viewport->FarClip;
    Settings.ViewMatrix = Viewport->GetViewMatrix();
    Settings.ProjectionMatrix = Viewport->GetProjectionMatrix();
    Settings.InvProjectionMatrix = FMatrix::Inverse(Viewport->GetProjectionMatrix());
    Settings.NumPointLights = PointLights.Num();
    Settings.NumSpotLights = SpotLights.Num();
    Settings.Enable25DCulling = 1;                      // TODO : IMGUI 연결!

    D3D11_MAPPED_SUBRESOURCE MSR;

    HRESULT hr = Graphics->DeviceContext->Map(TileLightConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MSR);
    if (FAILED(hr)) {
        UE_LOG(LogLevel::Error, TEXT("Failed to map TileLightConstantBuffer"));
        return;
    }
    memcpy(MSR.pData, &Settings, sizeof(TileLightCullSettings));
    
    Graphics->DeviceContext->Unmap(TileLightConstantBuffer, 0);
}

// Compute Shader에 사용되는 모든 SRV와 UAV를 해제
void FTileLightCullingPass::ResizeViewBuffers(const uint32 InWidth, const uint32 InHeight)
{
    ResizeTiles(InWidth, InHeight);

    Release();

    CreateViews();
    CreateBuffers(InWidth, InHeight);
}

// TileUAVBuffer = 조명 비트마스크 결과를 저장 UAV 
// UAV -> StagingBuffer로 복사하는 코드를 만들 것임 (UAV는 USAGE_DEFAULT라서 CPU에서 읽을 수 없음)
bool FTileLightCullingPass::CopyPerTileLightIndexMaskBufferToCPU(TArray<uint32>& OutData, ID3D11Buffer*& PerTileLightIndexMaskBuffer) const
{
    D3D11_BUFFER_DESC BufferDesc = {};
    PerTileLightIndexMaskBuffer->GetDesc(&BufferDesc);
    BufferDesc.Usage = D3D11_USAGE_STAGING;
    BufferDesc.BindFlags = 0; 
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ; // CPU 읽기 가능

    ID3D11Buffer* StagingBuffer = nullptr;

    HRESULT hr = Graphics->Device->CreateBuffer(&BufferDesc, nullptr, &StagingBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create staging buffer for TileUAVBuffer"));
        return false;
    }

    Graphics->DeviceContext->CopyResource(StagingBuffer, PerTileLightIndexMaskBuffer);

    D3D11_MAPPED_SUBRESOURCE MSR = {};

    hr = Graphics->DeviceContext->Map(StagingBuffer, 0, D3D11_MAP_READ, 0, &MSR);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("TileUAVBuffer Staging Buffer Mapping Failed"));
        SAFE_RELEASE(StagingBuffer)
        return false;
    }

    const int Count = static_cast<int>(BufferDesc.ByteWidth) / sizeof(uint32);
    OutData.SetNum(Count);
    memcpy(OutData.GetData(), MSR.pData, BufferDesc.ByteWidth);

    Graphics->DeviceContext->Unmap(StagingBuffer, 0);
    SAFE_RELEASE(StagingBuffer)

    return true;
}

