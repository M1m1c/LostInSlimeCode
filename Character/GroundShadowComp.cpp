

#include "GroundShadowComp.h"


UGroundShadowComp::UGroundShadowComp()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGroundShadowComp::Initalize(TArray<UPrimitiveComponent*> ignoreComps)
{
	collisionParams.AddIgnoredComponents(ignoreComps);
}


void UGroundShadowComp::UpdateShadow(float DeltaTime)
{
	auto traceOffset = FVector(0.f, 0.f, -100.0f);
	auto ownerLoc = owner->GetActorLocation() + traceOffset;
	SetWorldLocation(ownerLoc + localStartPos);

	FHitResult hit;
	auto end = ownerLoc + (FVector(0.0f, 0.0f, -1.0f) * traceLength);
	GetWorld()->SweepSingleByChannel(hit, ownerLoc, end, FQuat::Identity, ECC_WorldStatic, myColSphere, collisionParams);

	auto lerpAlpha = FMath::Clamp((hit.Distance / (startSize.X + (-traceOffset.Z))), 0.0f, 1.0f);

	auto newShadowWidth = FMath::Lerp(
		minGroundShadowWidth,
		maxGroundShadowWidth,
		lerpAlpha);
	DecalSize = FVector(startSize.X, newShadowWidth, newShadowWidth);
}

void UGroundShadowComp::SetShadowVisible(bool b)
{
	SetVisibility(b);
}

void UGroundShadowComp::BeginPlay()
{
	Super::BeginPlay();

	localStartPos = GetRelativeLocation();

	owner = GetOwner();
	myColSphere = FCollisionShape::MakeSphere(10.f);

	startSize = DecalSize;
	minGroundShadowWidth = DecalSize.Y;
	maxGroundShadowWidth = minGroundShadowWidth + 100.f;

	if (!ensure(owner)) { DestroyComponent(); }
}

