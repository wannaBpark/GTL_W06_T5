#pragma once
#include "Components/SceneComponent.h"
#include "Engine/OverlapInfo.h"

class UPrimitiveComponent : public USceneComponent
{
    DECLARE_CLASS(UPrimitiveComponent, USceneComponent)

public:
    UPrimitiveComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    
    bool IntersectRayTriangle(
        const FVector& RayOrigin, const FVector& RayDirection,
        const FVector& v0, const FVector& v1, const FVector& v2, float& OutHitDistance
    ) const;

    
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;


    FBoundingBox AABB;

    bool IsOverlappingActor(const AActor* Other) const;

    bool bGenerateOverlapEvents = true;
    bool bBlockComponent = true;

    const TArray<FOverlapInfo>& GetOverlapInfos() const;

protected:
    TArray<FOverlapInfo> OverlappingComponents;

    virtual void UpdateOverlapsImpl(const TArray<FOverlapInfo>* PendingOverlaps) override;

private:
    FString m_Type;

public:
    FString GetType() { return m_Type; }

    void SetType(const FString& _Type)
    {
        m_Type = _Type;
        //staticMesh = FEngineLoop::resourceMgr.GetMesh(m_Type);
    }
    
    FBoundingBox GetBoundingBox() const { return AABB; }
};

