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



    // TODO 임시 코드임. (Class Table에 생성되지 않으면 안넣어짐.)
    auto BoxComponent = AddComponent<UBoxComponent>();
    BoxComponent->DestroyComponent();

    auto SphereComponent = AddComponent<USphereComponent>();
    SphereComponent->DestroyComponent();

    auto CapsuleComponent = AddComponent<UCapsuleComponent>();
    CapsuleComponent->DestroyComponent();
    
    // End Test
}

void ACube::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //SetActorRotation(GetActorRotation() + FRotator(0, 0, 1));

}
