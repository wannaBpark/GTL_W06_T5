#pragma once

#include "Container/Map.h"
#include "Container/String.h"
#include "sol/sol.hpp"


class FLuaScriptManager
{

private:
    sol::state LuaState;
    static TMap<FString, sol::table> ScriptCacheMap;

public:
    FLuaScriptManager();

private:
    void SetLuaDefaultTypes();

public:

    static FLuaScriptManager& Get();

    sol::state& GetLua();
    sol::table CreateLuaTable(const FString& ScriptName);

};

