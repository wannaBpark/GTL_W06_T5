#include "ShadowRenderPass.h"

#include "Components/Light/LightComponent.h"
#include "Components/Light/PointLightComponent.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "UObject/Casts.h"
#include "UnrealEd/EditorViewportClient.h"

FShadowRenderPass::FShadowRenderPass()
{
}

FShadowRenderPass::~FShadowRenderPass()
{
}

void FShadowRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;

    // DepthOnly Vertex Shader
    CreateShader();
    CreateSampler();
    UpdateViewport(ShadowMapWidth, ShadowMapHeight);
}

void FShadowRenderPass::PrepareRenderState(ULightComponentBase* Light)
{
    // Shader Hot Reload 대응 
    StaticMeshIL = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
    DepthOnlyVS = ShaderManager->GetVertexShaderByKey(L"DepthOnlyVS");
    DepthOnlyPS = ShaderManager->GetPixelShaderByKey(L"DepthOnlyPS");
    
    Graphics->DeviceContext->IASetInputLayout(StaticMeshIL);
    Graphics->DeviceContext->VSSetShader(DepthOnlyVS, nullptr, 0);

    // Note : PS만 언바인드할 뿐, UpdateLightBuffer에서 바인딩된 SRV 슬롯들은 그대로 남아 있음
    Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
    Graphics->DeviceContext->RSSetState(Graphics->RasterizerShadow);

    ShadowMapWidth = Light->GetShadowMapWidth();
    ShadowMapHeight = Light->GetShadowMapHeight();
    UpdateViewport(ShadowMapWidth, ShadowMapHeight);
}

void FShadowRenderPass::PrepareRenderArr()
{
}

void FShadowRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{

}

void FShadowRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport, ULightComponentBase* Light)
{
    if (Cast<UDirectionalLightComponent>(Light))
    {
        Graphics->DeviceContext->ClearDepthStencilView(Light->GetShadowMap()[0].DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
        PrepareRenderState(Light);
        BufferManager->BindConstantBuffer(TEXT("FShadowConstantBuffer"), 11, EShaderStage::Vertex);
        BufferManager->BindConstantBuffer(TEXT("FShadowConstantBuffer"), 11, EShaderStage::Pixel);

        Graphics->DeviceContext->RSSetViewports(1, &ShadowViewport);
        Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, Light->GetShadowMap()[0].DSV);
    }

}

void FShadowRenderPass::ClearRenderArr()
{
    Graphics->DeviceContext->RSSetViewports(0, nullptr);
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    Graphics->DeviceContext->GSSetShader(nullptr, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
    Graphics->DeviceContext->VSSetShader(nullptr, nullptr, 0);

}

void FShadowRenderPass::CreateShader()
{
    HRESULT hr = ShaderManager->AddVertexShader(L"DepthOnlyVS", L"Shaders/DepthOnlyVS.hlsl", "mainVS");
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create DepthOnlyVS shader!"));
    }
    hr = ShaderManager->AddVertexShader(L"DepthCubeMapVS", L"Shaders/DepthCubeMapVS.hlsl", "mainVS");
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create DepthCubeMapVS shader!"));
    }

    hr = ShaderManager->AddGeometryShader(L"DepthCubeMapGS", L"Shaders/PointLightCubemapGS.hlsl", "mainGS");
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create DepthCubeMapGS shader!"));
    }

    hr = ShaderManager->AddPixelShader(L"DepthOnlyPS", L"Shaders/PointLightCubemapGS.hlsl", "mainPS");
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create DepthOnlyPS shader!"));
    }

    StaticMeshIL = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
    DepthOnlyVS = ShaderManager->GetVertexShaderByKey(L"DepthOnlyVS");
    DepthOnlyPS = ShaderManager->GetPixelShaderByKey(L"DepthOnlyPS");
}

