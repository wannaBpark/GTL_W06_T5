#include "CollisionManager.h"

#include "Components/PrimitiveComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

#include "Engine/OverlapResult.h"
#include "Math/Quat.h"
#include "UObject/Casts.h"
#include "UObject/UObjectIterator.h"

// 점 P와 선분 AB 사이의 가장 가까운 점을 찾는 함수
FVector ClosestPointOnLineSegment(const FVector& P, const FVector& A, const FVector& B)
{
    FVector AB = B - A;
    float t = FVector::DotProduct(P - A, AB) / FVector::DotProduct(AB, AB);
    t = FMath::Max(0.f, FMath::Min(1.f, t)); // 선분 내로 클램핑
    return A + AB * t;
}

// 두 선분 (A-B, C-D) 사이의 최단 거리 제곱을 반환하는 함수 (구현 복잡)
float SquaredDistBetweenLineSegments(const FVector& A, const FVector& B, const FVector& C, const FVector& D)
{
    FVector u = B - A;
    FVector v = D - C;
    FVector w = A - C;
    
    float a = FVector::DotProduct(u, u);
    float b = FVector::DotProduct(u, v);
    float c = FVector::DotProduct(v, v);
    float d = FVector::DotProduct(u, w);
    float e = FVector::DotProduct(v, w);
    float F = a*c - b*b;
    
    float sc, tc;
    
    // 두 선이 거의 평행한 경우
    if (F < FLT_EPSILON)
    {
        sc = 0.0f;
        tc = (b > c ? d/b : e/c);
    }
    else
    {
        sc = (b*e - c*d) / F;
        tc = (a*e - b*d) / F;
    }
    
    // 파라미터 값을 [0,1] 범위로 클램핑
    sc = FMath::Clamp(sc, 0.0f, 1.0f);
    tc = FMath::Clamp(tc, 0.0f, 1.0f);
    
    // 각 선분에서 가장 가까운 점 계산
    FVector P = A + u * sc;
    FVector Q = C + v * tc;
    
    // 두 점 사이의 거리 제곱 반환
    return (P - Q).LengthSquared();
}

// 점 P와 OBB 사이의 가장 가까운 점을 찾는 함수
FVector ClosestPointOnOBB(const FVector& P, const UBoxComponent* Box)
{
    FMatrix WorldToBox = FMatrix::Inverse(Box->GetWorldMatrix()); // 가상 함수: 월드->로컬 변환 행렬
    FVector P_Local = WorldToBox.TransformPosition(P); // 점을 박스 로컬 공간으로 변환

    // 로컬 공간에서 각 축으로 클램핑
    FVector BoxExtent = Box->GetBoxExtent();
    FVector ClosestP_Local;
    ClosestP_Local.X = std::max(-BoxExtent.X, std::min(P_Local.X, BoxExtent.X));
    ClosestP_Local.Y = std::max(-BoxExtent.Y, std::min(P_Local.Y, BoxExtent.Y));
    ClosestP_Local.Z = std::max(-BoxExtent.Z, std::min(P_Local.Z, BoxExtent.Z));

    FMatrix BoxToWorld = Box->GetWorldMatrix(); // 가상 함수: 로컬->월드 변환 행렬
    
    return BoxToWorld.TransformPosition(ClosestP_Local); // 월드 좌표로 다시 변환
}

// SAT 검사를 위한 헬퍼 함수: OBB를 특정 축에 투영하여 최소/최대값 반환
void ProjectOBB(const UBoxComponent* Box, const FVector& Axis, float& OutMin, float& OutMax)
{
    FVector Axes[3] = { Box->GetForwardVector(), Box->GetRightVector(), Box->GetUpVector() };
    FVector HalfSize = Box->GetBoxExtent();

    // 박스 중심을 축에 투영
    float CenterProj = FVector::DotProduct(Box->GetWorldLocation(), Axis);

    // 박스 각 축 방향의 반경을 현재 축에 투영한 값의 절대값을 모두 더함
    float RadiusProj = FMath::Abs(FVector::DotProduct(Axes[0] * HalfSize.X, Axis)) +
                       FMath::Abs(FVector::DotProduct(Axes[1] * HalfSize.Y, Axis)) +
                       FMath::Abs(FVector::DotProduct(Axes[2] * HalfSize.Z, Axis));

    OutMin = CenterProj - RadiusProj;
    OutMax = CenterProj + RadiusProj;
}

