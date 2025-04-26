
#pragma once
#include "PrimitiveComponent.h"

class UShapeComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UShapeComponent, UPrimitiveComponent)    

public:
    UShapeComponent();
    
    FColor ShapeColor = FColor(180, 180, 180, 255);
    bool bDrawOnlyIfSelected = true;
};
