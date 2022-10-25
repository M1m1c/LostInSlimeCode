
#include "SlimeTrailDecal.h"
#include "Components/DecalComponent.h"


ASlimeTrailDecal::ASlimeTrailDecal()
{

	PrimaryActorTick.bCanEverTick = true;

	root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = root;

	decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	decal->SetupAttachment(root);

}

//lightcolor = (0.303795,0.741667,0.282805)
//darkcolor = (0.067591,0.216667,0.188904)
void ASlimeTrailDecal::Initalize( FVector size, float lifeTime, FVector lightColor, FVector darkColor)
{
	SetActorScale3D(size);
	currentLifeTime = lifeTime;
	startingLifeTime = lifeTime;
	startingSize = size;
	targetSize = size * 0.1f;

	auto* matInterface= decal->GetDecalMaterial();
	materialInstance = UMaterialInstanceDynamic::Create(matInterface, this);
	materialInstance->SetVectorParameterValue(FName(TEXT("LightColor")), lightColor);
	materialInstance->SetVectorParameterValue(FName(TEXT("DarkColor")), darkColor);
	decal->SetDecalMaterial(materialInstance);
	bInitalized = true;
}


void ASlimeTrailDecal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bInitalized) { return; }
	

	float mappedLifeTime = GetMappedLifetime();

	auto newSize = FVector(
		FMath::Lerp(startingSize.X, targetSize.X, mappedLifeTime),
		FMath::Lerp(startingSize.Y, targetSize.Y, mappedLifeTime),
		FMath::Lerp(startingSize.Z, targetSize.Z, mappedLifeTime));

	SetActorScale3D(newSize);

	currentLifeTime = FMath::Max(currentLifeTime - DeltaTime, 0.f);
	if (FMath::IsNearlyZero(currentLifeTime)) { this->Destroy(); }
}

float ASlimeTrailDecal::GetMappedLifetime()
{
	return FMath::GetMappedRangeValueClamped(
		FVector2D(startingLifeTime, 0.f),
		FVector2D(0.0f, 1.f),
		currentLifeTime);
}

