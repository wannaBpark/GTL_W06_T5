#include "ShadowRenderPass.h"

#include "ShadowManager.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "Components/Light/LightComponent.h"
#include "Components/Light/PointLightComponent.h"
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
    DepthOnlyPS = ShaderManager->GetPixelShaderByKey(L"DepthOnlyPS");
    
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


void FShadowRenderPass::Render(ULightComponentBase* Light)
{
    
}

void FShadowRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{

    PrepareRenderState();
    for (const auto DirectionalLight : TObjectRange<UDirectionalLightComponent>())
       {
        // Cascade Shadow Map을 위한 ViewProjection Matrix 설정
            ShadowManager->UpdateCascadeMatrices(Viewport, DirectionalLight);
            
            FShadowConstantBuffer ShadowData;
            FMatrix LightViewMatrix = DirectionalLight->GetViewMatrix();
            FMatrix LightProjectionMatrix = DirectionalLight->GetProjectionMatrix();
            ShadowData.ShadowViewProj = LightViewMatrix * LightProjectionMatrix;
            ShadowData.ShadowInvProj = FMatrix::Inverse(LightProjectionMatrix);
            ShadowData.LightNearZ = DirectionalLight->GetShadowNearPlane();
            ShadowData.LightFrustumWidth = DirectionalLight->GetShadowFrustumWidth();
                        
            ShadowData.ShadowMapWidth = DirectionalLight->GetShadowMapWidth();
            ShadowData.ShadowMapHeight = DirectionalLight->GetShadowMapHeight();
            BufferManager->UpdateConstantBuffer(TEXT("FShadowConstantBuffer"), ShadowData);
    
            ShadowManager->BeginDirectionalShadowCascadePass(0);
            RenderAllStaticMeshes(Viewport);
           
            Graphics->DeviceContext->RSSetViewports(0, nullptr);
            Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
       }

    for (const auto & PonintLight : PointLights)
    {
        Render(PonintLight);
        RenderAllStaticMeshes(Viewport);
           
        Graphics->DeviceContext->RSSetViewports(0, nullptr);
        Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    }
    for (int i = 0 ; i < SpotLights.Num(); i++)
    {
        const auto& SpotLight = SpotLights[i];
        FShadowConstantBuffer ShadowData;
        FMatrix LightViewMatrix = SpotLight->GetViewMatrix();
        FMatrix LightProjectionMatrix = SpotLight->GetProjectionMatrix();
        ShadowData.ShadowViewProj = LightViewMatrix * LightProjectionMatrix;
        ShadowData.ShadowInvProj = FMatrix::Inverse(LightProjectionMatrix);
                    
        ShadowData.ShadowMapWidth = SpotLight->GetShadowMapWidth();
        ShadowData.ShadowMapHeight = SpotLight->GetShadowMapHeight();
        BufferManager->UpdateConstantBuffer(TEXT("FShadowConstantBuffer"), ShadowData);

        ShadowManager->BeginSpotShadowPass(i);
        RenderAllStaticMeshes(Viewport);
           
        Graphics->DeviceContext->RSSetViewports(0, nullptr);
        Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    }
}


void FShadowRenderPass::ClearRenderArr()
{
    StaticMeshComponents.Empty();
}

