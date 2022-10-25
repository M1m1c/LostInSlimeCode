
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KeyHolderComp.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GP4TEAM3_API UKeyHolderComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UKeyHolderComp(){ PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintCallable)
	int GetNumKeyes() { return numKeyes; }
	UFUNCTION(BlueprintCallable)
	void SetNumKeyes(int value) { numKeyes = value; }
	UFUNCTION(BlueprintCallable)
	void AddKeyes(int value) { numKeyes += value; }

protected:

	int numKeyes = 0;
};
