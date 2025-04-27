#pragma once
#include "Components/PrimitiveComponent.h"

struct FOverlapResult;

class FCollisionManager
{
public:
    FCollisionManager() = default;
    ~FCollisionManager() = default;

    void CheckOverlap(const UWorld* World, const UPrimitiveComponent* Component, TArray<FOverlapResult>& OutOverlaps) const;

protected:
    bool IsOverlapped(const UPrimitiveComponent* Component, const UPrimitiveComponent* OtherComponent, FOverlapResult& OutResult) const;
};
