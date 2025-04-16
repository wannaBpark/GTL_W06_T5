﻿#pragma once
#include "Container/String.h"

struct FVector;
struct FQuat;
struct FMatrix;

#include "MathUtility.h"

// 회전 정보를 Degree 단위로 저장하는 구조체
struct FRotator
{
    float Pitch;
    float Yaw;
    float Roll;

    FRotator()
        : Pitch(0.0f), Yaw(0.0f), Roll(0.0f)
    {}

    FRotator(float InPitch, float InYaw, float InRoll)
        : Pitch(InPitch), Yaw(InYaw), Roll(InRoll)
    {}

    FRotator(const FRotator& Other)
        : Pitch(Other.Pitch), Yaw(Other.Yaw), Roll(Other.Roll)
    {}

    FRotator(const FVector& InVector);
    FRotator(const FQuat& InQuat);

    FRotator operator+(const FRotator& Other) const;
    FRotator& operator+=(const FRotator& Other);

    FRotator operator-(const FRotator& Other) const;
    FRotator& operator-=(const FRotator& Other);

    FRotator operator*(float Scalar) const;
    FRotator& operator*=(float Scalar);

    FRotator operator/(const FRotator& Other) const;
    FRotator operator/(float Scalar) const;
    FRotator& operator/=(float Scalar);

    FRotator operator-() const;

    bool operator==(const FRotator& Other) const;
    bool operator!=(const FRotator& Other) const;

    bool IsNearlyZero(float Tolerance = KINDA_SMALL_NUMBER) const;
    bool IsZero() const;

    bool Equals(const FRotator& Other, float Tolerance = KINDA_SMALL_NUMBER) const;

    FRotator Add(float DeltaPitch, float DeltaYaw, float DeltaRoll) const;

    FRotator FromQuaternion(const FQuat& InQuat) const;
    FQuat ToQuaternion() const;
    FVector ToVector() const;
    FMatrix ToMatrix() const;

    float Clamp(float Angle) const;
    FRotator GetNormalized() const;
    void Normalize();

    
    FString ToString() const;
    bool InitFromString(const FString& InSourceString);
};
