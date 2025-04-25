
#pragma once
#include "CoreMiscDefines.h"
#include "HAL/PlatformType.h"
#include "HitResult.h"

class UPrimitiveComponent;

/** Overlap info consisting of the primitive and the body that is overlapping */
struct FOverlapInfo
{
    FOverlapInfo()
    {}

    explicit FOverlapInfo(const FHitResult& InSweepResult)
        : bFromSweep(true), OverlapInfo(InSweepResult)
    {
    }

    explicit FOverlapInfo(UPrimitiveComponent* InComponent, int32 InBodyIndex = INDEX_NONE);

    int32 GetBodyIndex() const { return OverlapInfo.Item; }

    //This function completely ignores SweepResult information. It seems that places that use this function do not care, but it still seems risky
    friend bool operator == (const FOverlapInfo& LHS, const FOverlapInfo& RHS) { return LHS.OverlapInfo.Component == RHS.OverlapInfo.Component && LHS.OverlapInfo.Item == RHS.OverlapInfo.Item; }
    bool bFromSweep;

    /** Information for both sweep and overlap queries. Different parts are valid depending on bFromSweep.
      * If bFromSweep is true then FHitResult is completely valid just like a regular sweep result.
      * If bFromSweep is false only FHitResult::Component, FHitResult::Actor, FHitResult::Item are valid as this is really just an FOverlapResult*/
    FHitResult OverlapInfo;
};
