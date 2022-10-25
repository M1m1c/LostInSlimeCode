
#include "CameraDriverComp.h"
#include <Runtime/Engine/Classes/GameFramework/SpringArmComponent.h>
#include <Runtime/Engine/Classes/Camera/CameraComponent.h>

UCameraDriverComp::UCameraDriverComp()
{

	PrimaryComponentTick.bCanEverTick = false;
}

void UCameraDriverComp::Initalize(USceneComponent* holder, USpringArmComponent* arm)
{

	if (!ensure(holder)) { return; }
	if (!ensure(arm)) { return; }

	cameraHolder = holder;
	cameraArm = arm;
	cameraHolderDefaultTransform = cameraHolder->GetComponentTransform();
	cameraArmDefaultTransform = cameraArm->GetRelativeTransform();

	bInitalized = true;
}


void UCameraDriverComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCameraDriverComp::UpdateCameraTurnCorner(float DeltaTime)
{
	if (!bShouldTurnCorner) { return; }
	bShouldResetCameraCorner = bNoHorizontalCamInput && bNoVerticalCamInput;

	if (bShouldResetCameraCorner)
	{

		CameraResetCornerStartTimer = FMath::Clamp(CameraResetCornerStartTimer - DeltaTime, 0.f, CameraResetStartTimerMax);
	}

	if (FMath::IsNearlyZero(CameraResetCornerStartTimer))
	{
		auto dotVerticalEdge = FVector::DotProduct(navUp, FVector::UpVector);
		auto dotVerticalDownEdge = FVector::DotProduct(navUp, -FVector::UpVector);
		auto dotGoingOverLedge = FVector::DotProduct(navUp, cameraHolder->GetForwardVector());
		if (dotVerticalEdge > 0.3f || dotGoingOverLedge > 0.3f || dotVerticalDownEdge > 0.5f)
		{
			CameraResetCornerStartTimer = CameraResetStartTimerMax;
			CameraResetTimeCorner = 0.f;
			bShouldTurnCorner = false;
			return;
		}
		auto forwardYaw = (-FVector(navUp.X, navUp.Y, 0.f)).ToOrientationQuat();

		auto currentHolderRot = cameraHolder->GetComponentRotation();
		cameraHolder->SetWorldRotation(
			FQuat::Slerp(FQuat(currentHolderRot),
				forwardYaw,
				CameraResetTimeCorner));

		CameraResetTimeCorner = FMath::Clamp(CameraResetTimeCorner + (DeltaTime * CameraResetSpeedCorner), 0.f, 1.f);
		if (FMath::IsNearlyEqual(CameraResetTimeCorner, 1.f))
		{
			bShouldTurnCorner = false;
		}
	}



}

void UCameraDriverComp::UpdateCameraTurnLedge(float DeltaTime)
{
	if (!bShouldTurnLedge) { return; }
	bShouldResetCameraLedge = bNoHorizontalCamInput && bNoVerticalCamInput;

	if (bShouldResetCameraLedge)
	{
		CameraResetLedgeStartTimer = FMath::Clamp(CameraResetLedgeStartTimer - DeltaTime, 0.f, CameraResetStartTimerMax);
	}

	if (FMath::IsNearlyZero(CameraResetLedgeStartTimer))
	{

		auto dotVerticalDownEdge = FVector::DotProduct(navUp, -FVector::UpVector);
		auto dotOldUp = FVector::DotProduct(oldNavUp, FVector::UpVector);
		auto bOutsideOfSpan = (dotVerticalDownEdge < 0.8f && dotVerticalDownEdge > -0.8f);
		
		if (!bShouldResetCameraLedge || bOutsideOfSpan || dotOldUp > 0.8f)
		{
			CameraResetLedgeStartTimer = CameraResetStartTimerMax;
			CameraResetTimeLedge = 0.f;
			oldNavUp = navUp;
			bShouldTurnLedge = false;
			return;
		}

		auto targetRot = navUp.ToOrientationRotator();
		targetRot = FRotator(
			dotVerticalDownEdge > 0.8f ? -(targetRot.Pitch + 45.f) : -(targetRot.Pitch - 45.f),
			0.f,
			0.f);

		auto currentArmRot = cameraArm->GetComponentRotation();
		cameraArm->SetRelativeRotation(FRotator(
			FMath::Lerp(currentArmRot.Pitch, targetRot.Pitch, CameraResetTimeLedge),
			0.f,
			0.f));

		CameraResetTimeLedge = FMath::Clamp(CameraResetTimeLedge + (DeltaTime * CameraResetSpeedLedge), 0.f, 1.f);
		if (FMath::IsNearlyEqual(CameraResetTimeLedge, 1.f))
		{
			oldNavUp = navUp;
			bShouldTurnLedge = false;
		}
	}



}

