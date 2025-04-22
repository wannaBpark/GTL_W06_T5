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

    virtual void UpdateViewMatrix();
    virtual void UpdateProjectionMatrix();
    
    FMatrix GetViewMatrix(int Index = 0) const
    {
        return ViewMatrices[Index];
    }
    FMatrix GetProjectionMatrix() const
    {
        return ProjectionMatrix;
    }
    
protected:

    TArray<FMatrix>		ViewMatrices;
    FMatrix		ProjectionMatrix;
    
    FBoundingBox AABB;

public:
    FBoundingBox GetBoundingBox() const {return AABB;}

public:
    // bool HasShadowMap() const { return ShadowMaps.Num() != 0; }
    //virtual TArray<FDepthStencilRHI> GetShadowMap();
    void SetShadowMapSize(const uint32 InWidth, const uint32 InHeight);
    uint32 GetShadowMapWidth() const { return ShadowMapWidth; }
    uint32 GetShadowMapHeight() const { return ShadowMapHeight; }

protected:
    uint32 ShadowMapWidth = 4096;
    uint32 ShadowMapHeight = 4096;
    bool bDirtyFlag = false;
};
