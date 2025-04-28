#pragma once
#include "GameFramework/Actor.h" 
#include "Classes/Components/InputComponent.h"

class APlayerController : public AActor
{
    DECLARE_CLASS(APlayerController, AActor)
public:
    APlayerController();
    ~APlayerController();

    virtual void BeginPlay();

    virtual void Tick(float DeltaTime) override;
    void ProcessInput(float DeltaTime) const;

    virtual void Destroyed();

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

    UInputComponent* GetInputComponent() const { return InputComponent; }

    virtual void Possess(AActor* InActor);

    virtual void UnPossess();
    
    virtual void BindAction(const FString& Key, const std::function<void(float)>& Callback);
protected:
    UPROPERTY
    (UInputComponent*, InputComponent, = nullptr)

    virtual void SetupInputComponent();

    AActor* CurrentPossess = nullptr;

    bool bHasPossessed = false;
};

