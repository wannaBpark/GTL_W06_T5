#pragma once

#include "ShapeComponent.h"

struct FBox;

class UBoxComponent : public UShapeComponent
{
    DECLARE_CLASS(UBoxComponent, UShapeComponent)

public:
    UBoxComponent() = default;
    
    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

   
public:
    virtual bool CheckOverlap(const UPrimitiveComponent* Other) const override;

    FBox GetWorldAABB() const;
  
private:
    FVector BoxExtent;

};

