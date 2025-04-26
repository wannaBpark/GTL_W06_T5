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
    
    if (ScriptName.IsEmpty())
    {
        FString SceneName = GetWorld()->GetActiveLevel()->GetLevelName();
        ScriptName = FString::Printf(TEXT("Scripts/%s/%s.lua"), *SceneName, *GetOwner()->GetClass()->GetName());
    }

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

sol::table& ULuaScriptComponent::LoadScript()
{
    if (ScriptName.IsEmpty())
    {
        FString SceneName = GetWorld()->GetActiveLevel()->GetLevelName();
        ScriptName = FString::Printf(TEXT("Scripts/%s/%s.lua"), *SceneName, *GetOwner()->GetClass()->GetName());
    }

    LuaEnv = FLuaScriptManager::Get().CreateLuaTable(ScriptName);

    if (!LuaEnv.valid())
        return LuaEnv;

    LuaEnv["self"] = GetOwner();

    return LuaEnv;
}
