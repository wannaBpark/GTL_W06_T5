#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class AActor;
class UWorld;


class ULevel : public UObject
{
    DECLARE_CLASS(ULevel, UObject)

public:
    ULevel() = default;

    void InitLevel(UWorld* InOwningWorld);
    void Release();

    virtual UObject* Duplicate(UObject* InOuter) override;

    TArray<AActor*> Actors;
    UWorld* OwningWorld;

    FString GetLevelName() const { return LevelName; }
    void SetLevelName(const FString& InLevelName) { LevelName = InLevelName; }
    
private:
    FString LevelName = "DefaultLevel";
};
