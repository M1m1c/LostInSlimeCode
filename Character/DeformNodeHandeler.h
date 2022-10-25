
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NodeLocation.h"

#include "DeformNodeHandeler.generated.h"


USTRUCT(Blueprintable)
struct FDeformSpring
{
	GENERATED_BODY()
public:
	FDeformSpring() {};
	void Initalize(AActor* Owner, TArray<class UDeformNodeComp*> deformNodes);
	void UpdateSpring(float deltaTime);

	UPROPERTY(EditAnywhere)
		ENodeLocation NodeARef;

	UPROPERTY(EditAnywhere)
		ENodeLocation NodeBRef;

	UPROPERTY(EditAnyWhere)
		float springStiffnes = 0.65f;
	UPROPERTY(EditAnyWhere)
		float springDampening = 0.85f;

protected:
	class UDeformNodeComp* nodeA;
	class UDeformNodeComp* nodeB;
	float springRestLength = 0.f;
	FVector localStartPos;
	AActor* owner;

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GP4TEAM3_API UDeformNodeHandeler : public UActorComponent
{
	GENERATED_BODY()

public:
	UDeformNodeHandeler();

	void Initalize();

	void UpdateDeformSprings(float DeltaTime);

	bool bSpringsActive = true;
	TArray<class UDeformNodeComp*> deformNodes;
protected:

	UPROPERTY(EditDefaultsOnly)
		TArray<FDeformSpring> deformSprings;


	

};
