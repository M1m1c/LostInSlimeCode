// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MovementComp.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangedNavUp, FVector, Up);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGorundedChange, bool, b);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnSpawnTrail, FVector, location, FRotator, rotation,FVector, size,USceneComponent*,attachTarget,float, lifeTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJump);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDash);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMove);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLand);


USTRUCT(Blueprintable)
struct FPushForce
{
	GENERATED_BODY()
public:
	FPushForce(){};
	FPushForce(FVector inDirection, float inMagnitude, float inDuration)
	{
		direction = inDirection.GetSafeNormal();
		magnitude = inMagnitude;
		duration = FMath::Max(0.f, inDuration);
	};
	FVector direction = FVector::ZeroVector;
	float magnitude = 0.f;
	float duration = 0.0f;
};

USTRUCT(Blueprintable)
struct FNavigationPlane
{
	GENERATED_BODY()
public:
	FNavigationPlane() {};
	FNavigationPlane(FVector forward, FVector right, FVector up )
	{
		forwardVector = forward;
		rightVector = right;
		upVector = up;
	};

	UPROPERTY(BlueprintReadOnly)
	FVector upVector = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector rightVector = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector forwardVector = FVector::ZeroVector;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GP4TEAM3_API UMovementComp : public UActorComponent
{
	GENERATED_BODY()

public:	

	UFUNCTION()
		void ReadVertical(float value);
	UFUNCTION()
		void ReadHorizontal(float value);
	UFUNCTION()
		void InputPressJump();
	UFUNCTION()
		void InputReleaseJump();
	UFUNCTION()
		void InputPressDash();
	UFUNCTION()
		void InputReleaseDash();

	UFUNCTION(BlueprintCallable)
		void TeleportToLocation(FVector location);

	UMovementComp();
	void Initalize(class USceneComponent* cameraPivot,TArray<UPrimitiveComponent*> ignoreComps);
	void UpdateMovement(float deltaTime);
	void DecelerateAttackForce();
	void DoPushForces(float deltaTime);
	void DoDashing(float deltaTime);
	void Dash(FVector direction, float distance);
	bool IsJumping();
	bool IsDashing();
	bool IsNavPlaneDefault();
	bool IsNavPlaneSemiHorizontal();
	bool IsNavPlaneSemiUpsideDown();
	void SetMainForce(FVector newForce);
	void DisableAllForces();
	FVector GetAttackForce();
	void SetAttackForce(FVector NewForce);
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool active = true;
	UFUNCTION(BlueprintCallable)
	FVector GetMainForce() { return mainForce; }

	UFUNCTION(BlueprintCallable)
	FVector GetDirectionForce() { return mainForce + gravForce + jumpSurfaceAddative + jumpForce; }

	UFUNCTION(BlueprintCallable)
	void ApplyPushForce(FVector direction, float magnitude, float duration = 0.0f);

	UFUNCTION(BlueprintCallable)
	bool IsGrounded();
	FOnChangedNavUp OnChangedNavUp;
	FOnGorundedChange OnGroundedChanged;
	FOnSpawnTrail OnSpawnTrail;
	FOnJump OnJump;
	FOnDash OnDash;
	FOnMove OnMove;
	FOnLand OnLand;
	FVector DashDirection;
	
protected:	
	void DoSurfaceChangeChecking();
	void DoJumpingAndFalling(float deltaTime);
	void ExtendJumpWithHeldButton(float deltaTime);
	void DoInputMovement(float deltaTime);
	void ApplyMovementForces(float deltaTime);
	bool IgnorePhysicsCollisions(FHitResult& hit, FVector& velocity, int& Iterations);
	void BecomeGrounded(FVector normal, USceneComponent* compToAttachTo);
	void SetGrounded(bool b);
	void UpdateNavigationPlane(FVector normal);
	void LineTrace(TArray<FHitResult>& OutHits, FVector& start, FVector3d& end);
	
	FVector GetForceAlongPlane(float vertStep, float horiStep, FVector upVector);
	FVector GetForceAlongPlane(FVector force, FVector upVector);

	bool CheckTraceHits(TArray<FHitResult>& hits, FHitResult& outHit);
	bool TraceForNewNavPlane(FVector direction, FHitResult& outHit);
	bool TraceForNewNavPlane(FVector start, FVector direction, FHitResult& outHit);

	struct FHitResult AttemptMove(FVector forceVector, bool bIgnoreSweep=false);

	bool bAllowSurfaceTracing = true;
	bool bShouldResetNavPlane = false;
	bool bGrounded = false;
	bool bAboutToFall = false;
	UPROPERTY(EditDefaultsOnly)
	float maxCoyoteTime = .3f;
	float currentCoyoteTime = .0f;

	float jumpTimer = 0.f;
	UPROPERTY(EditDefaultsOnly)
	float maxJumpTimer = 0.4f;
	float sphereRadius = 0.f;

	bool bJumpInputHeld = false;
	float jumpButtonHoldTimer = 0.f;
	UPROPERTY(EditDefaultsOnly)
	float maxJumpButtonHoldTimer = 0.15f;

	float dashTimer = 0.f;
	UPROPERTY(EditDefaultsOnly)
	float maxDashTimer = 0.2f;
	float dashCoolDown = 0.0f;
	UPROPERTY(EditDefaultsOnly)
	float maxDashCoolDown = 0.8f;
	UPROPERTY(EditDefaultsOnly)
	float dashMagnitude = 6000.f;
	bool bDashInputHeld = false;
	bool bAllowDashing = true;

	
	bool bUpdateVertVelocity = false;
	bool bUpdateHoriVelocity = false;
	float verticalDirection = 0.f;
	float horizontalDirection = 0.f;
	FVector currentForward;
	FVector currentRight;
	UPROPERTY(BlueprintReadOnly)
	FNavigationPlane navigationPlane;

	AActor* owner;
	USceneComponent* attachedComp;
	UPROPERTY(EditDefaultsOnly)
	float velAccelerationSpeed = 2.f;

	UPROPERTY(EditDefaultsOnly)
	float velDecelerationSpeed = 3.f;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float clampSizeMainForce = 1.f;

	UPROPERTY(EditDefaultsOnly)
	float maxSpeed = 20.f;

	float verticalVelocity=0.f;
	float horizontalVelocity=0.f;

	float gravity = -980.f;
	float accumulatedGravAccel = 0.7f;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.1", ClampMax = "1.0", UIMin = "0.1", UIMax = "1.0"))
	float defaultGravityAccel = 0.7f;

	UPROPERTY(EditDefaultsOnly)
	float gravityAccelMultiplier = 3.5f;

	float trailSpawnInterval = 0.1f;
	float trailSpawnTimer = 0.f;
	float attackForceDeceleration = 4.f;

	TArray<FPushForce> pushForces;

	class USceneComponent* camera;

	FCollisionQueryParams collisionParams;
	FCollisionShape myColSphere;

	FName unwalkableTag = "Unwalkable";

	FVector mainForce;
	FVector oldForce;
	FVector jumpForce;
	FVector gravForce;
	FVector dashForce;
	FVector attackForce;
	FVector jumpSurfaceAddative;
};
