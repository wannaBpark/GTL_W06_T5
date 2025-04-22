#pragma once
#include "IRenderPass.h"
#include "EngineBaseTypes.h"
#include "Container/Set.h"
#include "Define.h"
#include "UnrealClient.h" // Depth Stencil View
#include <d3d11.h>


// ShadowMap을 생성하기 위한 Render Pass입니다.
// Depth 값만을 추출하는 Vertex Shader Only 패스
// 모든 Light에 대한 Depth Map Texture를 생성합니다.
// ViewMode != Unlit일 때에만 Static Mesh Render Pass 에서 실행됩니다

class FDXDBufferManager;
class FDXDShaderManager;
class FGraphicsDevice;
class ULightComponentBase;

class FShadowRenderPass : public IRenderPass
{
public:
    FShadowRenderPass();
    virtual ~FShadowRenderPass() override;
    void CreateShader() const;
    void CreateSampler();
    ID3D11SamplerState* GetSampler() const { return Sampler; }
    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager);
    void InitializeShadowManager(class FShadowManager* InShadowManager);
    void PrepareRenderState();
    virtual void PrepareRenderArr() override;
    void Render(ULightComponentBase* Light);
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;    
    virtual void ClearRenderArr() override;

    void RenderPrimitive(OBJ::FStaticMeshRenderData* render_data, const TArray<FStaticMaterial*> array, TArray<UMaterial*> materials, int getselected_sub_mesh_index);
    virtual void RenderAllStaticMeshes(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void BindResourcesForSampling();

    void UpdateObjectConstant(const FMatrix& WorldMatrix, const FVector4& UUIDColor, bool bIsSelected) const;


private:

    
    TArray<class UStaticMeshComponent*> StaticMeshComponents;
    
    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;
    FShadowManager* ShadowManager;

    ID3D11InputLayout* StaticMeshIL;
    ID3D11VertexShader* DepthOnlyVS;
    ID3D11SamplerState* Sampler;
};
