
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SlimeTrailDecal.generated.h"

UCLASS()
class GP4TEAM3_API ASlimeTrailDecal : public AActor
{
	GENERATED_BODY()

public:
	ASlimeTrailDecal();
	void Initalize(FVector size, float lifeTime, FVector lightColor, FVector darkColor);
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		float GetMappedLifetime();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USceneComponent* root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UDecalComponent* decal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bInitalized = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float currentLifeTime;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float startingLifeTime;

	class UMaterialInstanceDynamic* materialInstance;

	FVector startingSize;
	FVector targetSize;
};
