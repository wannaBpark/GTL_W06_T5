#pragma once
#include "UnrealClient.h"
#include "Components/SceneComponent.h"


class ULightComponentBase : public USceneComponent
{
    DECLARE_CLASS(ULightComponentBase, USceneComponent)

public:
    ULightComponentBase();
    virtual ~ULightComponentBase() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
protected:

    FBoundingBox AABB;

public:
    FBoundingBox GetBoundingBox() const {return AABB;}

public:
    HRESULT CreateShadowMap();
    void ReleaseShadowMap();
    void ClearShadowMap(ID3D11DeviceContext* DeviceContext);

    bool HasShadowMap() const { return ShadowMaps.Num() != 0; }
    virtual TArray<FDepthStencilRHI> GetShadowMap();
    void SetShadowMapSize(const uint32 InWidth, const uint32 InHeight);
    uint32 GetShadowMapWidth() const { return ShadowMapWidth; }
    uint32 GetShadowMapHeight() const { return ShadowMapHeight; }

protected:
    TArray<FDepthStencilRHI> ShadowMaps;
    uint32 ShadowMapWidth = 2048;
    uint32 ShadowMapHeight = 2048;
    bool bDirtyFlag = false;
};
