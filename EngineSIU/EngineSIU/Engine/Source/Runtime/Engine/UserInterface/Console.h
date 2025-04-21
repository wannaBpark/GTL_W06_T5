#pragma once
#include "Container/Array.h"
#include "D3D11RHI/GraphicDevice.h"
#include "HAL/PlatformType.h"
#include "ImGUI/imgui.h"
#include "PropertyEditor/IWindowToggleable.h"

#define UE_LOG Console::GetInstance().AddLog

enum class LogLevel : uint8
{
    Display,
    Warning,
    Error
};

class StatOverlay
{
public:
    // @todo Stat-FPS Default 설정 복구 (showFPS = false, showRender = false)
    bool ShowFPS = true;
    bool ShowMemory = false;
    bool ShowLight = false;
    bool ShowRender = true;

    void ToggleStat(const std::string& Command);
    void Render(ID3D11DeviceContext* Context, UINT Width, UINT Height) const;
};

class Console : public IWindowToggleable
{
private:
    Console() = default;

public:
    // 복사 방지
    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;
    Console(Console&&) = delete;
    Console& operator=(Console&&) = delete;

public:
    static Console& GetInstance(); // 참조 반환으로 변경

    void Clear();
    void AddLog(LogLevel Level, const char* Format, ...);
    void Draw();
    void ExecuteCommand(const std::string& Command);
    void OnResize(HWND hWnd);

    virtual void Toggle() override
    {
        if (bWasOpen)
        {
            bWasOpen = false;
        }
        else
        {
            bWasOpen = true;
        }
    }

public:
    struct LogEntry
    {
        LogLevel Level;
        FString Message;
    };

    TArray<LogEntry> Items;
    TArray<FString> History;
    int32 HistoryPos = -1;
    char InputBuf[256] = "";
    bool ScrollToBottom = false;

    ImGuiTextFilter Filter; // 필터링을 위한 ImGuiTextFilter

    bool ShowLogTemp = true; // LogTemp 체크박스
    bool ShowWarning = true; // Warning 체크박스
    bool ShowError = true;   // Error 체크박스

    bool bWasOpen = true;

    StatOverlay Overlay;

private:
    bool bExpand = true;
    UINT Width;
    UINT Height;
};
