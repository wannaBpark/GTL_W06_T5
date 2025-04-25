#include "PlayerInput.h"

bool UPlayerInput::IsPressed(EKeys::Type Key) const
{
    return false;
}

float UPlayerInput::GetAxisValue(FName AxisName) const
{
    return 0.0f;
}

void UPlayerInput::InputKey(EKeys::Type Key, EInputEvent EventType, float AmountDepressed)
{
}

void UPlayerInput::InputAxis(EKeys::Type Key, float Delta)
{
}

void UPlayerInput::FlushPressedKeys()
{
}
