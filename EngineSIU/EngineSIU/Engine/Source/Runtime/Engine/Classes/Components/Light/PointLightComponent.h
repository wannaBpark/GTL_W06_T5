#pragma once
#include "LightComponent.h"

class UPointLightComponent :public ULightComponentBase
{

    DECLARE_CLASS(UPointLightComponent, ULightComponentBase)
public:
    UPointLightComponent();
    virtual ~UPointLightComponent() override;

    void Initialize();
    virtual HRESULT CreateShadowMap() override;
    void InitShadowDebugView();

    virtual UObject* Duplicate(UObject* InOuter) override;
    
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    const FPointLightInfo& GetPointLightInfo() const;
    void SetPointLightInfo(const FPointLightInfo& InPointLightInfo);

    float GetRadius() const;
    void SetRadius(float InRadius);

    FLinearColor GetLightColor() const;
    void SetLightColor(const FLinearColor& InColor);


    float GetIntensity() const;
    void SetIntensity(float InIntensity);

    int GetType() const;
    void SetType(int InType);

    
    void UpdateViewMatrix() override;
    void UpdateProjectionMatrix() override;

    
private:
    FPointLightInfo PointLightInfo;

    TArray<ID3D11Texture2D*> OutputTextures = {};
    TArray<ID3D11ShaderResourceView*> OutputSRVs = {};
    ID3D11ShaderResourceView* SliceSRVs[6] = { nullptr };

public:
    TArray<FDepthStencilRHI> GetShadowMap() override;
    ID3D11RenderTargetView* DepthRTVArray;
    ID3D11ShaderResourceView* GetSliceSRV(int SliceIndex) const;
    ID3D11ShaderResourceView* CreateSliceSRV(ID3D11Texture2D* texArray, DXGI_FORMAT format, UINT sliceIndex);
};


