#include "InputComponent.h"
#include "Engine/EditorEngine.h"
#include "World/World.h"

UInputComponent::UInputComponent()
{
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    KeyDownHandle = Handler->OnKeyDownDelegate.AddLambda(
        [this](const FKeyEvent& E)
        {
            HandleKeyDown(E);
        }
    );

    KeyUpHandle = Handler->OnKeyUpDelegate.AddLambda(
        [this](const FKeyEvent& InKeyEvent)
        {
            HandleKeyUp(InKeyEvent);
        }
    );
}

void UInputComponent::HandleKeyDown(const FKeyEvent& E)
{
    if (!GEngine || !GEngine->ActiveWorld || GEngine->ActiveWorld->WorldType != EWorldType::PIE)
        return;

    // (1) 액션 매핑 테이블 순회
    for (auto& Pair : ActionMappings)
    {
        const FString& ActionName = Pair.Key;
        const TArray<EKeys::Type>& Keys = Pair.Value;

        if (Keys.Contains(E.GetKey()) && E.GetInputEvent() == IE_Pressed)
        {
            if (auto* Del = ActionBindings.Find(ActionName))
            {
                Del->Broadcast();  // BindAction 에 등록된 모든 콜백 실행
            }
        }
    }

    // (2) 축 매핑 테이블 순회
    for (auto& Pair : AxisMappings)
    {
        const FString& AxisName = Pair.Key;
        const TArray<TPair<EKeys::Type, float>>& KeyMap = Pair.Value;

        for (auto& KeyScale : KeyMap)
        {
            if (KeyScale.Key == E.GetKey())
            {
                if (auto* Del = AxisBindings.Find(AxisName))
                {
                    Del->Broadcast(KeyScale.Value);
                }
            }
        }
    }
}

void UInputComponent::HandleKeyUp(const FKeyEvent& E)
{
    if (!GEngine || !GEngine->ActiveWorld || GEngine->ActiveWorld->WorldType != EWorldType::PIE)
        return;

    // (1) 액션 매핑 테이블 순회: 키 해제(Released) 이벤트만 처리
    for (auto& Pair : ActionMappings)
    {
        const FString& ActionName = Pair.Key;
        const TArray<EKeys::Type>& Keys = Pair.Value;

        if (Keys.Contains(E.GetKey()) && E.GetInputEvent() == IE_Released)
        {
            if (auto* Del = ActionBindings.Find(ActionName))
            {
                Del->Broadcast();  // BindAction 에 등록된 모든 콜백 실행
            }
        }
    }

    // (2) 축 매핑 테이블 순회: 키 해제 시 축 값을 0으로 리셋
    for (auto& Pair : AxisMappings)
    {
        const FString& AxisName = Pair.Key;
        const TArray<TPair<EKeys::Type, float>>& KeyMap = Pair.Value;

        for (auto& KeyScale : KeyMap)
        {
            if (KeyScale.Key == E.GetKey())
            {
                if (auto* Del = AxisBindings.Find(AxisName))
                {
                    Del->Broadcast(0.0f);
                }
            }
        }
    }
}

void UInputComponent::BindAction(const FString& ActionName, std::function<void()> Func)
{
    ActionBindings.FindOrAdd(ActionName).AddLambda(std::move(Func));
}

void UInputComponent::BindAxis(const FString& AxisName, std::function<void(float)> Func)
{
    AxisBindings.FindOrAdd(AxisName).AddLambda(std::move(Func));
}
