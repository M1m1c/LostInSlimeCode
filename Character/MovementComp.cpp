

#include "MovementComp.h"
#include "Components/SphereComponent.h"



UMovementComp::UMovementComp()
{

	PrimaryComponentTick.bCanEverTick = true;
}

void UMovementComp::Initalize(USceneComponent* cameraPivot, TArray<UPrimitiveComponent*> ignoreComps)
{
	owner = GetOwner();
	if (!ensure(owner)) { return; }
	camera = cameraPivot;
	if (!ensure(camera)) { return; }

	collisionParams.AddIgnoredComponents(ignoreComps);

	auto root = owner->GetRootComponent();
	auto sphere = Cast<USphereComponent>(root);
	if (sphere)
	{
		sphereRadius = sphere->GetScaledSphereRadius();
	}

	navigationPlane = FNavigationPlane(
		owner->GetActorForwardVector(),
		owner->GetActorRightVector(),
		owner->GetActorUpVector());

	myColSphere = FCollisionShape::MakeSphere(10.f);
}

void UMovementComp::UpdateMovement(float deltaTime)
{
	if (!active) { return; }
	if (!FMath::IsNearlyZero(mainForce.Size())) { oldForce = mainForce.GetSafeNormal(); }



	DoInputMovement(deltaTime);
	DoPushForces(deltaTime);
	DoDashing(deltaTime);
	ExtendJumpWithHeldButton(deltaTime);
	DoJumpingAndFalling(deltaTime);
	ApplyMovementForces(deltaTime);
	DecelerateAttackForce();
	DoSurfaceChangeChecking();

	if (bAboutToFall && bGrounded) 
	{
		currentCoyoteTime = FMath::Clamp(currentCoyoteTime + deltaTime, 0.f, maxCoyoteTime);
		if (FMath::IsNearlyEqual(currentCoyoteTime, maxCoyoteTime)) {
			bAboutToFall = false;
			SetGrounded(false);
		}
	}

	if (bGrounded && mainForce.Size() > 0.1f)
	{
		trailSpawnTimer = FMath::Clamp(trailSpawnTimer + deltaTime, 0.f, trailSpawnInterval);
		if (FMath::IsNearlyEqual(trailSpawnTimer, trailSpawnInterval))
		{
			trailSpawnTimer = 0.f;
			OnMove.Broadcast();
			OnSpawnTrail.Broadcast(
				owner->GetActorLocation() - (navigationPlane.forwardVector * 40.f),
				navigationPlane.upVector.ToOrientationRotator(),
				FVector(0.5f, 0.5f, 0.5f),
				attachedComp,
				4.f);
		}
	}

	owner->SetActorRotation(oldForce.GetSafeNormal().ToOrientationQuat());

}

void UMovementComp::DecelerateAttackForce()
{
	attackForce = FMath::Lerp(attackForce, FVector::ZeroVector, attackForceDeceleration * GetWorld()->DeltaTimeSeconds);
}

//TODO we might need to apply these in the main force movement func and added them to final force
void UMovementComp::DoPushForces(float deltaTime)
{
	if (pushForces.Num())
	{
		for (int i = pushForces.Num() - 1; i >= 0; i--)
		{
			auto force = &pushForces[i];
			AttemptMove(force->direction * force->magnitude * deltaTime);
			force->duration = FMath::Max(force->duration - deltaTime, 0.f);

			if (FMath::IsNearlyZero(force->duration))
			{
				//UE_LOG(LogTemp, Warning, TEXT("force removed"));
				pushForces.RemoveAt(i);
				continue;
			}
		}

	}
}

