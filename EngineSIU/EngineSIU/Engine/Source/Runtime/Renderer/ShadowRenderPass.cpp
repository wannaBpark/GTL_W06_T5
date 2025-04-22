#include "ShadowRenderPass.h"

#include "ShadowManager.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "Components/Light/LightComponent.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/Casts.h"
#include "UObject/UObjectIterator.h"

class UEditorEngine;
class UStaticMeshComponent;

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
}

void FShadowRenderPass::InitializeShadowManager(class FShadowManager* InShadowManager)
{
    ShadowManager = InShadowManager;
}


//한번만 실행하면 되는 것
void FShadowRenderPass::PrepareRenderState()
{
    // Shader Hot Reload 대응 
    StaticMeshIL = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
    DepthOnlyVS = ShaderManager->GetVertexShaderByKey(L"DepthOnlyVS");
    Graphics->DeviceContext->IASetInputLayout(StaticMeshIL);
    Graphics->DeviceContext->VSSetShader(DepthOnlyVS, nullptr, 0);

    // Note : PS만 언바인드할 뿐, UpdateLightBuffer에서 바인딩된 SRV 슬롯들은 그대로 남아 있음
    Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
    Graphics->DeviceContext->RSSetState(Graphics->RasterizerShadow);
    
    BufferManager->BindConstantBuffer(TEXT("FShadowConstantBuffer"), 11, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(TEXT("FShadowConstantBuffer"), 11, EShaderStage::Pixel);
    
}

void FShadowRenderPass::PrepareRenderArr()
{
    for (const auto iter : TObjectRange<UStaticMeshComponent>())
    {
        if (!Cast<UGizmoBaseComponent>(iter) && iter->GetWorld() == GEngine->ActiveWorld)
        {
            StaticMeshComponents.Add(iter);
        }
    }
}


void FShadowRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport, ULightComponentBase* Light)
{
    if (UDirectionalLightComponent* DirectionalLight = Cast<UDirectionalLightComponent>(Light))
    {

        FShadowConstantBuffer ShadowData;
        DirectionalLight->UpdateViewMatrix(Viewport->GetCameraLocation());
        DirectionalLight->UpdateProjectionMatrix();
        FMatrix LightViewMatrix = DirectionalLight->GetViewMatrix();
        FMatrix LightProjectionMatrix = DirectionalLight->GetProjectionMatrix();
        ShadowData.ShadowViewProj = LightViewMatrix * LightProjectionMatrix;
        ShadowData.ShadowInvProj = FMatrix::Inverse(LightProjectionMatrix);
        ShadowData.LightNearZ = DirectionalLight->GetShadowNearPlane();
        ShadowData.LightFrustumWidth = DirectionalLight->GetShadowFrustumWidth();
        /*ShadowData.AreaLightRadius = DirectionalLight->GetRadius();*/
                    
                    
        // FMatrix ViewMatrix = JungleMath::CreateViewMatrix(-DirectionalLight->GetDirection() * 40, FVector(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 1.0f));
        // FMatrix ProjectionMatrix = JungleMath::CreateOrthoProjectionMatrix(80.0f, 80.0f, 1.0f, 100.0f);
        //
        // ShadowData.ShadowViewProj = ViewMatrix * ProjectionMatrix;
        // ShadowData.ShadowInvProj = FMatrix::Inverse(ProjectionMatrix);
                    
        ShadowData.ShadowMapWidth = DirectionalLight->GetShadowMapWidth();
        ShadowData.ShadowMapHeight = DirectionalLight->GetShadowMapHeight();
        BufferManager->UpdateConstantBuffer(TEXT("FShadowConstantBuffer"), ShadowData);

        ShadowManager->BeginDirectionalShadowCascadePass(0);
    }
}

void FShadowRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{

    PrepareRenderState();
    for (const auto iter : TObjectRange<ULightComponentBase>())
       {
           if (iter->GetWorld() == GEngine->ActiveWorld)
           {
               if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(iter))
               {
                   
               }
               else if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(iter))
               {
               }
               else if (UDirectionalLightComponent* DirectionalLight = Cast<UDirectionalLightComponent>(iter))
               {  
                   Render(Viewport, DirectionalLight);
                   RenderAllStaticMeshes(Viewport);
               }
           }
           
        Graphics->DeviceContext->RSSetViewports(0, nullptr);
        Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
       }

    Graphics->DeviceContext->RSSetViewports(0, nullptr);
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}


void FShadowRenderPass::ClearRenderArr()
{
    StaticMeshComponents.Empty();
}

