#include "CollisionManager.h"

#include "Components/PrimitiveComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

#include "Engine/OverlapResult.h"
#include "UObject/Casts.h"
#include "UObject/UObjectIterator.h"

FCollisionManager::FCollisionManager()
{
    for (size_t i = 0; i <= NUM_TYPES; ++i)
    {
        for (size_t j = 0; j <= NUM_TYPES; ++j)
        {
            CollisionMatrix[i][j] = &FCollisionManager::Check_NotImplemented;
        }
    }

    CollisionMatrix[static_cast<size_t>(EShapeType::Box)][static_cast<size_t>(EShapeType::Box)] = &FCollisionManager::Check_Box_Box;
    CollisionMatrix[static_cast<size_t>(EShapeType::Box)][static_cast<size_t>(EShapeType::Sphere)] = &FCollisionManager::Check_Box_Sphere;
    CollisionMatrix[static_cast<size_t>(EShapeType::Box)][static_cast<size_t>(EShapeType::Capsule)] = &FCollisionManager::Check_Box_Capsule;
    CollisionMatrix[static_cast<size_t>(EShapeType::Sphere)][static_cast<size_t>(EShapeType::Box)] = &FCollisionManager::Check_Sphere_Box;
    CollisionMatrix[static_cast<size_t>(EShapeType::Sphere)][static_cast<size_t>(EShapeType::Sphere)] = &FCollisionManager::Check_Sphere_Sphere;
    CollisionMatrix[static_cast<size_t>(EShapeType::Sphere)][static_cast<size_t>(EShapeType::Capsule)] = &FCollisionManager::Check_Sphere_Capsule;
    CollisionMatrix[static_cast<size_t>(EShapeType::Capsule)][static_cast<size_t>(EShapeType::Box)] = &FCollisionManager::Check_Capsule_Box;
    CollisionMatrix[static_cast<size_t>(EShapeType::Capsule)][static_cast<size_t>(EShapeType::Sphere)] = &FCollisionManager::Check_Capsule_Sphere;
    CollisionMatrix[static_cast<size_t>(EShapeType::Capsule)][static_cast<size_t>(EShapeType::Capsule)] = &FCollisionManager::Check_Capsule_Capsule;
}

void FCollisionManager::CheckOverlap(const UWorld* World, const UPrimitiveComponent* Component, TArray<FOverlapResult>& OutOverlaps) const
{
    OutOverlaps.Empty();
    
    if (!Component || !Component->IsA<UShapeComponent>())
    {
        return;
    }

    const bool bComponentHasValidBox = Component->AABB.IsValidBox();
    
    for (const auto Iter : TObjectRange<UShapeComponent>())
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

    const UShapeComponent* ShapeA = Cast<UShapeComponent>(Component);
    const UShapeComponent* ShapeB = Cast<UShapeComponent>(OtherComponent);

    const SIZE_T ShapeTypeA = static_cast<SIZE_T>(ShapeA->GetShapeType());
    const SIZE_T ShapeTypeB = static_cast<SIZE_T>(ShapeB->GetShapeType());

    return (this->*CollisionMatrix[ShapeTypeA][ShapeTypeB])(ShapeA, ShapeB, OutResult);
}

bool FCollisionManager::Check_NotImplemented(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    return false;
}

bool FCollisionManager::Check_Box_Box(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    return false;
}

bool FCollisionManager::Check_Box_Sphere(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    return false;
}

bool FCollisionManager::Check_Box_Capsule(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    return false;
}

bool FCollisionManager::Check_Sphere_Box(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    return Check_Box_Sphere(B, A, OutResult);
}

bool FCollisionManager::Check_Sphere_Sphere(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    return false;
}

bool FCollisionManager::Check_Sphere_Capsule(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    return false;
}

bool FCollisionManager::Check_Capsule_Box(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    return Check_Box_Capsule(B, A, OutResult);   
}

bool FCollisionManager::Check_Capsule_Sphere(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    return Check_Sphere_Capsule(B, A, OutResult);  
}

bool FCollisionManager::Check_Capsule_Capsule(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    return false;
}
