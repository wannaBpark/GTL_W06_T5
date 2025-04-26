#include "PrimitiveComponent.h"

#include "UObject/Casts.h"


UObject* UPrimitiveComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->AABB = AABB;

    return NewComponent;
}

void UPrimitiveComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UPrimitiveComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
}

bool UPrimitiveComponent::IntersectRayTriangle(const FVector& RayOrigin, const FVector& RayDirection, const FVector& v0, const FVector& v1, const FVector& v2, float& OutHitDistance) const
{
    const FVector Edge1 = v1 - v0;
    const FVector Edge2 = v2 - v0;
    
    FVector FrayDirection = RayDirection;
    FVector h = FrayDirection.Cross(Edge2);
    float a = Edge1.Dot(h);

    if (fabs(a) < SMALL_NUMBER)
    {
        return false; // Ray와 삼각형이 평행한 경우
    }

    float f = 1.0f / a;
    FVector s = RayOrigin - v0;
    float u = f * s.Dot(h);
    if (u < 0.0f || u > 1.0f)
    {
        return false;
    }

    FVector q = s.Cross(Edge1);
    float v = f * FrayDirection.Dot(q);
    if (v < 0.0f || (u + v) > 1.0f)
    {
        return false;
    }

    float t = f * Edge2.Dot(q);
    if (t > SMALL_NUMBER)
    {
        OutHitDistance = t;
        return true;
    }

    return false;
}

void UPrimitiveComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("m_Type"), m_Type);
    OutProperties.Add(TEXT("AABB_min"), AABB.min.ToString());
    OutProperties.Add(TEXT("AABB_max"), AABB.max.ToString());
}


void UPrimitiveComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);

    const FString* TempStr = nullptr;

    // --- PrimitiveComponent 고유 속성 복원 ---

    TempStr = InProperties.Find(TEXT("m_Type"));
    if (TempStr)
    {
        this->m_Type = *TempStr; // m_Type이 FString이라고 가정
        // 만약 m_Type이 enum이라면 문자열로부터 enum 값을 파싱하는 로직 필요
    }

    const FString* AABBminStr = InProperties.Find(TEXT("AABB_min"));
    if (AABBminStr) AABB.min.InitFromString(*AABBminStr); 

    
    const FString* AABBmaxStr = InProperties.Find(TEXT("AABB_max"));
    if (AABBmaxStr) AABB.max.InitFromString(*AABBmaxStr); 
}

bool UPrimitiveComponent::IsOverlappingActor(const AActor* Other) const
{
    if (Other)
    {
        for (int32 OverlapIdx = 0; OverlapIdx < OverlappingComponents.Num(); ++OverlapIdx)
        {
            // UPrimitiveComponent const* const PrimComp = OverlappingComponents[OverlapIdx].OverlapInfo.Component.Get();
            UPrimitiveComponent const* const PrimComp = OverlappingComponents[OverlapIdx].OverlapInfo.Component;
            if ( PrimComp && (PrimComp->GetOwner() == Other) )
            {
                return true;
            }
        }
    }

    return false;
}

const TArray<FOverlapInfo>& UPrimitiveComponent::GetOverlapInfos() const
{
    return OverlappingComponents;
}

void UPrimitiveComponent::UpdateOverlapsImpl(const TArray<FOverlapInfo>* PendingOverlaps)
{
    const AActor* const MyActor = GetOwner();
    

    
}
