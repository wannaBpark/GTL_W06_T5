#pragma once
#include "Actors/Player.h"

class USphereComponent;
class UStaticMeshComponent;
class UFishTailComponent;

class AFish : public APlayer
{
    DECLARE_CLASS(AFish, APlayer)

public:
    AFish();
    virtual ~AFish() override = default;

    UObject* Duplicate(UObject* InOuter) override;

protected:
    UPROPERTY
    (USphereComponent*, SphereComponent, = nullptr)

    UPROPERTY
    (UStaticMeshComponent*, FishBody, = nullptr)

    UPROPERTY
    (UFishTailComponent*, FishTail, = nullptr)
};
