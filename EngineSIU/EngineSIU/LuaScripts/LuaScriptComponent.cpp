#include "LuaScriptComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "LuaBindingHelpers.h"
#include "World/World.h"
#include <Windows.h>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Shell32.lib")


static inline bool CopyTemplateToActorScript(
    const std::wstring& templateName,
    const std::wstring& sceneName,
    const std::wstring& actorName,
    FString& outScriptPath,
    FString& outScriptName
)
{
    LPCWSTR luaDir = L"LuaScripts";

    // 원본 템플릿 절대 경로
    wchar_t src[MAX_PATH] = { 0 };
    PathCombineW(src, luaDir, templateName.c_str());
    if (!PathFileExistsW(src))
        return false;

    // 대상 파일명: Scene_Actor.lua
    std::wstring destName = sceneName + L"_" + actorName + L".lua";
    outScriptName = FString(destName.c_str());

    wchar_t dst[MAX_PATH] = { 0 };
    PathCombineW(dst, luaDir, destName.c_str());

    if (PathFileExistsW(dst)) {
        outScriptPath = FString(dst);
        return true;
    }

    // 복제 (덮어쓰기 허용)
    if (!CopyFileW(src, dst, FALSE))
        return false;

    outScriptPath = FString(dst);
    return true;
}


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

void ULuaScriptComponent::SetScriptPath(const FString& InScriptPath)
{
    ScriptPath = InScriptPath;
    bScriptValid = false;
}

void ULuaScriptComponent::InitializeLuaState()
{
    if (ScriptPath.IsEmpty()) {
        bool bSuccess = CopyTemplateToActorScript(
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

