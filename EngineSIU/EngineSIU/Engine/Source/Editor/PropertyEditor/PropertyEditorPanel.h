#pragma once

#include "Components/ActorComponent.h"
#include "Components/Shapes/ShapeComponent.h"
#include "UnrealEd/EditorPanel.h"
#include "Math/Rotator.h"

class ULightComponentBase;
class AEditorPlayer;
class USceneComponent;
class USpotLightComponent;
class UPointLightComponent;
class UDirectionalLightComponent;
class UAmbientLightComponent;
class UProjectileMovementComponent;
class UTextComponent;
class UHeightFogComponent;
class UStaticMeshComponent;

// 헬퍼 함수 예시
template<typename Getter, typename Setter>
void DrawColorProperty(const char* Label, Getter Get, Setter Set)
{
    ImGui::PushItemWidth(200.0f);
    const FLinearColor CurrentColor = Get();
    float Col[4] = { CurrentColor.R, CurrentColor.G, CurrentColor.B, CurrentColor.A };

    if (ImGui::ColorEdit4(Label, Col,
        ImGuiColorEditFlags_DisplayRGB |
        ImGuiColorEditFlags_NoSidePreview |
        ImGuiColorEditFlags_NoInputs |
        ImGuiColorEditFlags_Float))
    {
        Set(FLinearColor(Col[0], Col[1], Col[2], Col[3]));
    }
    ImGui::PopItemWidth();
}


class PropertyEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    static void RGBToHSV(float R, float G, float B, float& H, float& S, float& V);
    static void HSVToRGB(float H, float S, float V, float& R, float& G, float& B);

    void RenderForSceneComponent(USceneComponent* SceneComponent, AEditorPlayer* Player) const;
    void RenderForActor(AActor* InActor, USceneComponent* TargetComponent) const;

    /* Static Mesh Settings */
    void RenderForStaticMesh(UStaticMeshComponent* StaticMeshComp) const;
    /* Materials Settings */
    void RenderForMaterial(UStaticMeshComponent* StaticMeshComp);
    void RenderMaterialView(UMaterial* Material);
    void RenderCreateMaterialView();

    void RenderForAmbientLightComponent(UAmbientLightComponent* LightComp) const;
    void RenderForDirectionalLightComponent(UDirectionalLightComponent* LightComponent) const;
    void RenderForPointLightComponent(UPointLightComponent* LightComponent) const;
    void RenderForSpotLightComponent(USpotLightComponent* LightComponent) const;
    void RenderForLightShadowCommon(ULightComponentBase* LightComponent) const;
    
    void RenderForProjectileMovementComponent(UProjectileMovementComponent* ProjectileComp) const;
    void RenderForTextComponent(UTextComponent* TextComponent) const;
    
    void RenderForExponentialHeightFogComponent(UHeightFogComponent* ExponentialHeightFogComp) const;

    void RenderForShapeComponent(UShapeComponent* ShapeComponent) const;

    template<typename T>
        requires std::derived_from<T, UActorComponent>
    T* GetTargetComponent(AActor* SelectedActor, USceneComponent* SelectedComponent);

private:
    float Width = 0, Height = 0;
    /* Material Property */
    int SelectedMaterialIndex = -1;
    int CurMaterialIndex = -1;
    UStaticMeshComponent* SelectedStaticMeshComp = nullptr;
    FObjMaterialInfo tempMaterialInfo;
    bool IsCreateMaterial;

    const FString TemplateFilePath = FString("Contents/Template/LuaTemplate.lua");
};
