#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UInputComponent : public UObject
{
    DECLARE_CLASS(UInputComponent, UObject)

public:
    UInputComponent();

    // 실제 바인딩된 델리게이트
    TMap<FString, TMulticastDelegate<void()>>       ActionBindings;
    TMap<FString, TMulticastDelegate<void(float)>>  AxisBindings;

    TMap<FString, TArray<EKeys::Type>> ActionMappings;
    TMap<FString, TArray<TPair<EKeys::Type, float>>> AxisMappings;

    // 원시 핸들러에서 호출
    void HandleKeyDown(const FKeyEvent& E);
    void HandleKeyUp(const FKeyEvent& E);

    // 바인딩 함수들
    void BindAction(const FString& ActionName, std::function<void()> Func);
    void BindAxis(const FString& AxisName, std::function<void(float)> Func);
};
