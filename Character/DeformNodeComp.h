
#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "NodeLocation.h"

#include "DeformNodeComp.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GP4TEAM3_API UDeformNodeComp : public USphereComponent
{
	GENERATED_BODY()

public:
	UDeformNodeComp();

	void Initalize(float springStiffnes = 0.65f, float springDampening = 0.85f);

	void ApplyForce(FVector force);

	void UpdateNodePos(float DeltaTime);

	ENodeLocation GetNodeLocation() { return nodeLocaiton; }
	FVector GetLocalStartPos() { return localStartPos; }
	FVector GetVelocity() { return velocity; }

protected:
	virtual void BeginPlay() override;

	FVector localStartPos;

	UPROPERTY(EditDefaultsOnly)
	ENodeLocation nodeLocaiton;

	UPROPERTY(EditDefaultsOnly)
	float lerpSpeed = 100.f;

	float mass = 1.f;

	FVector velocity = FVector::ZeroVector;
	float stiffnes = 0.65f;
	float dampening = 0.85f;
	float springRestLength = 0.f;

	AActor* owner;

};
