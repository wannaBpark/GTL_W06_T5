#include "ShadowRenderPass.h"

#include "Components/Light/LightComponent.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

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

void FShadowRenderPass::PrepareRenderState()
{
    // Shader Hot Reload 대응 
    StaticMeshIL = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
    DepthOnlyVS = ShaderManager->GetVertexShaderByKey(L"DepthOnlyVS");
    Graphics->DeviceContext->IASetInputLayout(StaticMeshIL);
    Graphics->DeviceContext->VSSetShader(DepthOnlyVS, nullptr, 0);

    // Note : PS만 언바인드할 뿐, UpdateLightBuffer에서 바인딩된 SRV 슬롯들은 그대로 남아 있음
    Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
    Graphics->DeviceContext->RSSetState(Graphics->RasterizerSolidBack);
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
    ShadowMapWidth = Light->GetShadowMapWidth();
    ShadowMapHeight = Light->GetShadowMapHeight();
    Graphics->DeviceContext->ClearDepthStencilView(Light->GetShadowMap()[0].DSV, 
        D3D11_CLEAR_DEPTH, 1.0f, 0);
    PrepareRenderState();
    BufferManager->BindConstantBuffer(TEXT("FShadowConstantBuffer"), 11, EShaderStage::Vertex);
    Graphics->DeviceContext->RSSetViewports(1, &ShadowViewport);
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, Light->GetShadowMap()[0].DSV);
}

void FShadowRenderPass::ClearRenderArr()
{
    Graphics->DeviceContext->RSSetViewports(0, nullptr);
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void FShadowRenderPass::CreateShader() const
{
    HRESULT hr = ShaderManager->AddVertexShader(L"DepthOnlyVS", L"Shaders/DepthOnlyVS.hlsl", "mainVS");
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create DepthOnlyVS shader!"));
    }
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
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
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