void FShadowRenderPass::UpdateViewport(const uint32& InWidth, const uint32& InHeight)
{
    // Set the viewport
    ZeroMemory(&ShadowViewport, sizeof(D3D11_VIEWPORT));
    ShadowViewport.TopLeftX = 0;
    ShadowViewport.TopLeftY = 0;
    ShadowViewport.Width = static_cast<float>(InWidth);
    ShadowViewport.Height = static_cast<float>(InHeight);
    ShadowViewport.MinDepth = 0.0f;
    ShadowViewport.MaxDepth = 1.0f;
}

void FShadowRenderPass::CreateSampler()
{
    D3D11_SAMPLER_DESC SamplerDesc = {};
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    SamplerDesc.BorderColor[0] = 1.f;
    SamplerDesc.BorderColor[1] = 1.f;
    SamplerDesc.BorderColor[2] = 1.f;
    SamplerDesc.BorderColor[3] = 1.f;
    SamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    SamplerDesc.MinLOD = 0.f;
    SamplerDesc.MaxLOD = 0.f;
    SamplerDesc.MipLODBias = 0.f;
    HRESULT hr = Graphics->Device->CreateSamplerState(&SamplerDesc, &Sampler);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Sampler!"));
    }
}

void FShadowRenderPass::PrepareCubeMapRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport, UPointLightComponent*& PointLight)
{
    DepthCubeMapVS = ShaderManager->GetVertexShaderByKey(L"DepthCubeMapVS");
    DepthCubeMapGS = ShaderManager->GetGeometryShaderByKey(L"DepthCubeMapGS");
    DepthOnlyPS = ShaderManager->GetPixelShaderByKey(L"DepthOnlyPS");
    StaticMeshIL = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");

    /*auto*& DSV = Viewport->GetViewportResource()->GetDepthStencil(EResourceType::ERT_Scene)->DSV;*/
    auto sm = PointLight->GetShadowMap();
    auto*& DSV = sm[1].DSV;
    Graphics->DeviceContext->ClearDepthStencilView(DSV,
        D3D11_CLEAR_DEPTH, 1.0f, 0);
    Graphics->DeviceContext->ClearRenderTargetView(PointLight->DepthRTVArray, ClearColor);
    Graphics->DeviceContext->OMSetRenderTargets(1, &PointLight->DepthRTVArray, DSV);
    Graphics->DeviceContext->IASetInputLayout(StaticMeshIL);

    Graphics->DeviceContext->VSSetShader(DepthCubeMapVS, nullptr, 0);
    Graphics->DeviceContext->GSSetShader(DepthCubeMapGS, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(DepthOnlyPS, nullptr, 0);
    // VS, GS에 대한 상수버퍼 업데이트
    BufferManager->BindConstantBuffer(TEXT("FPointLightGSBuffer"), 0, EShaderStage::Geometry);
    BufferManager->BindConstantBuffer(TEXT("FPointLightShadowConstantBuffer"), 5, EShaderStage::Pixel);

    UpdateViewport(ShadowMapWidth, ShadowMapHeight);
    Graphics->DeviceContext->RSSetState(Graphics->RasterizerShadow);
    Graphics->DeviceContext->RSSetViewports(1, &ShadowViewport);
}

void FShadowRenderPass::UpdateCubeMapConstantBuffer(UPointLightComponent*& PointLight,
    const FMatrix& WorldMatrix
    ) const
{
    FPointLightGSBuffer DepthCubeMapBuffer;
    DepthCubeMapBuffer.World = WorldMatrix;
    for (uint32 i = 0; i < 6; ++i)
    {
        DepthCubeMapBuffer.ViewProj[i] = PointLight->GetViewMatrix(i) * PointLight->GetProjectionMatrix();
    }
    BufferManager->UpdateConstantBuffer(TEXT("FPointLightGSBuffer"), DepthCubeMapBuffer);
}

void FShadowRenderPass::RenderCubeMap(const std::shared_ptr<FEditorViewportClient>& Viewport, UPointLightComponent*& PointLight)
{
    //UpdateCubeMapConstantBuffer(PointLight);
    PrepareCubeMapRenderState(Viewport, PointLight);
}
