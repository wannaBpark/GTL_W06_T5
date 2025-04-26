#include "PlayerController.h"
#include "Camera/CameraComponent.h"
#include "PlayerInput.h"

void APlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!PlayerInput)
    {
        PlayerInput = FObjectFactory::ConstructObject<UPlayerInput>(this);
    }

    PlayerInput->InitializeDefaultMappings();
}

void APlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    SetupInputBindings();
}

void APlayerController::AddYawInput(float Value)
{
    if (Value < KINDA_SMALL_NUMBER)
        return;

    // Z축(Yaw) 회전만 조절
    FRotator NewRot = GetControlRotation();
    NewRot.Yaw += Value;
    SetControlRotation(NewRot);
}

void APlayerController::AddPitchInput(float Value)
{
    if (Value < KINDA_SMALL_NUMBER)
        return;

    // X축(Pitch) 회전만 조절
    FRotator NewRot = GetControlRotation();
    NewRot.Pitch += Value;
    SetControlRotation(NewRot);
}

bool APlayerController::IsInputKeyDown(EKeys::Type Key) const
{
    if (PlayerInput)
    {
        // 키 상태를 PlayerInput에서 확인
        return PlayerInput->IsPressed(Key);
    }
    return false;
}

void APlayerController::PushInputComponent(UInputComponent* InputComponent)
{
    if (InputComponent)
    {
        // InputComponent를 스택에 추가
        InputComponentStack.Add(InputComponent);
    }
}

void APlayerController::PopInputComponent(UInputComponent* InputComponent)
{
    if (InputComponent)
    {
        // InputComponent를 스택에서 제거
        InputComponentStack.Remove(InputComponent);
    }
}

void APlayerController::InputKey(EKeys::Type Key, EInputEvent EventType)
{
    PlayerInput->InputKey(Key, EventType);
}

void APlayerController::InputAxis(EKeys::Type Key, float Delta, float DeltaTime)
{
    PlayerInput->InputAxis(Key, Delta, DeltaTime);
}

void APlayerController::SetupInputBindings()
{
    // 카메라 조작용 축 바인딩
    if (InputComponent)
    {
        InputComponent->BindAxis("Turn", [this](float V) { AddYawInput(V); });
        InputComponent->BindAxis("LookUp", [this](float V) { AddPitchInput(V); });
    }
}
