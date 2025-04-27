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
    
    for (const auto iter : TObjectRange<UPrimitiveComponent>())
    {
        if (!iter || iter->GetWorld() != World || iter == Component)
        {
            continue;            
        }
        
        if (iter->AABB.IsValidBox() && bComponentHasValidBox)
        {
            if (FBoundingBox::CheckOverlap(Component->AABB, iter->AABB))
            {
                // 
            }
        }
        else
        {
            
        }
    }
}
