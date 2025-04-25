
#pragma once
#include "PrimitiveComponent.h"

class UShapeComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UShapeComponent, UPrimitiveComponent)    

public:
    UShapeComponent();
    
    FColor ShapeColor;
    bool bDrawOnlyIfSelected;
};