void FShadowRenderPass::RenderPrimitive(OBJ::FStaticMeshRenderData* RenderData, const TArray<FStaticMaterial*> Materials, TArray<UMaterial*> OverrideMaterials,
    int SelectedSubMeshIndex)
{
    UINT Stride = sizeof(FStaticMeshVertex);
    UINT Offset = 0;
    
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &RenderData->VertexBuffer, &Stride, &Offset);

    if (RenderData->IndexBuffer)
    {
        Graphics->DeviceContext->IASetIndexBuffer(RenderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    }

    if (RenderData->MaterialSubsets.Num() == 0)
    {
        Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
        return;
    }

    for (int SubMeshIndex = 0; SubMeshIndex < RenderData->MaterialSubsets.Num(); SubMeshIndex++)
    {
        uint32 MaterialIndex = RenderData->MaterialSubsets[SubMeshIndex].MaterialIndex;

        FSubMeshConstants SubMeshData = (SubMeshIndex == SelectedSubMeshIndex) ? FSubMeshConstants(true) : FSubMeshConstants(false);

        BufferManager->UpdateConstantBuffer(TEXT("FSubMeshConstants"), SubMeshData);

        if (OverrideMaterials[MaterialIndex] != nullptr)
        {
            MaterialUtils::UpdateMaterial(BufferManager, Graphics, OverrideMaterials[MaterialIndex]->GetMaterialInfo());
        }
        else
        {
            MaterialUtils::UpdateMaterial(BufferManager, Graphics, Materials[MaterialIndex]->Material->GetMaterialInfo());
        }

        uint32 StartIndex = RenderData->MaterialSubsets[SubMeshIndex].IndexStart;
        uint32 IndexCount = RenderData->MaterialSubsets[SubMeshIndex].IndexCount;
        Graphics->DeviceContext->DrawIndexed(IndexCount, StartIndex, 0);
    }
}

void FShadowRenderPass::RenderAllStaticMeshes(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    for (UStaticMeshComponent* Comp : StaticMeshComponents)
    {
        if (!Comp || !Comp->GetStaticMesh())
        {
            continue;
        }

        OBJ::FStaticMeshRenderData* RenderData = Comp->GetStaticMesh()->GetRenderData();
        if (RenderData == nullptr)
        {
            continue;
        }

        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

        FMatrix WorldMatrix = Comp->GetWorldMatrix();
        FVector4 UUIDColor = Comp->EncodeUUID() / 255.0f;
        const bool bIsSelected = (Engine && Engine->GetSelectedActor() == Comp->GetOwner());

        UpdateObjectConstant(WorldMatrix, UUIDColor, bIsSelected);

        RenderPrimitive(RenderData, Comp->GetStaticMesh()->GetMaterials(), Comp->GetOverrideMaterials(), Comp->GetselectedSubMeshIndex());
        
    }
}

void FShadowRenderPass::BindResourcesForSampling()
{
    ShadowManager->BindResourcesForSampling(static_cast<UINT>(EShaderSRVSlot::SRV_SpotLight),
        static_cast<UINT>(EShaderSRVSlot::SRV_DirectionalLight),
    10);
}

void FShadowRenderPass::UpdateObjectConstant(const FMatrix& WorldMatrix, const FVector4& UUIDColor, bool bIsSelected) const
{
    FObjectConstantBuffer ObjectData = {};
    ObjectData.WorldMatrix = WorldMatrix;
    ObjectData.InverseTransposedWorld = FMatrix::Transpose(FMatrix::Inverse(WorldMatrix));
    ObjectData.UUIDColor = UUIDColor;
    ObjectData.bIsSelected = bIsSelected;
    
    BufferManager->UpdateConstantBuffer(TEXT("FObjectConstantBuffer"), ObjectData);
}

void FShadowRenderPass::CreateShader() const
{
    HRESULT hr = ShaderManager->AddVertexShader(L"DepthOnlyVS", L"Shaders/DepthOnlyVS.hlsl", "mainVS");
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create DepthOnlyVS shader!"));
    }
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

    D3D11_SAMPLER_DESC PointSamplerDesc = {};
    PointSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    PointSamplerDesc.MinLOD = 0;
    PointSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    PointSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    PointSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    PointSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    PointSamplerDesc.BorderColor[0] = 1.0f;                     // 큰 Z값
    PointSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

    hr = Graphics->Device->CreateSamplerState(&SamplerDesc, &ShadowPointSampler);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Shadow Point Sampler!"));
    }
}
