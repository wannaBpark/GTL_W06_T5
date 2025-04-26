#pragma once

#include "ActorComponent.h"
#include "sol/sol.hpp"

//#pragma comment( linker, "/entry:WinMainCRTStartup /subsystem:console" )


class AActor;

class ULuaScriptComponent : public UActorComponent
{
    DECLARE_CLASS(ULuaScriptComponent, UActorComponent)

public:
    ULuaScriptComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;

    // 기본 함수는 자체적으로 불러지도록 세팅.
    // 추후 특정 Actor에서 함수를 추가 실행할 때는 Component->ActivateFunction()으로 호출해주기.
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    FString GetScriptName() const { return ScriptName; }
    sol::table& LoadScript();

    template<typename... Args>
    void ActivateFunction(const FString& FunctionName, Args&&... args);

    sol::table& GetLuaEnv() { return LuaEnv; }

private:
    FString ScriptName;
    sol::table LuaEnv;
};

template<typename ...Args>
inline void ULuaScriptComponent::ActivateFunction(const FString& FunctionName, Args && ...args)
{
    if (LuaEnv.valid() && LuaEnv[FunctionName].valid())
    {
        LuaEnv[FunctionName](std::forward<Args>(args)...);
    }
}
