#include "Character.h"  
#include "Components/InputComponent.h"  
#include "Controller.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FLoaderOBJ.h"

ACharacter::ACharacter()  
{  
    StaticMeshComponent = AddComponent<UStaticMeshComponent>("StaticMeshComponent_0");
    RootComponent = StaticMeshComponent;
    StaticMeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Reference/Reference.obj"));
}  

void ACharacter::BeginPlay()  
{  
   Super::BeginPlay();  
}  

void ACharacter::Tick(float DeltaTime)  
{  
   Super::Tick(DeltaTime);  
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
