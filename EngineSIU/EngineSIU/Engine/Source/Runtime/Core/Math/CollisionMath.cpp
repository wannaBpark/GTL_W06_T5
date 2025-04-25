#include "Math/CollisionMath.h"
#include "Math/ShapeInfo.h" // FBox, FSphere, FCapsule
#include "MathUtility.h"


bool FCollisionMath::IntersectBoxBox(const FBox& A, const FBox& B)
{
    return (A.Max.X > B.Min.X && A.Min.X < B.Max.X) &&
        (A.Max.Y > B.Min.Y && A.Min.Y < B.Max.Y) &&
        (A.Max.Z > B.Min.Z && A.Min.Z < B.Max.Z);
}

bool FCollisionMath::IntersectBoxSphere(const FBox& Box, const FVector& SphereCenter, float Radius)
{
    const FVector ClosestPoint = FVector(
        FMath::Clamp(SphereCenter.X, Box.Min.X, Box.Max.X),
        FMath::Clamp(SphereCenter.Y, Box.Min.Y, Box.Max.Y),
        FMath::Clamp(SphereCenter.Z, Box.Min.Z, Box.Max.Z)
    );

    return (ClosestPoint - SphereCenter).LengthSquared() <= Radius * Radius;
}

bool FCollisionMath::IntersectBoxCapsule(const FBox& Box, const FCapsule& Capsule)
{
    const FVector SegmentCenter = (Capsule.PointTop + Capsule.PointBottom) * 0.5f;
    FVector ClosestPoint = ClosestPointOnSegment(Capsule.PointTop, Capsule.PointBottom, SegmentCenter);

    ClosestPoint.X = FMath::Clamp(ClosestPoint.X, Box.Min.X, Box.Max.X);
    ClosestPoint.Y = FMath::Clamp(ClosestPoint.Y, Box.Min.Y, Box.Max.Y);
    ClosestPoint.Z = FMath::Clamp(ClosestPoint.Z, Box.Min.Z, Box.Max.Z);

    return (ClosestPoint - SegmentCenter).LengthSquared() <= Capsule.Radius * Capsule.Radius;
}

bool FCollisionMath::IntersectSphereSphere(const FSphere& A, const FSphere& B)
{
    float RadiusSum = A.Radius + B.Radius;
    return (A.Center - B.Center).LengthSquared() <= RadiusSum * RadiusSum;
}

bool FCollisionMath::IntersectCapsuleSphere(const FCapsule& Capsule, const FVector& SphereCenter, float Radius)
{
    FVector Closest = ClosestPointOnSegment(Capsule.PointTop, Capsule.PointBottom, SphereCenter);
    float RadiusSum = Capsule.Radius + Radius;
    return (Closest - SphereCenter).LengthSquared() <= RadiusSum * RadiusSum;
}

bool FCollisionMath::IntersectCapsuleCapsule(const FCapsule& A, const FCapsule& B)
{
    FVector P1, P2;
    ClosestPointsBetweenSegments(A.PointTop, A.PointBottom, B.PointTop, B.PointBottom, P1, P2);
    float RadiusSum = A.Radius + B.Radius;
    return (P1 - P2).LengthSquared() <= RadiusSum * RadiusSum;
}

FVector FCollisionMath::ClosestPointOnSegment(const FVector& A, const FVector& B, const FVector& P)
{
    FVector AB = B - A;
    float AB_LengthSq = AB.LengthSquared();
    if (AB_LengthSq <= SMALL_NUMBER) return A;

    float T = FVector::DotProduct(P - A, AB) / AB_LengthSq;
    T = FMath::Clamp(T, 0.0f, 1.0f);
    return A + AB * T;
}
void FCollisionMath::ClosestPointsBetweenSegments(const FVector& P1, const FVector& Q1,
    const FVector& P2, const FVector& Q2,
    FVector& OutP, FVector& OutQ)
{
    const FVector D1 = Q1 - P1;
    const FVector D2 = Q2 - P2;
    const FVector R = P1 - P2;
    const float A = D1.LengthSquared();
    const float E = D2.LengthSquared();
    const float F = FVector::DotProduct(D2, R);

    float S, T;

    if (A <= SMALL_NUMBER && E <= SMALL_NUMBER)
    {
        S = T = 0.0f;
    }
    else if (A <= SMALL_NUMBER)
    {
        S = 0.0f;
        T = FMath::Clamp(F / E, 0.0f, 1.0f);
    }
    else
    {
        const float C = FVector::DotProduct(D1, R);
        if (E <= SMALL_NUMBER)
        {
            T = 0.0f;
            S = FMath::Clamp(-C / A, 0.0f, 1.0f);
        }
        else
        {
            const float B = FVector::DotProduct(D1, D2);
            const float Denom = A * E - B * B;
            if (Denom != 0.0f)
            {
                S = FMath::Clamp((B * F - C * E) / Denom, 0.0f, 1.0f);
            }
            else S = 0.0f;

            T = (B * S + F) / E;

            if (T < 0.0f) { T = 0.0f; S = FMath::Clamp(-C / A, 0.0f, 1.0f); }
            else if (T > 1.0f) { T = 1.0f; S = FMath::Clamp((B - C) / A, 0.0f, 1.0f); }
        }
    }

    OutP = P1 + D1 * S;
    OutQ = P2 + D2 * T;
}