// 두 구간 [minA, maxA], [minB, maxB]가 겹치는지 검사
bool OverlapOnAxis(float MinA, float MaxA, float MinB, float MaxB)
{
    return MaxA >= MinB && MinA <= MaxB;
}

// 선분(A-B)과 OBB Box 사이의 최단 거리 제곱을 계산하는 가상 함수 (구현 복잡)
float SquaredDistSegmentOBB(const FVector& A, const FVector& B, const UBoxComponent* Box)
{
    // 구현: 선분의 각 점 P에 대해 ClosestPointOnOBB(P, Box)를 찾고,
    // P와 그 결과 점 사이의 거리 제곱 중 최소값을 찾아야 함.
    // 이는 OBB의 면, 에지, 꼭지점 영역을 고려해야 하는 복잡한 과정임.
    // Real-Time Collision Detection 책 등 고급 자료 참고 필요.
    return FLT_MAX; // 플레이스홀더
}

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

    const bool bOverlapped = (this->*CollisionMatrix[ShapeTypeA][ShapeTypeB])(ShapeA, ShapeB, OutResult);
    if (bOverlapped)
    {
        OutResult.Actor = OtherComponent->GetOwner();
        OutResult.Component = const_cast<UPrimitiveComponent*>(OtherComponent);
        OutResult.bBlockingHit = false;
        
        return true;
    }

    return false;
}

bool FCollisionManager::Check_NotImplemented(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    return false;
}

bool FCollisionManager::Check_Box_Box(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    UBoxComponent* BoxA = Cast<UBoxComponent>(A);
    UBoxComponent* BoxB = Cast<UBoxComponent>(B);
    
    FVector AxesA[3] = { BoxA->GetForwardVector(), BoxA->GetRightVector(), BoxA->GetUpVector() };
    FVector AxesB[3] = { BoxB->GetForwardVector(), BoxB->GetRightVector(), BoxB->GetUpVector() };
    FVector TestAxes[15]; // 검사할 축 (최대 15개)

    int32 AxisIndex = 0;
    
    // 1. Box A의 3개 축
    for (int32 i = 0; i < 3; ++i)
    {
        TestAxes[AxisIndex++] = AxesA[i];
    }
    
    // 2. Box B의 3개 축
    for (int32 i = 0; i < 3; ++i)
    {
        TestAxes[AxisIndex++] = AxesB[i];
    }
    
    // 3. Box A의 축과 Box B의 축 간의 외적 (Cross Product) 9개
    for (int32 i = 0; i < 3; ++i)
    {
        for (int32 j = 0; j < 3; ++j)
        {
            FVector Cross = FVector::CrossProduct(AxesA[i], AxesB[j]);
            // 외적이 0 벡터에 가까우면 (축이 평행하면) 검사할 필요 없음
            if (Cross.LengthSquared() > 1e-6f)
            { // 작은 값 (epsilon)으로 비교
                TestAxes[AxisIndex++] = Cross; //.Normalize(); // 정규화는 이론적으로 필요하나, OverlapOnAxis 결과에 영향 안 줌
            }
        }
    }

    // 모든 축에 대해 검사
    for (int32 i = 0; i < AxisIndex; ++i)
    {
        float minA, maxA, minB, maxB;
        ProjectOBB(BoxA, TestAxes[i], minA, maxA);
        ProjectOBB(BoxB, TestAxes[i], minB, maxB);

        if (!OverlapOnAxis(minA, maxA, minB, maxB))
        {
            return false; // 분리 축 발견! 충돌하지 않음.
        }
    }

    // 모든 축에서 겹쳤다면 충돌한 것임.
    return true;
}

