#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

struct FInputAxisKeyMapping
{
    FName AxisName;
    EKeys::Type Key;
    float Scale = 1.f;
};

struct FInputActionKeyMapping
{
    FName ActionName;
    EKeys::Type Key;
};

class UPlayerInput : public UObject
{
    DECLARE_CLASS(UPlayerInput, UObject)

public:
    UPlayerInput() = default;

    // === 입력 상태 ===
    TMap<EKeys::Type, bool> KeyStates;             // 키가 눌렸는지 여부
    TMap<EKeys::Type, float> KeyAxisValues;         // 축 값

    // === 액션/축 매핑 ===
    UPROPERTY
    (TArray<FInputActionKeyMapping>, ActionMappings, = {})
    
    UPROPERTY
    (TArray<FInputAxisKeyMapping>, AxisMappings, = {})

    // === 주요 기능 ===

    /** 키가 눌려있는지 확인 */
    bool IsPressed(EKeys::Type Key) const;

    /** 축 값 읽기 */
    float GetAxisValue(FName AxisName) const;

    /** 키 이벤트 수신 */
    void InputKey(EKeys::Type Key, EInputEvent EventType, float AmountDepressed = 1.0f);

    /** 축 이벤트 수신 */
    void InputAxis(EKeys::Type Key, float Delta);

protected:
    /** 매 프레임 입력 상태 초기화 */
    void FlushPressedKeys();
};

