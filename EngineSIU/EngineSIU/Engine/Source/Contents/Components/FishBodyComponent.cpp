
#include "FishBodyComponent.h"

#include "Engine/FObjLoader.h"

UFishBodyComponent::UFishBodyComponent()
{
}

void UFishBodyComponent::TickComponent(float DeltaTime)
{
    UStaticMeshComponent::TickComponent(DeltaTime);
}

void UFishBodyComponent::InitializeComponent()
{
    UStaticMeshComponent::InitializeComponent();

    SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Fish/Fish_Front.obj"));
}
