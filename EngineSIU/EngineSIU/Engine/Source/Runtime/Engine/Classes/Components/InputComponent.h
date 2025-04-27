#pragma once

#include "Core/Container/Map.h"
#include "Delegates/DelegateCombination.h"
#include "Runtime/InputCore/InputCoreTypes.h"
#include "Components/ActorComponent.h"

class UInputComponent : public UActorComponent
{
    DECLARE_CLASS(UInputComponent, UActorComponent)
    struct FInputActionBinding
    {
        TMulticastDelegate<void()> PressedDelegate;
        TMulticastDelegate<void()> ReleasedDelegate;
        TMulticastDelegate<void()> RepeatDelegate;
        bool bPrevState = false;
    };

    TMap<EKeys::Type, FInputActionBinding> ActionBindings;

public:
    UInputComponent() = default;
    ~UInputComponent() = default;
    FDelegateHandle BindAction(EKeys::Type Key, EInputEvent EventType, std::function<void()> Callback);
    void ProcessInput(float DeltaTime);
    uint8 EKeysToVirtualKey(EKeys::Type);
};
