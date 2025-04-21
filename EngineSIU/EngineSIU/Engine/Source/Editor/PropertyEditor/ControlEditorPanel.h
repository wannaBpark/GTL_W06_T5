#pragma once
#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"
#include "LightGridGenerator.h"

class ControlEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void CreateMenuButton(ImVec2 ButtonSize, ImFont* IconFont);
    void CreateModifyButton(ImVec2 ButtonSize, ImFont* IconFont);
    static void CreateFlagButton();
    static void CreatePIEButton(ImVec2 ButtonSize, ImFont* IconFont);
    static void CreateSRTButton(ImVec2 ButtonSize);
    void CreateLightSpawnButton(ImVec2 InButtonSize, ImFont* IconFont);
    
private:
    float Width = 300, Height = 100;
    bool bOpenMenu = false;

    float* FOV = nullptr;
    float CameraSpeed = 0.0f;
    float GridScale = 1.0f;
    FLightGridGenerator LightGridGenerator;
};