void FShadowRenderPass::SetLightData(const TArray<class UPointLightComponent*>& InPointLights, const TArray<class USpotLightComponent*>& InSpotLights)
{
    PointLights = InPointLights;
    SpotLights = InSpotLights;
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
   // Graphics->DeviceContext->GSSetShader(nullptr, nullptr, 0);
    //Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
    //Graphics->DeviceContext->VSSetShader(nullptr, nullptr, 0);
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

    hr = ShaderManager->AddVertexShader(L"CascadedShadowMapVS", L"Shaders/CascadedShadowMap.hlsl", "mainVS");
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Cascaded ShadowMap Vertex shader!"));
    }

    hr = ShaderManager->AddGeometryShader(L"CascadedShadowMapGS", L"Shaders/CascadedShadowMap.hlsl", "mainGS");
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Cascaded ShadowMap Geometry shader!"));
    }

    hr = ShaderManager->AddPixelShader(L"CascadedShadowMapPS", L"Shaders/CascadedShadowMap.hlsl", "mainPS");
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to create Cascaded ShadowMap Pixel shader!"));
    }
    


    StaticMeshIL = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
    DepthOnlyVS = ShaderManager->GetVertexShaderByKey(L"DepthOnlyVS");
    DepthOnlyPS = ShaderManager->GetPixelShaderByKey(L"DepthOnlyPS");

    CascadedShadowMapVS = ShaderManager->GetVertexShaderByKey(L"CascadedShadowMapVS");
    CascadedShadowMapGS = ShaderManager->GetGeometryShaderByKey(L"CascadedShadowMapGS");
    CascadedShadowMapPS = ShaderManager->GetPixelShaderByKey(L"CascadedShadowMapPS");
}


//void FShadowRenderPass::PrepareCubeMapRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport, UPointLightComponent*& PointLight)
//{
//    /*auto*& DSV = Viewport->GetViewportResource()->GetDepthStencil(EResourceType::ERT_Scene)->DSV;*/
//    auto sm = PointLight->GetShadowMap();
//    auto*& DSV = sm[1].DSV;
//    Graphics->DeviceContext->ClearDepthStencilView(DSV,
//        D3D11_CLEAR_DEPTH, 1.0f, 0);
//    Graphics->DeviceContext->ClearRenderTargetView(PointLight->DepthRTVArray, ClearColor);
//    Graphics->DeviceContext->OMSetRenderTargets(1, &PointLight->DepthRTVArray, DSV);
//    Graphics->DeviceContext->IASetInputLayout(StaticMeshIL);
//
//    DepthCubeMapVS = ShaderManager->GetVertexShaderByKey(L"DepthCubeMapVS");
//    DepthCubeMapGS = ShaderManager->GetGeometryShaderByKey(L"DepthCubeMapGS");
//    DepthOnlyPS = ShaderManager->GetPixelShaderByKey(L"DepthOnlyPS");
//
//    Graphics->DeviceContext->VSSetShader(DepthCubeMapVS, nullptr, 0);
//    Graphics->DeviceContext->GSSetShader(DepthCubeMapGS, nullptr, 0);
//    Graphics->DeviceContext->PSSetShader(DepthOnlyPS, nullptr, 0);
//    // VS, GS에 대한 상수버퍼 업데이트
//    BufferManager->BindConstantBuffer(TEXT("FPointLightGSBuffer"), 0, EShaderStage::Geometry);
//
//    UpdateViewport(ShadowMapWidth, ShadowMapHeight);
//    Graphics->DeviceContext->RSSetState(Graphics->RasterizerSolidBack);
//    Graphics->DeviceContext->RSSetViewports(1, &ShadowViewport);
//}
//
//void FShadowRenderPass::UpdateCubeMapConstantBuffer(UPointLightComponent*& PointLight,
//    const FMatrix& WorldMatrix
//    ) const
//{
//    FPointLightGSBuffer DepthCubeMapBuffer;
//    DepthCubeMapBuffer.World = WorldMatrix;
//    for (uint32 i = 0; i < 6; ++i)
//    {
//        DepthCubeMapBuffer.ViewProj[i] = PointLight->GetViewMatrix(i) * PointLight->GetProjectionMatrix();
//    }
//    BufferManager->UpdateConstantBuffer(TEXT("FPointLightGSBuffer"), DepthCubeMapBuffer);
//}
//
//void FShadowRenderPass::RenderCubeMap(const std::shared_ptr<FEditorViewportClient>& Viewport, UPointLightComponent*& PointLight)
//{
//    //UpdateCubeMapConstantBuffer(PointLight);
//    PrepareCubeMapRenderState(Viewport, PointLight);
//
//}
//
//
