#pragma once
#include "Components/SceneComponent.h"
#include "Engine/OverlapInfo.h"

DECLARE_MULTICAST_DELEGATE_FiveParams(FComponentHitSignature, UPrimitiveComponent*, AActor*, UPrimitiveComponent*, FVector, const FHitResult&);
DECLARE_MULTICAST_DELEGATE_SixParams(FComponentBeginOverlapSignature, UPrimitiveComponent*, AActor*, UPrimitiveComponent*, int32, bool, const FHitResult&);
DECLARE_MULTICAST_DELEGATE_FourParams(FComponentEndOverlapSignature, UPrimitiveComponent*, AActor*, UPrimitiveComponent*, int32);

class UPrimitiveComponent : public USceneComponent
{
    DECLARE_CLASS(UPrimitiveComponent, USceneComponent)

public:
    UPrimitiveComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    
    bool IntersectRayTriangle(
        const FVector& RayOrigin, const FVector& RayDirection,
        const FVector& v0, const FVector& v1, const FVector& v2, float& OutHitDistance
    ) const;
    
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;
    
    FBoundingBox AABB;
    
    bool bGenerateOverlapEvents = true;
    bool bBlockComponent = true;

    FComponentHitSignature OnComponentHit;

    FComponentBeginOverlapSignature OnComponentBeginOverlap;

    FComponentEndOverlapSignature OnComponentEndOverlap;

    /** 
     * Begin tracking an overlap interaction with the component specified.
     * @param OtherComp - The component of the other actor that this component is now overlapping
     * @param bDoNotifies - True to dispatch appropriate begin/end overlap notifications when these events occur.
     * @see [Overlap Events](https://docs.unrealengine.com/InteractiveExperiences/Physics/Collision/Overview#overlapandgenerateoverlapevents)
     */
    void BeginComponentOverlap(const FOverlapInfo& OtherOverlap, bool bDoNotifies);

    /** 
     * Finish tracking an overlap interaction that is no longer occurring between this component and the component specified. 
     * @param OtherComp The component of the other actor to stop overlapping
     * @param bDoNotifies True to dispatch appropriate begin/end overlap notifications when these events occur.
     * @param bSkipNotifySelf True to skip end overlap notifications to this component's.  Does not affect notifications to OtherComp's actor.
     * @see [Overlap Events](https://docs.unrealengine.com/InteractiveExperiences/Physics/Collision/Overview#overlapandgenerateoverlapevents)
     */
    void EndComponentOverlap(const FOverlapInfo& OtherOverlap, bool bDoNotifies=true, bool bSkipNotifySelf=false);

    /**
     * Check whether this component is overlapping another component.
     * @param OtherComp Component to test this component against.
     * @return Whether this component is overlapping another component.
     */
    bool IsOverlappingComponent(const UPrimitiveComponent* OtherComp) const;

    /** Check whether this component has the specified overlap. */
    bool IsOverlappingComponent(const FOverlapInfo& Overlap) const;

    /**
     * Check whether this component is overlapping any component of the given Actor.
     * @param Other Actor to test this component against.
     * @return Whether this component is overlapping any component of the given Actor.
     */
    bool IsOverlappingActor(const AActor* Other) const;

    /** 
     * Returns a list of actors that this component is overlapping.
     * @param OverlappingActors		[out] Returned list of overlapping actors
     * @param ClassFilter			[optional] If set, only returns actors of this class or subclasses
     */
    //void GetOverlappingActors(TArray<AActor*>& OverlappingActors, TSubclassOf<AActor> ClassFilter=nullptr) const;

    /** 
    * Returns the set of actors that this component is overlapping.
    * @param OverlappingActors		[out] Returned list of overlapping actors
    * @param ClassFilter			[optional] If set, only returns actors of this class or subclasses
    */
    //void GetOverlappingActors(TSet<AActor*>& OverlappingActors, TSubclassOf<AActor> ClassFilter=nullptr) const;

    /** Returns unique list of components this component is overlapping. */
    void GetOverlappingComponents(TArray<UPrimitiveComponent*>& OutOverlappingComponents) const;

    /** Returns unique set of components this component is overlapping. */
    void GetOverlappingComponents(TSet<UPrimitiveComponent*>& OutOverlappingComponents) const;

    /** Returns list of components this component is overlapping. */
    const TArray<FOverlapInfo>& GetOverlapInfos() const;

protected:
    TArray<FOverlapInfo> OverlappingComponents;

    virtual void UpdateOverlapsImpl(const TArray<FOverlapInfo>* PendingOverlaps = nullptr, bool bDoNotifies = true, const TArray<const FOverlapInfo>* OverlapsAtEndLocation = nullptr) override;

    void ClearComponentOverlaps(bool bDoNotifies, bool bSkipNotifySelf);
    
private:
    FString m_Type;

public:
    FString GetType() { return m_Type; }

    void SetType(const FString& _Type)
    {
        m_Type = _Type;
        //staticMesh = FEngineLoop::resourceMgr.GetMesh(m_Type);
    }
    
    FBoundingBox GetBoundingBox() const { return AABB; }
};

