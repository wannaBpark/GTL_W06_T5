#include "BoxComponent.h"
#include "UObject/Casts.h"
#include "Math/CollisionMath.h"
#include "Math/ShapeInfo.h"
#include "Components/Shapes/SphereComponent.h"
#include "Components/Shapes//CapsuleComponent.h"

UBoxComponent::UBoxComponent()
{
    BoxExtent = FVector(0.5,0.5,0.5);
}

UObject* UBoxComponent::Duplicate(UObject* InOuter)
{
    UBoxComponent* NewComponent = Cast<UBoxComponent>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->BoxExtent = BoxExtent;
    }
    return NewComponent;
}

void UBoxComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("BoxExtent"), BoxExtent.ToString());
}

void UBoxComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("BoxExtent"));
    if (TempStr)
    {
        BoxExtent.InitFromString(*TempStr);
    }
}

bool UBoxComponent::CheckOverlap(const UPrimitiveComponent* Other) const
{
    if (const UBoxComponent* Box = Cast<UBoxComponent>(Other))
    {
        return FCollisionMath::IntersectBoxBox(this->GetWorldAABB(), Box->GetWorldAABB());
    }
    else if (const USphereComponent* Sphere = Cast<USphereComponent>(Other))
    {
        return FCollisionMath::IntersectBoxSphere(this->GetWorldAABB(), Sphere->GetWorldLocation(), Sphere->GetRadius());
    }
    else if (const UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(Other))
    {
        return FCollisionMath::IntersectBoxCapsule(this->GetWorldAABB(), Capsule->ToFCapsule());
    }
    return false;
}

FBox UBoxComponent::GetWorldAABB() const
{
    FVector Center = GetWorldLocation();
    return FBox{ Center - BoxExtent, Center + BoxExtent };
}


