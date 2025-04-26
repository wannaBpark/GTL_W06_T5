
#pragma once
#include "ShapeComponent.h"
#include "Math/Vector.h"

class UBoxComponent : public UShapeComponent
{
    DECLARE_CLASS(UBoxComponent, UShapeComponent)

public:
    UBoxComponent();
    
    FVector BoxExtent;
};
