#include "Cube.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Engine/FObjLoader.h"

#include "GameFramework/Actor.h"

ACube::ACube()
{
    // Begin Test
    //StaticMeshComponent->SetStaticMesh(FManagerGetStaticMesh(L"Contents/helloBlender.obj"));
    StaticMeshComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Reference/Reference.obj"));
    // End Test

    // TODO 임시 코드임. (Class Table에 생성되지 않으면 안넣어짐.)
    auto BoxComponent = AddComponent<UBoxComponent>();
    BoxComponent->DestroyComponent();

    auto SphereComponent = AddComponent<USphereComponent>();
    SphereComponent->DestroyComponent();

    auto CapsuleComponent = AddComponent<UCapsuleComponent>();
    CapsuleComponent->DestroyComponent();
}

void ACube::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //SetActorRotation(GetActorRotation() + FRotator(0, 0, 1));

}
