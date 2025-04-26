#include "LuaScriptComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "LuaBindingHelpers.h"
#include "World/World.h"
#include "LuaScriptFileUtils.h"
#include <tchar.h>

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
    // GetOwner()->SetActorTickInPIE(true); // Lua 스크립트 대상 - PIE의 tick 호출 대상

}

void ULuaScriptComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    CallLuaFunction("EndPlay");
}

UObject* ULuaScriptComponent::Duplicate(UObject* InOuter)
{
    ULuaScriptComponent* NewComponent = Cast<ULuaScriptComponent>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->ScriptPath = ScriptPath;
        NewComponent->DisplayName = DisplayName;
    }
    return NewComponent;
}

void ULuaScriptComponent::SetScriptPath(const FString& InScriptPath)
{
    ScriptPath = InScriptPath;
    bScriptValid = false;
}

void ULuaScriptComponent::InitializeLuaState()
{
    if (ScriptPath.IsEmpty()) {
        bool bSuccess = LuaScriptFileUtils::CopyTemplateToActorScript(
            L"template.lua",
            GetWorld()->GetName().ToWideString(),
            GetOwner()->GetName().ToWideString(),
            ScriptPath,
            DisplayName
        );
        if (!bSuccess) {
            UE_LOG(LogLevel::Error, TEXT("Failed to create script from template"));
            return;
        }
    }

    LuaState.open_libraries();
    BindEngineAPI();

    try {
        LuaState.script_file((*ScriptPath));
        bScriptValid = true;
        const std::wstring FilePath = ScriptPath.ToWideString();
        LastWriteTime = std::filesystem::last_write_time(FilePath);
    }
    catch (const sol::error& err) {
        UE_LOG(LogLevel::Error, TEXT("Lua Initialization error: %s"), err.what());
    }
}

void ULuaScriptComponent::BindEngineAPI()
{
    LuaBindingHelpers::BindPrint(LuaState);    // 0) Print 바인딩
    LuaBindingHelpers::BindUE_LOG(LuaState);    // 1) UE_LOG 바인딩
    LuaBindingHelpers::BindFVector(LuaState);   // 2) FVector 바인딩

    /*bool bHasFVector = LuaState["FVector"].valid();
    bool bHasUELOG =   LuaState["UE_LOG"].valid();
    UE_LOG(LogLevel::Error, TEXT("LuaBindings – FVector valid=%s, UE_LOG valid=%s"),
        bHasFVector ? TEXT("true") : TEXT("false"),
        bHasUELOG ? TEXT("true") : TEXT("false"));*/

    // 2) AActor 바인딩 및 Location Property
    auto ActorType = LuaState.new_usertype<AActor>("Actor",
        sol::constructors<>(),
        "Location", sol::property(
            &AActor::GetActorLocation,
            &AActor::SetActorLocation
        ),
        "GetLocation", &AActor::GetActorLocation,
        "SetLocation", &AActor::SetActorLocation
    );
    
    // 프로퍼티 바인딩
    LuaState["actor"] = GetOwner();
    LuaState["script"] = this;
}


bool ULuaScriptComponent::CheckFileModified()
{
    if (ScriptPath.IsEmpty()) return false;

    try {
        std::wstring FilePath = ScriptPath.ToWideString();
        const auto CurrentTime = std::filesystem::last_write_time(FilePath);

        if (CurrentTime > LastWriteTime) {
            LastWriteTime = CurrentTime;
            return true;
        }
    }
    catch (const std::exception& e) {
        UE_LOG(LogLevel::Error, TEXT("Failed to check lua script file"));
    }
    return false;
}

void ULuaScriptComponent::ReloadScript()
{
    sol::table PersistentData;
    if (bScriptValid && LuaState["PersistentData"].valid()) {
        PersistentData = LuaState["PersistentData"];
    }

    LuaState = sol::state();
    InitializeLuaState();

    if (PersistentData.valid()) {
        LuaState["PersistentData"] = PersistentData;
    }

    CallLuaFunction("OnHotReload");
    CallLuaFunction("BeginPlay");
}

void ULuaScriptComponent::CallLuaFunction(const FString& FunctionName)
{
    if (!bScriptValid) return;

    sol::protected_function func = LuaState[FunctionName];
    if (func.valid()) {
        auto result = func();
        if (!result.valid()) {
            sol::error err = result;
            UE_LOG(LogLevel::Error, TEXT("Lua Function Error: %s"), err.what());
        }
    }
}

void ULuaScriptComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    if (bScriptValid && LuaState["Tick"].valid()) {
        LuaState["Tick"](DeltaTime);
        CallLuaFunction("Tick");
    }

    if (CheckFileModified()) {
        try {
            ReloadScript();
            UE_LOG(LogLevel::Display, TEXT("Lua script reloaded"));
        }
        catch (const sol::error& e) {
            UE_LOG(LogLevel::Error, TEXT("Failed to reload lua script"));
        }
    }
}

