#pragma once
#include "Components/SceneComponent.h"
#include "OverlapInfo.h"

class UPrimitiveComponent : public USceneComponent
{
    DECLARE_CLASS(UPrimitiveComponent, USceneComponent)

public:
    UPrimitiveComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;

    bool IntersectRayTriangle(const FVector& rayOrigin, const FVector& rayDirection,
        const FVector& v0, const FVector& v1, const FVector& v2, float& hitDistance) const;

    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;

    FBoundingBox AABB;

protected:
    FString m_Type;
    TArray<FOverlapInfo> OverlapInfos;
    TArray<FOverlapInfo> PreviousOverlapInfos;

public:
    FString GetType() { return m_Type; }
    void SetType(const FString& _Type) { m_Type = _Type; }
    FBoundingBox GetBoundingBox() const { return AABB; }

    const TArray<FOverlapInfo>& GetOverlapInfos() const { return OverlapInfos; }
    const TArray<FOverlapInfo>& GetPreviousOverlapInfos() const { return PreviousOverlapInfos; }

    bool IsOverlappingActor(const AActor* Other) const;
    void UpdateOverlaps();
    virtual bool CheckOverlap(const UPrimitiveComponent* Other) const { return false; }
};
