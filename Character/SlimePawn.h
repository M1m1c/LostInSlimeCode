// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GP4Team3/Enemy/EnemyCharacter.h"
#include "SlimePawn.generated.h"

UCLASS()
class GP4TEAM3_API ASlimePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASlimePawn();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintImplementableEvent)
		void OnFixedTick(const float deltaTime);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnSetTarget"))
		void OnSetTarget(AEnemyCharacter* Target);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnGroundSlamImpact"))
	void OnGroundSlam();

	UFUNCTION(BlueprintImplementableEvent, meta = (DispayName = "OnDashAttack"))
	void OnDashAttack();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDashAttackHitEnemy"))
	void OnDashAttackHit();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnGroundSlamHitEnemy"))
	void OnGroundSlamHit();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnRemoveTarget"))
		void OnRemoveTarget(AEnemyCharacter* Target);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnJump"))
		void OnJump();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDash"))
		void OnDash();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnMovement"))
		void OnMove();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnLanded"))
		void OnLand();

	UFUNCTION(BlueprintCallable)
		void SetSlimeColor(FVector light, FVector dark);

	UFUNCTION(BlueprintCallable)
		void ResetSlimeColor();

	UFUNCTION(BlueprintCallable)
	void UpdateSlimeSize(int newSize);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UMovementComp* movementComp;
	UFUNCTION()
	void ActivateDeformSpring();

	

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void FixedTick();

	UFUNCTION()
	void SpawnSlimeTrailDecal(FVector location, FRotator rotation, FVector size, USceneComponent* attachTarget, float lifeTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class USphereComponent* root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USceneComponent* cameraHolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* cameraArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* cameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UCameraDriverComp* cameraDriver;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UAttackComponent* attackComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UDeformNodeHandeler* deformHandeler;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UKeyHolderComp* keyHolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UGroundShadowComp* groundShadowComp;

	UPROPERTY(EditDefaultsOnly)
	const TSubclassOf<class ASlimeTrailDecal> slimeTrailBP;

	class UStaticMeshComponent* mesh;
	
	FTimerHandle FixedTickHandle;
	const float fixedTickDeltaTime=0.016f;

	class UMaterialInstanceDynamic* materialInstance;
	FVector lightColor = FVector(0.303795f, 0.741667f, 0.282805f);
	FVector darkColor = FVector(0.067591f, 0.216667f, 0.188904f);
	FVector originalLightColor = FVector(0.303795f, 0.741667f, 0.282805f);
	FVector originalDarkColor = FVector(0.067591f, 0.216667f, 0.188904f);

	UWorld* worldPtr;
};