void UMovementComp::DoDashing(float deltaTime)
{
	if (IsDashing())
	{
		accumulatedGravAccel = defaultGravityAccel;
		gravForce = FVector::ZeroVector;
		/*auto cameraDir = (bGrounded ?
			FVector::VectorPlaneProject(camera->GetForwardVector(), navigationPlane.upVector).GetSafeNormal()
			: camera->GetForwardVector());*/

		auto moveDir =
			(bUpdateVertVelocity ? verticalDirection * camera->GetForwardVector() : camera->GetForwardVector()) +
			(bUpdateHoriVelocity ? horizontalDirection * camera->GetRightVector() : FVector::ZeroVector);
		moveDir = FVector::VectorPlaneProject(moveDir, bGrounded ? navigationPlane.upVector : FVector::UpVector).GetSafeNormal();

		auto dashDir = FVector(
			moveDir.X,
			moveDir.Y,
			bGrounded ? moveDir.Z : FMath::Max(camera->GetForwardVector().Z, 0.f));

		dashForce = (dashDir + DashDirection).GetSafeNormal() * dashMagnitude * deltaTime;
		dashTimer = FMath::Clamp(dashTimer - deltaTime, 0.f, maxDashTimer);
	}
	else
	{
		if (dashForce != FVector::ZeroVector) { dashForce = FVector::ZeroVector; }
		dashCoolDown = FMath::Clamp(dashCoolDown - deltaTime, 0.f, maxDashCoolDown);
	}
}

void UMovementComp::ApplyPushForce(FVector direction, float magnitude, float duration)
{
	pushForces.Add(FPushForce(direction, magnitude, duration));
}

bool UMovementComp::IsGrounded()
{
	return bGrounded;
}

void UMovementComp::DoSurfaceChangeChecking()
{
	if (!bAllowSurfaceTracing) { return; }

	FHitResult forwardHit;
	auto notHitingForwardWall = TraceForNewNavPlane(mainForce, forwardHit);

	FHitResult planeHit;
	auto notHittignNavPlaneDown = TraceForNewNavPlane(IsJumping() ? navigationPlane.upVector : -navigationPlane.upVector, planeHit);

	FHitResult groundHit;
	auto notHitingNormalGround = TraceForNewNavPlane(-FVector::UpVector, groundHit);

	FHitResult cornerHit;
	bool notHittingCorner = true;
	if (!IsJumping())
	{
		auto start = owner->GetActorLocation() + (-navigationPlane.upVector.GetSafeNormal() * (sphereRadius * 0.5f)) + (navigationPlane.forwardVector.GetSafeNormal() * (sphereRadius));
		auto dir = (owner->GetActorLocation() + (-navigationPlane.upVector * (sphereRadius + 10.f))) - start;
		notHittingCorner = TraceForNewNavPlane(start, dir, cornerHit);
	}


	if (notHitingForwardWall && notHitingNormalGround && notHittignNavPlaneDown && notHittingCorner)
	{
		if (!IsJumping() && !bAboutToFall && bGrounded)
		{
			bAboutToFall = true;
			currentCoyoteTime = 0.f;
			owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			UpdateNavigationPlane(FVector::UpVector);
		}
		else if(IsJumping())
		{
			SetGrounded(false);
			owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		}
	}
	else
	{
		FVector resultNormal = FVector::UpVector;
		USceneComponent* resultComp = nullptr;

		if (!notHitingForwardWall)
		{
			resultNormal = forwardHit.Normal;
			resultComp = forwardHit.GetComponent();
			OnChangedNavUp.Broadcast(forwardHit.Normal.GetSafeNormal());
		}
		else if (!notHitingNormalGround && IsNavPlaneSemiHorizontal())
		{
			resultNormal = groundHit.Normal;
			resultComp = groundHit.GetComponent();
		}
		else if (!notHittignNavPlaneDown)
		{
			resultNormal = planeHit.Normal;
			resultComp = planeHit.GetComponent();
		}
		else if (!notHittingCorner && !IsJumping() && cornerHit.GetActor() != nullptr)
		{
			resultNormal = cornerHit.Normal;
			resultComp = cornerHit.GetComponent();
			owner->SetActorLocation(cornerHit.Location + (cornerHit.Normal.GetSafeNormal() * sphereRadius));
			OnChangedNavUp.Broadcast(cornerHit.Normal.GetSafeNormal());
		}

		BecomeGrounded(resultNormal, resultComp);
	}
}

