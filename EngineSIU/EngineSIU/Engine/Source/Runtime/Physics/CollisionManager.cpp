#include "CollisionManager.h"

#include "Components/PrimitiveComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

#include "Engine/OverlapResult.h"
#include "UObject/UObjectIterator.h"

void FCollisionManager::CheckOverlap(const UWorld* World, const UPrimitiveComponent* Component, TArray<FOverlapResult>& OutOverlaps) const
{
    OutOverlaps.Empty();
    
    if (!Component)
    {
        return;
    }

    const bool bComponentHasValidBox = Component->AABB.IsValidBox();
    
    for (const auto Iter : TObjectRange<UPrimitiveComponent>())
    {
        if (!Iter || Iter->GetWorld() != World || Iter == Component)
        {
            continue;            
        }

        bool bCanSkip = true;
        
        if (Iter->AABB.IsValidBox() && bComponentHasValidBox)
        {
            if (FBoundingBox::CheckOverlap(Component->AABB, Iter->AABB))
            {
                bCanSkip = false;
            }
        }
        else
        {
            bCanSkip = false;
        }

        if (!bCanSkip)
        {
            FOverlapResult OverlapResult;
            if (IsOverlapped(Component, Iter, OverlapResult))
            {
                OutOverlaps.Add(OverlapResult);
            }
        }
    }
}

bool FCollisionManager::IsOverlapped(const UPrimitiveComponent* Component, const UPrimitiveComponent* OtherComponent, FOverlapResult& OutResult) const
{
    if (!Component || !OtherComponent)
    {
        return false;
    }
    if (!Component->IsA<UShapeComponent>() || !OtherComponent->IsA<UShapeComponent>())
    {
        return false;
    }

    return false;
}
