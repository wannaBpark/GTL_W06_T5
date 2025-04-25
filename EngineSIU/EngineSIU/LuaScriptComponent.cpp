#include "LuaScriptComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"

ULuaScriptComponent::ULuaScriptComponent()
{
    LuaState.open_libraries();
}

ULuaScriptComponent::~ULuaScriptComponent()
{
}

void ULuaScriptComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeLuaState();
    CallLuaFunction("BeginPlay");
}

void ULuaScriptComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    CallLuaFunction("EndPlay");
}

void ULuaScriptComponent::InitializeLuaState()
{
    LuaState.open_libraries();

    BindEngineAPI();

    try {
        LuaState.script_file((*ScriptPath));
        bScriptValid = true;
    }
    catch (const sol::error& e) {
        UE_LOG(LogLevel::Error, TEXT("Lua Initialization error"));
    }
}

void ULuaScriptComponent::BindEngineAPI()
{
    auto ActorType = LuaState.new_usertype<AActor>("Actor",
        "GetLocation", &AActor::GetActorLocation,
        "SetLocation", &AActor::SetActorLocation
    );

    // 프로퍼티 바인딩
    LuaState["actor"] = GetOwner();
    LuaState["script"] = this;
}

void ULuaScriptComponent::CallLuaFunction(const FString& FunctionName)
{
    if (!bScriptValid) return;

    sol::protected_function func = LuaState[FunctionName];
    if (func.valid()) {
        auto result = func();
        if (!result.valid()) {
            sol::error err = result;
            UE_LOG(LogLevel::Error, TEXT("Lua Function Error: %s"), err);
        }
    }
}

void ULuaScriptComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    if (bScriptValid && LuaState["Tick"].valid()) {
        LuaState["Tick"](DeltaTime);
    }
}
