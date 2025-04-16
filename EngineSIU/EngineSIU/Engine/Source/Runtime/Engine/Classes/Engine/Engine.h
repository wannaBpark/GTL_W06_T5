#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "Container/Array.h"
#include "World/WorldContext.h"

class UAssetManager;
class UWorld;

class UEngine : public UObject
{
    DECLARE_ABSTRACT_CLASS(UEngine, UObject)

public:
    UEngine() = default;

public:
    UWorld* ActiveWorld;
    UAssetManager* AssetManager = nullptr;

protected:
    TArray<FWorldContext*> WorldList;
    int32 NextWorldContextHandle = 0;

public:
    virtual void Init();
    virtual void Tick(float DeltaTime) = 0;

    // TODO: UObject->GetWorld() 구현 이후 추가.
    UWorld* GetWorldFromContextObject(const UObject* Object) const;
    FWorldContext* GetWorldContextFromWorld(const UWorld* InWorld);

    FWorldContext& CreateNewWorldContext(EWorldType InWorldType);

    
    void LoadWorld(const FString& FileName) const;
    void SaveWorld(const FString& FileName) const;
};

extern class UEngine* GEngine;