void UMovementComp::DoJumpingAndFalling(float deltaTime)
{
	if (IsJumping())
	{
		auto jumpMagnitude = -gravity * deltaTime * FMath::Pow(jumpTimer * 4.f, 2.f);

		jumpSurfaceAddative = IsNavPlaneSemiUpsideDown() ? FVector::ZeroVector : FVector::UpVector;

		auto inputVertAdditive = bUpdateVertVelocity && !IsNavPlaneSemiHorizontal() ?
			verticalDirection * camera->GetForwardVector() : FVector::ZeroVector;

		auto inputHoriAdditive = bUpdateHoriVelocity && !IsNavPlaneSemiHorizontal() ?
			horizontalDirection * camera->GetRightVector() : FVector::ZeroVector;

		auto additives = jumpSurfaceAddative + (inputVertAdditive + inputHoriAdditive).GetSafeNormal();

		jumpForce = (navigationPlane.upVector.GetSafeNormal() + additives).GetSafeNormal() * jumpMagnitude;
		jumpTimer = FMath::Clamp(jumpTimer - deltaTime, 0.f, maxJumpTimer);
	}
	else if (!bGrounded)
	{
		if (bShouldResetNavPlane)
		{
			bShouldResetNavPlane = false;
			navigationPlane = FNavigationPlane(FVector::ForwardVector, FVector::RightVector, FVector::UpVector);
		}
		gravForce = FVector(0.f, 0.f, gravity * accumulatedGravAccel * deltaTime);
		accumulatedGravAccel += deltaTime * gravityAccelMultiplier;
	}
	else
	{
		if (jumpForce != FVector::ZeroVector) { jumpForce = FVector::ZeroVector; }
		if (gravForce != FVector::ZeroVector) { gravForce = FVector::ZeroVector; }
		jumpSurfaceAddative = FVector::ZeroVector;
	}
}

void UMovementComp::ExtendJumpWithHeldButton(float deltaTime)
{
	if (!bJumpInputHeld) { return; }
	if (FMath::IsNearlyEqual(jumpButtonHoldTimer, maxJumpButtonHoldTimer)) { return; }

	jumpButtonHoldTimer = FMath::Clamp(jumpButtonHoldTimer + (deltaTime * 0.7f), 0.f, maxJumpButtonHoldTimer);
	jumpTimer += (deltaTime + jumpButtonHoldTimer) * 0.2f;
}

void UMovementComp::DoInputMovement(float deltaTime)
{
	auto bNoInput = !bUpdateVertVelocity && !bUpdateHoriVelocity;
	if (!bUpdateVertVelocity && !bUpdateHoriVelocity)
	{
		auto decelSpeed = bGrounded ? velDecelerationSpeed : velDecelerationSpeed * 0.25f;
		auto reductionForce = (mainForce.GetSafeNormal() * -1.f) * decelSpeed * deltaTime;
		auto resultForce = (reductionForce + mainForce);

		if (FVector::DotProduct(resultForce.GetSafeNormal(), mainForce.GetSafeNormal()) < 0.f)
		{
			mainForce = FVector::ZeroVector;
		}
		else
		{
			mainForce += reductionForce;
		}
	}
	else
	{
		auto newVertStep = bUpdateVertVelocity ? deltaTime * verticalDirection * velAccelerationSpeed : 0.f;
		auto newHoriStep = bUpdateHoriVelocity ? deltaTime * horizontalDirection * velAccelerationSpeed : 0.f;

		auto force = GetForceAlongPlane(newVertStep, newHoriStep, navigationPlane.upVector);
		auto dirInput = FMath::Clamp(FMath::Abs(verticalDirection) + FMath::Abs(horizontalDirection), 0.f, 1.f);
		mainForce += (force + mainForce).Size() < dirInput ? force : FVector::ZeroVector;
		if (force.Size() > 0.f)
		{
			mainForce = (force.GetSafeNormal() * mainForce.Size());
		}
		mainForce = mainForce.GetClampedToMaxSize(clampSizeMainForce);
	}
}

