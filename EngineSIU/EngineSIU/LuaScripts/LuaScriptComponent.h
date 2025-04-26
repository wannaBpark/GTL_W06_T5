#pragma once
#include "Runtime/CoreUObject/UObject/ObjectMacros.h"
#include "Components/ActorComponent.h"
#include <sol/sol.hpp>
#include <filesystem>

class ULuaScriptComponent : public UActorComponent
{
    DECLARE_CLASS(ULuaScriptComponent, UActorComponent)

public:
    ULuaScriptComponent();
    ~ULuaScriptComponent();

    FString BasePath = FString(L"LuaScripts");

    TMap<FString, sol::object> ExposedProperties;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime) override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void InitializeComponent() override;

    // Lua 함수 호출 메서드
    void CallLuaFunction(const FString& FunctionName);

    FString GetScriptPath() const { return ScriptPath; }
    void SetScriptPath(const FString& InScriptPath);

    FString GetDisplayName() const { return DisplayName; }
    void SetDisplayName(const FString& InDisplayName) { DisplayName = InDisplayName; }


private:
    // Lua 환경 초기화
    void InitializeLuaState();

    // Lua-Engine 바인딩
    void BindEngineAPI();

    FString ScriptPath;
    FString DisplayName;

    sol::state LuaState;
    bool bScriptValid = false;

    std::filesystem::file_time_type LastWriteTime;
    bool CheckFileModified();
    void ReloadScript();
};
