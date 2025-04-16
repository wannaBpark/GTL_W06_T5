#pragma once
#include "IRenderPass.h"
#include "LevelEditor/SlateAppMessageHandler.h"

class FPostProcessCompositingPass : public IRenderPass
{
public:
    FPostProcessCompositingPass();
    virtual ~FPostProcessCompositingPass();
    
    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage) override;
    
    virtual void PrepareRender() override;

    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;

    virtual void ClearRenderArr() override;
    
private:
    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;

    ID3D11SamplerState* Sampler;
};
