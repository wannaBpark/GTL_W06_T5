#pragma once

#include "ShapeComponent.h"


struct FCapsule;

class UCapsuleComponent : public UShapeComponent
{
    DECLARE_CLASS(UCapsuleComponent, UShapeComponent)

public:
    UCapsuleComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
public:
    virtual bool CheckOverlap(const UPrimitiveComponent* Other) const override;

    FVector GetStartPoint() const
    {
        return GetWorldLocation() + FVector(0, 0, CapsuleHalfHeight);
    }
    FVector GetEndPoint() const
    {
        return GetWorldLocation() - FVector(0, 0, CapsuleHalfHeight);
    }
    float GetCapsuleRadius() const
    {
        return CapsuleRadius;
    }
    FCapsule ToFCapsule() const;

private:
    float CapsuleHalfHeight;
    float CapsuleRadius;


};

