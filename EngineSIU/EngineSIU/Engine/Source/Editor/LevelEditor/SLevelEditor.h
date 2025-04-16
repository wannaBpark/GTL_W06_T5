#pragma once
#include <sstream>
#include "Define.h"
#include "Container/Map.h"

class SSplitterH;
class SSplitterV;
class UWorld;
class FEditorViewportClient;


class SLevelEditor
{
public:
    SLevelEditor();

    void Initialize(uint32 InEditorWidth, uint32 InEditorHeight);
    void Tick(float DeltaTime);
    void Release();

    void ResizeEditor(uint32 InEditorWidth, uint32 InEditorHeight);
    void SelectViewport(const FVector2D& Point);

    void ResizeViewports();
    void SetEnableMultiViewport(bool bIsEnable);
    bool IsMultiViewport() const;

private:
    SSplitterH* HSplitter;
    SSplitterV* VSplitter;
    
    std::shared_ptr<FEditorViewportClient> ViewportClients[4];
    std::shared_ptr<FEditorViewportClient> ActiveViewportClient;

    /** 우클릭 시 캡처된 마우스 커서의 초기 위치 (스크린 좌표계) */
    FVector2D MousePinPosition;

    /** 우클릭이 눌려있는지 여부 */
    bool bIsPressedMouseRightButton = false;

    bool bMultiViewportMode;
    
    uint32 EditorWidth;
    uint32 EditorHeight;

public:
    std::shared_ptr<FEditorViewportClient>* GetViewports() { return ViewportClients; }
    std::shared_ptr<FEditorViewportClient> GetActiveViewportClient() const
    {
        return ActiveViewportClient;
    }
    void SetActiveViewportClient(const std::shared_ptr<FEditorViewportClient>& InViewportClient)
    {
        ActiveViewportClient = InViewportClient;
    }
    void SetActiveViewportClient(int Index)
    {
        ActiveViewportClient = ViewportClients[Index];
    }

    //Save And Load
private:
    const FString IniFilePath = "editor.ini";

public:
    void LoadConfig();
    void SaveConfig();

private:
    TMap<FString, FString> ReadIniFile(const FString& FilePath);
    void WriteIniFile(const FString& FilePath, const TMap<FString, FString>& Config);

    template <typename T>
    T GetValueFromConfig(const TMap<FString, FString>& Config, const FString& Key, T DefaultValue) {
        if (const FString* Value = Config.Find(Key))
        {
            std::istringstream iss(**Value);
            T ConfigValue;
            if (iss >> ConfigValue)
            {
                return ConfigValue;
            }
        }
        return DefaultValue;
    }
};
