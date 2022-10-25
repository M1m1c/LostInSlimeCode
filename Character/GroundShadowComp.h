
#pragma once

#include "CoreMinimal.h"
#include "Components/DecalComponent.h"

#include "GroundShadowComp.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GP4TEAM3_API UGroundShadowComp : public UDecalComponent
{
	GENERATED_BODY()

public:
	UGroundShadowComp();

	void Initalize(TArray<UPrimitiveComponent*> ignoreComps);


	void UpdateShadow(float DeltaTime);

	UFUNCTION()
	void SetShadowVisible(bool b);
protected:
	virtual void BeginPlay() override;

	FVector localStartPos;
	FVector startSize;

	FCollisionQueryParams collisionParams;
	FCollisionShape myColSphere;

	float traceLength = 1000.0f;
	float minGroundShadowWidth;
	float maxGroundShadowWidth;

	AActor* owner;

};
