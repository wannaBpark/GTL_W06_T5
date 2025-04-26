#include "Cube.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Shapes/BoxComponent.h"
#include "Components/Shapes/CapsuleComponent.h"
#include "Components/Shapes/SphereComponent.h"

#include "Engine/FLoaderOBJ.h"

#include "GameFramework/Actor.h"

ACube::ACube()
{
    // Begin Test
    //StaticMeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/helloBlender.obj"));
    StaticMeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Reference/Reference.obj"));
    AddComponent<UBoxComponent>("CUBECOLLISION_0");
    AddComponent<UCapsuleComponent>("CAPSULECOLLISION_0");
    AddComponent<USphereComponent>("SPHERECOLLISION_0");
    // End Test
}

void ACube::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //SetActorRotation(GetActorRotation() + FRotator(0, 0, 1));

}
