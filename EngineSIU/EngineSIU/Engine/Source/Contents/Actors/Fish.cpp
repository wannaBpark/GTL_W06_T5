
#include "Fish.h"

#include "PlatformActor.h"
#include "Components/SphereComponent.h"
#include "Contents/Components/FishTailComponent.h"
#include "Contents/Components/FishBodyComponent.h"
#include "Engine/FObjLoader.h"

AFish::AFish()
{
    
}

void AFish::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
    
    SphereComponent = AddComponent<USphereComponent>();
    SetRootComponent(SphereComponent);

    FishBody = AddComponent<UFishBodyComponent>();
    FishBody->SetupAttachment(SphereComponent);
    
    FishTail = AddComponent<UFishTailComponent>();
    FishTail->SetupAttachment(FishBody);
}

UObject* AFish::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    return NewActor;
}

void AFish::BeginPlay()
{
    APlayer::BeginPlay();

    Velocity = FVector(0.f, 0.f, JumpZVelocity);

    OnActorBeginOverlap.AddLambda(
        [this](AActor* OverlappedActor, AActor* OtherActor)
        {
            ActorBeginOverlap(OverlappedActor, OtherActor);
        }
    );
}

void AFish::Tick(float DeltaTime)
{
    APlayer::Tick(DeltaTime);

    Move(DeltaTime);

    RotateMesh();
}

void AFish::Move(float DeltaTime)
{
    FVector NextVelocity = Velocity;
    NextVelocity.Z += Gravity * DeltaTime;

    Velocity = NextVelocity;
    
    FVector NewLocation = GetActorLocation() + Velocity * DeltaTime;
    SetActorLocation(NewLocation);
}

void AFish::RotateMesh()
{
    const float VelocityZ = Velocity.Z;

    float RotFactor = FMath::Clamp(VelocityZ, -MeshRotSpeed, MeshRotSpeed);
    
    if (UFishBodyComponent* MeshComp = GetComponentByClass<UFishBodyComponent>())
    {
        // 현재 PIE 모드에서 맴버 변수를 접근할 수 없기 때문에 이렇게 접근 함.
        FRotator CompRotation = MeshComp->GetRelativeRotation();
        CompRotation.Roll = RotFactor * MeshPitchMax + 180.f;
        CompRotation.Pitch = 0.1f;
        
        MeshComp->SetRelativeRotation(CompRotation);
    }
}

void AFish::ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    if (OtherActor->IsA<APlatformActor>())
    {
        Velocity.Z = JumpZVelocity;
    }
}