void UMovementComp::ApplyMovementForces(float deltaTime)
{
	float RemainingTime = deltaTime;
	int Iterations = 0;

	while (RemainingTime > 0.f && ++Iterations < 10)
	{
		auto clampedForce = FMath::Clamp(mainForce.Size(), 0.f, 1.f);
		auto moveForce = mainForce.GetSafeNormal() * (clampedForce * maxSpeed * (1 / clampSizeMainForce));;

		auto finalForce = moveForce + gravForce + dashForce + jumpForce + attackForce;
		auto hit = AttemptMove(finalForce);

		auto ownerLocation = owner->GetActorLocation();

		if (hit.bBlockingHit)
		{
			if (IgnorePhysicsCollisions(hit, finalForce, Iterations))
			{
				continue;
			}
			else if (hit.bStartPenetrating)
			{
				owner->AddActorWorldOffset(hit.Normal * (hit.PenetrationDepth + 0.1f));
			}
			else
			{
				mainForce = FVector::VectorPlaneProject(mainForce, hit.Normal);

				auto comp = hit.GetComponent();
				if (comp && (comp->ComponentHasTag(unwalkableTag) || hit.GetActor()->ActorHasTag(unwalkableTag)))
				{
					AttemptMove(FVector::VectorPlaneProject(finalForce * 0.2f, hit.Normal));
					continue;
				}

				BecomeGrounded(hit.Normal, hit.GetComponent());
				OnChangedNavUp.Broadcast(hit.Normal.GetSafeNormal());
				RemainingTime -= RemainingTime * hit.Time;
			}
		}
		else
		{
			break;
		}
	}
}

bool UMovementComp::IgnorePhysicsCollisions(FHitResult& hit, FVector& velocity, int& Iterations)
{
	auto retflag = false;
	auto comp = hit.GetComponent();
	if (comp && comp->GetCollisionObjectType() == ECollisionChannel::ECC_PhysicsBody)
	{
		if (!comp->IsSimulatingPhysics()) { comp->SetSimulatePhysics(true); }
		comp->AddImpulseAtLocation(mainForce.GetSafeNormal() * maxSpeed * 100.f, hit.Location);
		AttemptMove(velocity, true);
		Iterations = 10;
		retflag = true;
	}
	return retflag;
}