void UCameraDriverComp::UpdateCameraRotation(float DeltaTime)
{
	float VerticalInput = CameraVertical;
	/*if (cameraArm->GetComponentRotation().Pitch < -75)
	{
		VerticalInput = FMath::Clamp(VerticalInput, 0.0f, 1.0f);
	}
	if (cameraArm->GetComponentRotation().Pitch > 0)
	{
		VerticalInput = FMath::Clamp(VerticalInput, -1.0f, 0.0f);
	}*/

	FRotator newArmRotation = FRotator(VerticalInput * CameraRotationSpeed, 0.f, 0.f) * DeltaTime;
	FQuat quatArmRotation = FQuat(newArmRotation);
	cameraArm->AddRelativeRotation(quatArmRotation);

	FRotator newHolderRotation = FRotator(0.f, CameraHorizontal * CameraRotationSpeed, 0.f) * DeltaTime;
	FQuat quatHolderRotation = FQuat(newHolderRotation);
	cameraHolder->AddRelativeRotation(quatHolderRotation);

}

void UCameraDriverComp::UpdateCameraZoom(float DeltaTime)
{
	auto velocityChange = GetPropVelocityChangeConstantDec(
		DeltaTime,
		zoomVelocity,
		zoomAccelerationSpeed,
		zoomDecelerationSpeed,
		bZoomAccOrDec);

	zoomVelocity = FMath::Clamp(zoomVelocity + velocityChange, 0.f, 1.f);
	auto zoomChange = (zoomSpeed * zoomVelocity) * DeltaTime;
	auto newArmLength = FMath::Clamp(cameraArm->TargetArmLength - zoomDirection * zoomChange, minCameraZoom, maxCameraZoom);
	cameraArm->TargetArmLength = newArmLength;
}

float UCameraDriverComp::GetPropVelocityChangeConstantDec(float deltaTime, float currentVelocity, float accelSpeed, float decelSpeed, bool changeCondition)
{
	auto proportionalDec = -(deltaTime + (deltaTime * (decelSpeed * currentVelocity)));
	auto deceleration = currentVelocity > .0f ? proportionalDec : -deltaTime;
	auto velocityChange = (changeCondition ? deltaTime * accelSpeed : deceleration);
	return velocityChange;
}

void UCameraDriverComp::UpdateCamera(const float deltaTime)
{
	if (!bInitalized) { return; }

	UpdateCameraRotation(deltaTime);
	UpdateCameraZoom(deltaTime);

	UpdateCameraTurnCorner(deltaTime);
	UpdateCameraTurnLedge(deltaTime);
}

void UCameraDriverComp::ReadCameraHorizontal(float value)
{
	bNoHorizontalCamInput = FMath::IsNearlyZero(value);
	CameraResetCornerStartTimer = bNoHorizontalCamInput ? CameraResetCornerStartTimer : CameraResetStartTimerMax;
	CameraResetTimeCorner = bNoHorizontalCamInput ? CameraResetTimeCorner : 0.f;
	//bFreeCameraLook = bNoHorizontalCamInput ? bFreeCameraLook : true;
	bShouldTurnCorner = bNoHorizontalCamInput ? bShouldTurnCorner : false;

	CameraHorizontal = value;
}

void UCameraDriverComp::ReadCameraVertical(float value)
{
	bNoVerticalCamInput = FMath::IsNearlyZero(value);
	CameraResetCornerStartTimer = bNoVerticalCamInput ? CameraResetCornerStartTimer : CameraResetStartTimerMax;
	CameraResetTimeCorner = bNoVerticalCamInput ? CameraResetTimeCorner : 0.f;
	//bFreeCameraLook = bNoVerticalCamInput ? bFreeCameraLook : true;
	bShouldTurnCorner = bNoVerticalCamInput ? bShouldTurnCorner : false;

	CameraVertical = value;
}

void UCameraDriverComp::ReadCameraZoom(float value)
{
	if (value > 0.f || value < 0.f)
	{
		bZoomAccOrDec = true;
		zoomDirection = value;
	}
	else if (FMath::IsNearlyZero(value))
	{
		bZoomAccOrDec = false;
	}

}

void UCameraDriverComp::ToggleCameraFreeLook()
{
	bFreeCameraLook = !bFreeCameraLook;
}

bool UCameraDriverComp::GetbFreeCameraLook()
{
	return bFreeCameraLook;
}

void UCameraDriverComp::SetNavPlaneVariables(FVector up)
{
	navUp = up;
	CameraResetTimeCorner = 0.f;
	CameraResetTimeLedge = 0.f;
	bShouldTurnCorner = true;
	bShouldTurnLedge = true;
}

