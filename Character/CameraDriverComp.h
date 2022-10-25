// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraDriverComp.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GP4TEAM3_API UCameraDriverComp : public UActorComponent
{
	GENERATED_BODY()

public:
		UCameraDriverComp();
	
		void Initalize( class USceneComponent* holder, class USpringArmComponent* arm);
	
		bool bInitalized = false;
	
		void UpdateCamera(const float deltaTime);

		void ReadCameraHorizontal(float value);
	
		void ReadCameraVertical(float value);
	
		void ReadCameraZoom(float value);
	
		void ToggleCameraFreeLook();
	
		bool GetbFreeCameraLook();

		UFUNCTION()
		void SetNavPlaneVariables(FVector up);

		UPROPERTY(BlueprintReadWrite)
		float CameraRotationSpeed = 100.f;

	protected:
	
		void UpdateCameraRotation(float DeltaTime);
	
		void UpdateCameraZoom(float DeltaTime);

		float GetPropVelocityChangeConstantDec(float deltaTime, float currentVelocity, float accelSpeed, float decelSpeed, bool changeCondition);
	
		void UpdateCameraTurnCorner(float DeltaTime);

		void UpdateCameraTurnLedge(float DeltaTime);
	
		UPROPERTY(EditDefaultsOnly)
			float minCameraZoom = 100.f;
		UPROPERTY(EditDefaultsOnly)
			float maxCameraZoom = 2500.f;
	
		class USceneComponent* cameraHolder;
		class USpringArmComponent* cameraArm;
		//class ABoatPawn* owner;
	

		FTransform cameraHolderDefaultTransform;
		FTransform cameraArmDefaultTransform;
		
		float CameraHorizontal = 0.f;
		float CameraVertical = 0.f;
	
		UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ClampMax = "3.0", UIMin = "0.0", UIMax = "3.0"))
		float CameraResetStartTimerMax = 0.1f;
		UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float CameraResetSpeedCorner = 0.2f;
		UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float CameraResetSpeedLedge = 0.8f;
	
		float CameraResetCornerStartTimer = 0.f;
		float CameraResetLedgeStartTimer = 0.f;
		float CameraResetTimeCorner = 0.f;
		float CameraResetTimeLedge = 0.f;
		
	
		float zoomDirection = 0;
		float zoomVelocity = 0;
		UPROPERTY(EditDefaultsOnly)
		float zoomSpeed = 700.f;
		float zoomDecelerationSpeed = 1.f;
		float zoomAccelerationSpeed = 800.f;
		bool bZoomAccOrDec = false;
	
		bool bFreeCameraLook = false;
		bool bShouldResetCameraCorner = false;
		bool bShouldResetCameraLedge = false;
		bool bNoHorizontalCamInput = false;
		bool bNoVerticalCamInput = false;
	
		bool bShouldTurnCorner = false;
		bool bShouldTurnLedge = false;
		FVector navUp;
		FVector oldNavUp = FVector::ZeroVector;

	public:
		virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};