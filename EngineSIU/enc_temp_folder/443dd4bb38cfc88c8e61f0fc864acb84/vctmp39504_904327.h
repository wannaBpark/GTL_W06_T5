#pragma once

#include "ShapeComponent.h"

struct FBox;

class UBoxComponent : public UShapeComponent
{
    DECLARE_CLASS(UBoxComponent, UShapeComponent)

public:
    UBoxComponent();
    
    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

<<<<<<< HEAD
   
public:
    virtual bool CheckOverlap(const UPrimitiveComponent* Other) const override;

    FBox GetWorldAABB() const;
  
=======
    FVector GetBoxExtent() const { return BoxExtent; }
    void SetBoxExtent(FVector InExtent) { BoxExtent = InExtent; }
    
>>>>>>> origin/feature/Add-Collision-Gizmo
private:
    FVector BoxExtent = FVector::ZeroVector;
};

