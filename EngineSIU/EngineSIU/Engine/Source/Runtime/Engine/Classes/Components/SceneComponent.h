#pragma once
#include "ActorComponent.h"
#include "Math/Rotator.h"
#include "UObject/ObjectMacros.h"

class USceneComponent : public UActorComponent
{
    DECLARE_CLASS(USceneComponent, UActorComponent)

public:
    USceneComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;

    
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& InRayOrigin, FVector& InRayDirection, float& pfNearHitDistance);
    virtual void DestroyComponent(bool bPromoteChildren = false) override;

    virtual FVector GetForwardVector();
    virtual FVector GetRightVector();
    virtual FVector GetUpVector();
    
    void AddLocation(FVector InAddValue);
    void AddRotation(FVector InAddValue);
    void AddScale(FVector InAddValue);

    USceneComponent* GetAttachParent() const { return AttachParent; }
    const TArray<USceneComponent*>& GetAttachChildren() const { return AttachChildren; }

    void AttachToComponent(USceneComponent* InParent);
    void SetupAttachment(USceneComponent* InParent);
    void DetachFromComponent(USceneComponent* Target);

public:
    void SetRelativeLocation(FVector InNewLocation) { RelativeLocation = InNewLocation; }
    void SetRelativeRotation(FRotator InNewRotation) { RelativeRotation = InNewRotation; }
    void SetRelativeScale3D(FVector NewScale) { RelativeScale3D = NewScale; }
    
    FVector GetRelativeLocation() const { return RelativeLocation; }
    FRotator GetRelativeRotation() const { return RelativeRotation; }
    FVector GetRelativeScale3D() const { return RelativeScale3D; }

    FVector GetWorldLocation() const;
    FRotator GetWorldRotation() const;
    FVector GetWorldScale3D() const;

    FMatrix GetScaleMatrix() const;
    FMatrix GetRotationMatrix() const;
    FMatrix GetTranslationMatrix() const;

    FMatrix GetWorldMatrix() const;

protected:
    /** 부모 컴포넌트로부터 상대적인 위치 */
    UPROPERTY
    (FVector, RelativeLocation);

    /** 부모 컴포넌트로부터 상대적인 회전 */
    UPROPERTY
    (FRotator, RelativeRotation);

    /** 부모 컴포넌트로부터 상대적인 크기 */
    UPROPERTY
    (FVector, RelativeScale3D);


    UPROPERTY
    (USceneComponent*, AttachParent, = nullptr);

    UPROPERTY
    (TArray<USceneComponent*>, AttachChildren);
};