void UMovementComp::BecomeGrounded(FVector normal, USceneComponent* compToAttachTo)
{
	if (!bGrounded)
	{
		dashTimer = 0.f;
		OnLand.Broadcast();
	}

	accumulatedGravAccel = defaultGravityAccel;
	jumpTimer = 0.0f;
	SetGrounded(true);
	bAllowDashing = true;
	UpdateNavigationPlane(normal);
	if (compToAttachTo)
	{
		attachedComp = compToAttachTo;
		owner->AttachToComponent(attachedComp, FAttachmentTransformRules::KeepWorldTransform);

	}
	else if (!compToAttachTo && attachedComp)
	{
		owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void UMovementComp::SetGrounded(bool b)
{
	bGrounded = b;
	OnGroundedChanged.Broadcast(!b);
}

void UMovementComp::UpdateNavigationPlane(FVector normal)
{
	navigationPlane.upVector = normal.GetSafeNormal();
	navigationPlane.forwardVector = FVector::VectorPlaneProject(oldForce, normal).GetSafeNormal();
	navigationPlane.rightVector = FVector::CrossProduct(navigationPlane.upVector, navigationPlane.forwardVector);
	mainForce = GetForceAlongPlane(mainForce, navigationPlane.upVector);
}

bool UMovementComp::CheckTraceHits(TArray<FHitResult>& hits, FHitResult& outHit)
{
	bool retval = true;
	if (hits.Num())
	{
		int count = 0;
		for (auto hit : hits)
		{
			auto comp = hit.GetComponent();
			if (!comp) { continue; }
			auto bIgnoreChannels = comp->GetCollisionObjectType() != ECollisionChannel::ECC_Vehicle;
			bIgnoreChannels = bIgnoreChannels && comp->GetCollisionObjectType() != ECollisionChannel::ECC_PhysicsBody;
			auto bUnwalkable = comp->ComponentHasTag(unwalkableTag);
			if (hit.GetActor() && hit.GetActor() != owner && bIgnoreChannels && !bUnwalkable) {
				count++;
				outHit = hit;
				break;
			}
		}
		if (count == 0) { retval = false; }
	}
	return retval;
}

bool UMovementComp::TraceForNewNavPlane(FVector direction, FHitResult& outHit)
{
	auto start = owner->GetActorLocation();
	auto end = start + (direction.GetSafeNormal() * (sphereRadius + 5.f));
	TArray<FHitResult> hits;
	LineTrace(hits, start, end);
	return !CheckTraceHits(hits, outHit);

}
bool UMovementComp::TraceForNewNavPlane(FVector start, FVector direction, FHitResult& outHit)
{
	auto end = start + (direction.GetSafeNormal() * (sphereRadius * 2.f));
	TArray<FHitResult> hits;
	LineTrace(hits, start, end);
	return !CheckTraceHits(hits, outHit);

}

void UMovementComp::LineTrace(TArray<FHitResult>& OutHits, FVector& start, FVector3d& end)
{
	GetWorld()->SweepMultiByChannel(OutHits, start, end, FQuat::Identity, ECC_WorldStatic, myColSphere, collisionParams);
	DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 2.f, 0, 1.f);

	for (auto item : OutHits)
	{
		if (item.GetActor() == owner) { continue; }
		DrawDebugSphere(GetWorld(), item.Location, 10.f, 4, FColor::Purple, false, 2.f);
	}
}

FHitResult UMovementComp::AttemptMove(FVector forceVector, bool bIgnoreSweep)
{
	FHitResult hit;

	if (!bIgnoreSweep)
	{
		owner->AddActorWorldOffset(
			forceVector,
			true,
			&hit);
	}
	else
	{
		owner->AddActorWorldOffset(forceVector);
	}


	return hit;
}

void UMovementComp::ReadVertical(float value)
{
	if (FMath::IsNearlyZero(value))
	{
		bUpdateVertVelocity = false;
		return;
	}
	verticalDirection = FMath::Clamp(value, -1.f, 1.f);

	auto verticalDot = FVector::DotProduct(camera->GetUpVector(), navigationPlane.upVector);
	auto horizontalDot = FVector::DotProduct(camera->GetRightVector(), navigationPlane.upVector);
	auto forwardDot = FVector::DotProduct(camera->GetForwardVector(), navigationPlane.upVector);

	auto vertSpan = (verticalDot > -0.75f && verticalDot < 0.75f);
	auto horiSpan = (horizontalDot > -0.7f && horizontalDot < 0.7f);
	auto forwardLedge = (forwardDot > 0.3f);

	currentForward = vertSpan && horiSpan ? camera->GetUpVector() : camera->GetForwardVector();
	currentForward = forwardLedge ? -camera->GetUpVector() : currentForward;

	bUpdateVertVelocity = true;
}

void UMovementComp::ReadHorizontal(float value)
{
	if (FMath::IsNearlyZero(value))
	{
		bUpdateHoriVelocity = false;
		return;
	}

	auto dot = FVector::DotProduct(camera->GetRightVector(), navigationPlane.upVector);
	auto maxSpan = (dot < -0.75f || dot > 0.75f);

	horizontalDirection = FMath::Clamp(value, -1.f, 1.f);
	currentRight = maxSpan ? FVector::ZeroVector : camera->GetRightVector();
	bUpdateHoriVelocity = true;
}

void UMovementComp::InputPressJump()
{
	if (bGrounded)
	{
		auto bForwardFacingNavUp = FVector::DotProduct(camera->GetForwardVector(), navigationPlane.upVector) > 0.f;
		if (bForwardFacingNavUp)
		{
			UpdateNavigationPlane(FVector::UpVector);
		}

		jumpTimer = maxJumpTimer;
		bShouldResetNavPlane = true;
		bJumpInputHeld = true;
		jumpButtonHoldTimer = 0.f;
		OnJump.Broadcast();
	}
}

void UMovementComp::InputReleaseJump()
{
	bJumpInputHeld = false;
}

void UMovementComp::InputPressDash()
{
	if (bAllowDashing && !bDashInputHeld && FMath::IsNearlyZero(dashCoolDown))
	{
		bAllowDashing = false;
		bDashInputHeld = true;
		dashCoolDown = maxDashCoolDown;
		dashTimer = maxDashTimer;
		OnDash.Broadcast();
	}
}

void UMovementComp::InputReleaseDash()
{
	bDashInputHeld = false;
}

void UMovementComp::TeleportToLocation(FVector location)
{
	bAllowSurfaceTracing = false;
	bGrounded = false;
	attachedComp = nullptr;
	owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	owner->SetActorLocation(location);
	bAllowSurfaceTracing = true;
}

bool UMovementComp::IsJumping()
{
	return !FMath::IsNearlyZero(jumpTimer);
}

bool UMovementComp::IsDashing()
{
	return !FMath::IsNearlyZero(dashTimer);
}

bool UMovementComp::IsNavPlaneDefault()
{
	auto b1 = navigationPlane.forwardVector == FVector::UpVector;
	auto b2 = navigationPlane.rightVector == FVector::RightVector;
	auto b3 = navigationPlane.upVector == FVector::UpVector;
	return  b1 && b2 && b3;
}

bool UMovementComp::IsNavPlaneSemiHorizontal()
{
	auto dot = FVector::DotProduct(navigationPlane.upVector, FVector::UpVector);
	//UE_LOG(LogTemp, Warning, TEXT("dot %f"), dot);
	return dot > 0.2f;
}

bool UMovementComp::IsNavPlaneSemiUpsideDown()
{
	auto dot = FVector::DotProduct(navigationPlane.upVector, FVector::UpVector);
	//UE_LOG(LogTemp, Warning, TEXT("dot %f"), dot);
	return dot < -0.5f;
}

void UMovementComp::SetMainForce(FVector newForce)
{
	mainForce = newForce;
}

void UMovementComp::DisableAllForces()
{
	mainForce = FVector::ZeroVector;
	jumpForce = FVector::ZeroVector;
	gravForce = FVector::ZeroVector;
	dashForce = FVector::ZeroVector;
}

FVector UMovementComp::GetAttackForce()
{
	return attackForce;
}

void UMovementComp::SetAttackForce(FVector NewForce)
{
	attackForce = NewForce;
}

FVector UMovementComp::GetForceAlongPlane(float vertStep, float horiStep, FVector upVector)
{
	auto forwardVelocity = FVector::VectorPlaneProject(currentForward, upVector).GetSafeNormal() * vertStep;
	auto rightVelocity = FVector::VectorPlaneProject(currentRight, upVector).GetSafeNormal() * horiStep;

	return forwardVelocity + rightVelocity;
}

FVector UMovementComp::GetForceAlongPlane(FVector force, FVector upVector)
{
	auto resultForce = FVector::VectorPlaneProject(force, upVector);

	return resultForce;
}
