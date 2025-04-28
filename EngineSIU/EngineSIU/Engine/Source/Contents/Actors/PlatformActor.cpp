
#include "PlatformActor.h"

#include "Engine/FObjLoader.h"

APlatformActor::APlatformActor()
{
    BoxComponent = AddComponent<UBoxComponent>();
    RootComponent = BoxComponent;

    MeshComponent = AddComponent<UStaticMeshComponent>();
    MeshComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/CubePrimitive.obj"));
    MeshComponent->SetupAttachment(BoxComponent);
}
