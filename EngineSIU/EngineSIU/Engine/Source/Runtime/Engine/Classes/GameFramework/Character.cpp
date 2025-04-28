#include "Character.h"  
#include "Components/InputComponent.h"  
#include "Controller.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "LevelEditor/SLevelEditor.h"
#include "Editor/UnrealEd/EditorViewportClient.h"

ACharacter::ACharacter()  
{  
    RootScene = AddComponent<USceneComponent>("RootScene");
    RootComponent = RootScene;

    BodyMesh = AddComponent<UStaticMeshComponent>("Player");
    BodyMesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Reference/Reference.obj"));
    BodyMesh->SetupAttachment(RootComponent);

    //CameraBoom = AddComponent<USpringArmComponent>("CameraBoom");
    //CameraBoom->SetupAttachment(RootComponent);
    //CameraBoom->TargetArmLength = 300.f;

    FollowCamera = AddComponent<UCameraComponent>("PlayerCamera");
    FollowCamera->SetupAttachment(RootComponent);
}  

void ACharacter::BeginPlay()  
{  
    Super::BeginPlay();  
}  

void ACharacter::Tick(float DeltaTime)  
{  
    Super::Tick(DeltaTime);  

    FVector Forward = GetActorForwardVector();
    FVector BackOffset = -Forward * 3.0f;
    FVector UpOffset = FVector(0.0f, 0.0f, 3.0f);
    FVector CamLocation = GetActorLocation() + BackOffset + UpOffset;

    FollowCamera->SetLocation(CamLocation);
    FollowCamera->SetRotation(GetActorRotation());

}

void ACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)  
{  
    Super::SetupPlayerInputComponent(PlayerInputComponent);  

    // Bind input actions and axes here  
    PlayerInputComponent->BindAxis("MoveForward", [this](float Value) { MoveForward(Value); });
    PlayerInputComponent->BindAxis("MoveRight", [this](float Value) { MoveRight(Value); });
    PlayerInputComponent->BindAxis("MoveLeft", [this](float Value) { MoveLeft(Value); });
    PlayerInputComponent->BindAxis("MoveBackward", [this](float Value) { MoveBackward(Value); });
}  

void ACharacter::PossessedBy(AController* NewController)  
{  
    Super::PossessedBy(NewController);  
}  

void ACharacter::UnPossessed()  
{  
    Super::UnPossessed();  
}  

void ACharacter::MoveForward(float Value)  
{  
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorForwardVector(), Value);
    }
}  

void ACharacter::MoveRight(float Value)  
{  
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorRightVector(), Value);
    }
}  

void ACharacter::MoveLeft(float Value)  
{  
    if (Value != 0.0f)
    {
        AddMovementInput(-GetActorRightVector(), Value);
    }
}  

void ACharacter::MoveBackward(float Value)  
{
    if (Value != 0.0f)
    {
        AddMovementInput(-GetActorForwardVector(), Value);
    }
}
