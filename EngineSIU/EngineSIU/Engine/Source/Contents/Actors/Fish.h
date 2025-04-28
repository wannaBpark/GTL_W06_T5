#pragma once
#include "Actors/Player.h"

class UFishBodyComponent;
class USphereComponent;
class UStaticMeshComponent;
class UFishTailComponent;

class AFish : public APlayer
{
    DECLARE_CLASS(AFish, APlayer)

public:
    AFish();
    virtual ~AFish() override = default;

    virtual void PostSpawnInitialize() override;

    UObject* Duplicate(UObject* InOuter) override;

    void BeginPlay() override;

    void Tick(float DeltaTime) override;

protected:
    UPROPERTY
    (USphereComponent*, SphereComponent, = nullptr)

    UPROPERTY
    (UFishBodyComponent*, FishBody, = nullptr)

    UPROPERTY
    (UFishTailComponent*, FishTail, = nullptr)

    FVector Velocity = FVector::ZeroVector;

    float JumpZVelocity = 50.f;

    float Gravity = -9.8f * 10.f;

    void Move(float DeltaTime);

    void RotateMesh();

    float MeshPitchMax = 15.f;

    float MeshRotSpeed = 10.f;

    void ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
