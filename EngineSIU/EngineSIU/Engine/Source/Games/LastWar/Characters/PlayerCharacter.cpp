#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"  
#include "Components/StaticMeshComponent.h"
#include "Engine/FLoaderOBJ.h"

APlayerCharacter::APlayerCharacter()
{
    BodyMesh = AddComponent<UStaticMeshComponent>("Player");
    BodyMesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Reference/Reference.obj"));
    BodyMesh->SetupAttachment(RootComponent);

    FollowCamera = AddComponent<UCameraComponent>("PlayerCamera");
    FollowCamera->SetupAttachment(RootComponent);
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector Forward = GetActorForwardVector();
    FVector BackOffset = -Forward * 3.0f;
    FVector UpOffset = FVector(0.0f, 0.0f, 3.0f);
    FVector CamLocation = GetActorLocation() + BackOffset + UpOffset;
    FollowCamera->SetLocation(CamLocation);
    FollowCamera->SetRotation(GetActorRotation());
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Bind input actions and axes here  
    PlayerInputComponent->BindAxis("MoveForward", [this](float Value) { MoveForward(Value); });
    PlayerInputComponent->BindAxis("MoveRight", [this](float Value) { MoveRight(Value); });
}

void APlayerCharacter::MoveForward(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorForwardVector(), Value);
    }
}

void APlayerCharacter::MoveRight(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorRightVector(), Value);
    }
}
