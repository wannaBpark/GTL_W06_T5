#include "InputComponent.h"
#include "Classes/Components/InputComponent.h"

FDelegateHandle UInputComponent::BindAction(EKeys::Type Key, EInputEvent EventType, std::function<void()> Callback)
{
    FInputActionBinding& Binding = ActionBindings.FindOrAdd(Key);

    switch (EventType)
    {
    case EInputEvent::IE_Pressed:
        return Binding.PressedDelegate.AddLambda([Callback]() { Callback(); });
    case EInputEvent::IE_Released:
        return Binding.ReleasedDelegate.AddLambda([Callback]() { Callback(); });
    case EInputEvent::IE_Repeat:
        return Binding.RepeatDelegate.AddLambda([Callback]() { Callback(); });
    }
    // TODO Default Case & Invalid EventType
}


// [TEMP] Convert EKeys type to VK type to check with GetKeyState

uint8 UInputComponent::EKeysToVirtualKey(EKeys::Type Key)
{
    switch (Key)
    {
        // 알파벳 (A-Z)
    case EKeys::A: return 'A';
    case EKeys::B: return 'B';
    case EKeys::C: return 'C';
    case EKeys::D: return 'D';
    case EKeys::E: return 'E';
    case EKeys::F: return 'F';
    case EKeys::G: return 'G';
    case EKeys::H: return 'H';
    case EKeys::I: return 'I';
    case EKeys::J: return 'J';
    case EKeys::K: return 'K';
    case EKeys::L: return 'L';
    case EKeys::M: return 'M';
    case EKeys::N: return 'N';
    case EKeys::O: return 'O';
    case EKeys::P: return 'P';
    case EKeys::Q: return 'Q';
    case EKeys::R: return 'R';
    case EKeys::S: return 'S';
    case EKeys::T: return 'T';
    case EKeys::U: return 'U';
    case EKeys::V: return 'V';
    case EKeys::W: return 'W';
    case EKeys::X: return 'X';
    case EKeys::Y: return 'Y';
    case EKeys::Z: return 'Z';

        // 숫자 (0-9)
    case EKeys::Zero:  return '0';
    case EKeys::One:   return '1';
    case EKeys::Two:   return '2';
    case EKeys::Three: return '3';
    case EKeys::Four:  return '4';
    case EKeys::Five:  return '5';
    case EKeys::Six:   return '6';
    case EKeys::Seven: return '7';
    case EKeys::Eight: return '8';
    case EKeys::Nine:  return '9';

        // 기능 키
    case EKeys::F1:  return VK_F1;
    case EKeys::F2:  return VK_F2;
    case EKeys::F3:  return VK_F3;
    case EKeys::F4:  return VK_F4;
    case EKeys::F5:  return VK_F5;
    case EKeys::F6:  return VK_F6;
    case EKeys::F7:  return VK_F7;
    case EKeys::F8:  return VK_F8;
    case EKeys::F9:  return VK_F9;
    case EKeys::F10: return VK_F10;
    case EKeys::F11: return VK_F11;
    case EKeys::F12: return VK_F12;

        // 특수 키
    case EKeys::BackSpace:   return VK_BACK;
    case EKeys::Tab:         return VK_TAB;
    case EKeys::Enter:       return VK_RETURN;
    case EKeys::Pause:       return VK_PAUSE;
    case EKeys::CapsLock:    return VK_CAPITAL;
    case EKeys::Escape:      return VK_ESCAPE;
    case EKeys::SpaceBar:    return VK_SPACE;
    case EKeys::PageUp:      return VK_PRIOR;
    case EKeys::PageDown:    return VK_NEXT;
    case EKeys::End:         return VK_END;
    case EKeys::Home:        return VK_HOME;
    case EKeys::Left:        return VK_LEFT;
    case EKeys::Up:          return VK_UP;
    case EKeys::Right:       return VK_RIGHT;
    case EKeys::Down:        return VK_DOWN;
    case EKeys::Insert:      return VK_INSERT;
    case EKeys::Delete:      return VK_DELETE;

        // 넘패드
    case EKeys::NumPadZero:  return VK_NUMPAD0;
    case EKeys::NumPadOne:   return VK_NUMPAD1;
    case EKeys::NumPadTwo:   return VK_NUMPAD2;
    case EKeys::NumPadThree: return VK_NUMPAD3;
    case EKeys::NumPadFour:  return VK_NUMPAD4;
    case EKeys::NumPadFive:  return VK_NUMPAD5;
    case EKeys::NumPadSix:   return VK_NUMPAD6;
    case EKeys::NumPadSeven: return VK_NUMPAD7;
    case EKeys::NumPadEight: return VK_NUMPAD8;
    case EKeys::NumPadNine:  return VK_NUMPAD9;
    case EKeys::Multiply:    return VK_MULTIPLY;
    case EKeys::Add:         return VK_ADD;
    case EKeys::Subtract:    return VK_SUBTRACT;
    case EKeys::Decimal:     return VK_DECIMAL;
    case EKeys::Divide:      return VK_DIVIDE;

        // 특수 문자
    case EKeys::Semicolon:    return VK_OEM_1;      // ;
    case EKeys::Equals:       return VK_OEM_PLUS;   // =
    case EKeys::Comma:        return VK_OEM_COMMA;  // ,
    case EKeys::Hyphen:       return VK_OEM_MINUS;  // -
    case EKeys::Period:       return VK_OEM_PERIOD; // .
    case EKeys::Slash:        return VK_OEM_2;      // /
    case EKeys::Tilde:        return VK_OEM_3;      // ~
    case EKeys::LeftBracket:  return VK_OEM_4;      // [
    case EKeys::Backslash:    return VK_OEM_5;      // \
                    case EKeys::RightBracket: return VK_OEM_6;      // ]
    case EKeys::Apostrophe:   return VK_OEM_7;      // '

        // 마우스 버튼
    case EKeys::LeftMouseButton:   return VK_LBUTTON;
    case EKeys::RightMouseButton:  return VK_RBUTTON;
    case EKeys::MiddleMouseButton: return VK_MBUTTON;
    case EKeys::ThumbMouseButton:  return VK_XBUTTON1;
    case EKeys::ThumbMouseButton2: return VK_XBUTTON2;

        // 미지원 키
    case EKeys::Gamepad_Left2D:
    case EKeys::Gamepad_LeftX:
    case EKeys::Gamepad_LeftY:
        // ... (기타 게임패드 키들)
        return 0;

    default:
        return 0;
    }
}

void UInputComponent::ProcessInput(float DeltaTime)
{
    for (auto& [Key, Binding] : ActionBindings)
    {
        //const bool bCurrentState = Need to get key state;
        // TEMP
        uint8 VK = EKeysToVirtualKey(Key);
        const bool bCurrentState = GetKeyState(VK) & 0x8000;

        if (bCurrentState != Binding.bPrevState)
        {
            if (bCurrentState) Binding.PressedDelegate.Broadcast();
            else Binding.ReleasedDelegate.Broadcast();
        }
        else if (bCurrentState)
        {
            Binding.RepeatDelegate.Broadcast();
        }

        Binding.bPrevState = bCurrentState;
    }
}
