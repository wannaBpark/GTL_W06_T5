#include "LuaScriptComponent.h"

#include "Engine/LuaScriptManager.h"
#include "GameFramework/Actor.h"

#include "World/World.h"
#include "Level.h"

ULuaScriptComponent::ULuaScriptComponent()
{
}

UObject* ULuaScriptComponent::Duplicate(UObject* InOuter)
{
    ULuaScriptComponent* NewComponent = Cast<ULuaScriptComponent>(Super::Duplicate(InOuter));

    if (!NewComponent)
        return nullptr;

    NewComponent->ScriptName = ScriptName;
    NewComponent->LuaEnv = LuaEnv;

    return NewComponent;
}

void ULuaScriptComponent::BeginPlay()
{
    Super::BeginPlay();
    
    FString SceneName = GetWorld()->GetActiveLevel()->GetLevelName();
    ScriptName = FString::Printf(TEXT("Scripts/%s/%s.lua"), *SceneName, *GetOwner()->GetClass()->GetName());

    if (LuaEnv.valid() && LuaEnv["BeginPlay"].valid())
    {
        LuaEnv["BeginPlay"]();
    }
}

void ULuaScriptComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    if (LuaEnv.valid() && LuaEnv["Tick"].valid())
    {
        UE_LOG(LogLevel::Display, TEXT("LuaScriptComponent::TickComponent DeltaTime: %f"), DeltaTime);
        LuaEnv["Tick"](DeltaTime);
    }
}

void ULuaScriptComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (LuaEnv.valid() && LuaEnv["EndPlay"].valid())
    {
        LuaEnv["EndPlay"](EndPlayReason);
    }
}

void ULuaScriptComponent::LoadScript()
{
    /*FString SceneName = GetWorld()->GetActiveLevel()->GetLevelName();
    ScriptName = FString::Printf(TEXT("Scripts/%s/%s.lua"), *SceneName, *GetOwner()->GetClass()->GetName());*/

    if (ScriptName.IsEmpty())
    {
        return;
    }
    LuaEnv = FLuaScriptManager::Get().CreateLuaTable(ScriptName);

    if (!LuaEnv.valid())
        return;

    LuaEnv["self"] = GetOwner();
    GetOwner()->SetupLuaProperties(FLuaScriptManager::Get().GetLua(), this);
}