bool FCollisionManager::Check_Box_Sphere(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    UBoxComponent* Box = Cast<UBoxComponent>(A);
    USphereComponent* Sphere = Cast<USphereComponent>(B);
    
    // 스피어 중심 P에서 OBB 위의 가장 가까운 점 Q 찾기
    FVector ClosestPoint = ClosestPointOnOBB(Sphere->GetWorldLocation(), Box);

    // P와 Q 사이의 거리 제곱 계산
    FVector Diff = Sphere->GetWorldLocation() - ClosestPoint;
    float DistSq = Diff.LengthSquared();

    // 거리 제곱이 스피어 반지름 제곱보다 작거나 같으면 충돌
    float RadiusSq = Sphere->GetRadius() * Sphere->GetRadius();
    return DistSq <= RadiusSq;
}

bool FCollisionManager::Check_Box_Capsule(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    UBoxComponent* Box = Cast<UBoxComponent>(A);
    UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(B);
    
    FVector StartCap, EndCap;
    Capsule->GetEndPoints(StartCap, EndCap);

    // 캡슐 선분과 OBB 사이의 최단 거리 제곱 계산
    float DistSq = SquaredDistSegmentOBB(StartCap, EndCap, Box);

    // 캡슐 반지름 제곱 계산
    float RadiusSq = Capsule->GetRadius() * Capsule->GetRadius();

    // 최단 거리 제곱이 캡슐 반지름 제곱보다 작거나 같으면 충돌
    return DistSq <= RadiusSq;
}

bool FCollisionManager::Check_Sphere_Box(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    return Check_Box_Sphere(B, A, OutResult);
}

bool FCollisionManager::Check_Sphere_Sphere(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    USphereComponent* SphereA = Cast<USphereComponent>(A);
    USphereComponent* SphereB = Cast<USphereComponent>(B);

    float RadiusSum = SphereA->GetRadius() + SphereB->GetRadius();

    FVector Diff = SphereA->GetWorldLocation() - SphereB->GetWorldLocation();
    float DistSq = Diff.LengthSquared();
    
    return DistSq <= (RadiusSum * RadiusSum);
}

bool FCollisionManager::Check_Sphere_Capsule(const UShapeComponent* A, const UShapeComponent* B, FOverlapResult& OutResult) const
{
    USphereComponent* Sphere = Cast<USphereComponent>(A);
    UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(B);

    FVector StartCap, EndCap;
    Capsule->GetEndPoints(StartCap, EndCap);

    // 스피어 중심(Sp.Location)에서 캡슐 선분(StartCap-EndCap)까지 가장 가까운 점 찾기
    FVector ClosestPointOnSegment = ClosestPointOnLineSegment(Sphere->GetWorldLocation(), StartCap, EndCap);

    // 스피어 중심과 가장 가까운 점 사이의 거리 제곱 계산
    FVector Diff = Sphere->GetWorldLocation() - ClosestPointOnSegment;
    float DistSq = Diff.LengthSquared();

    // 캡슐 반지름과 스피어 반지름의 합 계산
    float TotalRadius = Capsule->GetRadius() + Sphere->GetRadius();
    float TotalRadiusSq = TotalRadius * TotalRadius;

    // 거리 제곱이 반지름 합 제곱보다 작거나 같으면 충돌
    return DistSq <= TotalRadiusSq;
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
    UCapsuleComponent* CapsuleA = Cast<UCapsuleComponent>(A);
    UCapsuleComponent* CapsuleB = Cast<UCapsuleComponent>(B);
    
    FVector StartA, EndA, StartB, EndB;
    CapsuleA->GetEndPoints(StartA, EndA);
    CapsuleB->GetEndPoints(StartB, EndB);

    // 두 선분 (StartA-EndA, StartB-EndB) 사이의 최단 거리 제곱 계산
    // 주의: SquaredDistBetweenLineSegments 함수는 직접 구현해야 합니다.
    float DistSq = SquaredDistBetweenLineSegments(StartA, EndA, StartB, EndB);

    // 두 반지름의 합 계산
    float TotalRadius = CapsuleA->GetRadius() + CapsuleB->GetRadius();
    float TotalRadiusSq = TotalRadius * TotalRadius;

    // 최단 거리 제곱이 반지름 합 제곱보다 작거나 같으면 충돌
    return DistSq <= TotalRadiusSq;
}
