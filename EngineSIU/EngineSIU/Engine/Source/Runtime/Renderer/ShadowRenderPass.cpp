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
    //ShadowMapWidth = LightShadow->GetShadowWidth();
    //ShadowMapHeight = LightShadow->GetShadowHeight();
    PrepareRenderState();
    


    Graphics->DeviceContext->RSSetViewports(0, nullptr);
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void FShadowRenderPass::ClearRenderArr()
{
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
    D3D11_VIEWPORT shadowViewport;
    ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
    shadowViewport.TopLeftX = 0;
    shadowViewport.TopLeftY = 0;
    shadowViewport.Width = static_cast<float>(InWidth);
    shadowViewport.Height = static_cast<float>(InHeight);
    shadowViewport.MinDepth = 0.0f;
    shadowViewport.MaxDepth = 1.0f;
}
