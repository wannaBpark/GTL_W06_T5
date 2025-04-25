#pragma once

class UBoxComponent;
class USphereComponent;
class UCapsuleComponent;

class CollisionMath
{
    bool AABBvsAABB(const FBoundingBox& A, const FBoundingBox& B);
    bool AABBvsSphere(const FBoundingBox& Box, const FVector& SphereCenter, float Radius);
    bool CapsuleVsSphere(const FCapsule& Capsule, const FVector& SphereCenter, float Radius);
    bool CapsuleVsCapsule(const FCapsule& A, const FCapsule& B);
    bool AABBvsCapsule(const FBoundingBox& Box, const FCapsule& Capsule);
    bool SphereVsSphere(const FSphere& A, const FSphere& B);
};

