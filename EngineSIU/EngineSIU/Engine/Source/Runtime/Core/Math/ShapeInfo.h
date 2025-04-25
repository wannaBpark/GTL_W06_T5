#pragma once
#include"Math/Vector.h"

struct FBox
{
    FVector Min;
    FVector Max;

    FBox() = default;

    FBox(const FVector& InMin, const FVector& InMax)
        : Min(InMin), Max(InMax) {
    }

    FVector GetCenter() const { return (Min + Max) * 0.5f; }
    FVector GetExtent() const { return (Max - Min) * 0.5f; }
    bool IsValid() const { return Min.X <= Max.X && Min.Y <= Max.Y && Min.Z <= Max.Z; }
};

// Sphere
struct FSphere
{
    FVector Center;
    float Radius;

    FSphere() = default;

    FSphere(const FVector& InCenter, float InRadius)
        : Center(InCenter), Radius(InRadius) {
    }
};

// Capsule = Line segment + radius
struct FCapsule
{
    FVector PointTop;
    FVector PointBottom;
    float Radius;

    FCapsule() = default;

    FCapsule(const FVector& InA, const FVector& InB, float InRadius)
        : PointTop(InA), PointBottom(InB), Radius(InRadius) {
    }

    FVector GetCenter() const { return (PointTop + PointBottom) * 0.5f; }
};
