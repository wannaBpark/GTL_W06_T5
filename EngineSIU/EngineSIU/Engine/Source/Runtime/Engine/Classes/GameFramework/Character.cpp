#include "Character.h"  
#include "Components/InputComponent.h"  
#include "Controller.h"
#include "Components/Shapes/CapsuleComponent.h"

ACharacter::ACharacter()  
{  
    CollisionCapsule = AddComponent<UCapsuleComponent>("CollisionCapsule");
    RootComponent = CollisionCapsule;
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
}  

void ACharacter::PossessedBy(AController* NewController)  
{  
    Super::PossessedBy(NewController);  
}  

void ACharacter::UnPossessed()  
{  
    Super::UnPossessed();  
}  
