
#include "SlimePawn.h"

#include "CameraDriverComp.h"
#include "MovementComp.h"
#include "DeformNodeComp.h"
#include "DeformNodeHandeler.h"
#include "SlimeTrailDecal.h"
#include "KeyHolderComp.h"
#include "GP4Team3/Armour/SlimeHealthSystem.h"
#include "GroundShadowComp.h"

#include "Components/SphereComponent.h"
#include <Runtime/Engine/Classes/GameFramework/SpringArmComponent.h>
#include <Runtime/Engine/Classes/Camera/CameraComponent.h>

#include "Combat/Attacks/AttackComponent.h"


ASlimePawn::ASlimePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	root = CreateDefaultSubobject<USphereComponent>(TEXT("Root"));
	RootComponent = root;

	cameraHolder = CreateDefaultSubobject<USceneComponent>(TEXT("CameraHolder"));
	cameraHolder->SetupAttachment(root);

	cameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	cameraArm->SetupAttachment(cameraHolder);
	cameraArm->TargetArmLength = 1000.0f;

	cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	cameraComp->SetupAttachment(cameraArm, USpringArmComponent::SocketName);

	cameraDriver = CreateDefaultSubobject<UCameraDriverComp>(TEXT("CameraDriver"));
	movementComp = CreateDefaultSubobject<UMovementComp>(TEXT("MovementComp"));

	attackComp = CreateDefaultSubobject<UAttackComponent>(TEXT("AttackComponent"));
	deformHandeler = CreateDefaultSubobject<UDeformNodeHandeler>(TEXT("DeformNodeHandeler"));
	keyHolder = CreateDefaultSubobject<UKeyHolderComp>(TEXT("KeyHolder"));

	groundShadowComp = CreateDefaultSubobject<UGroundShadowComp>(TEXT("GroundShadowComp"));
	groundShadowComp->SetupAttachment(root);
}

void ASlimePawn::SetSlimeColor(FVector light, FVector dark)
{
	materialInstance->SetVectorParameterValue(FName(TEXT("LightColor")), light);
	materialInstance->SetVectorParameterValue(FName(TEXT("DarkColor")), dark);
	lightColor = light;
	darkColor = dark;
}

void ASlimePawn::ResetSlimeColor()
{
	SetSlimeColor(originalLightColor, originalDarkColor);
}

void ASlimePawn::UpdateSlimeSize(int newSize)
{
	auto size = newSize != 0 ? FMath::Abs((float)newSize) : 0.5f;
	size = (newSize / 2.f);
	size = size < 1.f ? size + 0.3f : size;
	mesh->SetRelativeScale3D(FVector(size, size, size));
}

void ASlimePawn::ActivateDeformSpring()
{
	deformHandeler->bSpringsActive = true;
}

void ASlimePawn::BeginPlay()
{
	Super::BeginPlay();

	worldPtr = GetWorld();
	if (!ensure(worldPtr)) { return; }

	attackComp->SlimePawn = this;

	movementComp->OnChangedNavUp.AddDynamic(cameraDriver, &UCameraDriverComp::SetNavPlaneVariables);

	if (!ensure(slimeTrailBP)) { return; }
	movementComp->OnSpawnTrail.AddDynamic(this, &ASlimePawn::SpawnSlimeTrailDecal);

	cameraDriver->Initalize(cameraHolder, cameraArm);

	auto arr = GetComponentsByClass(UDeformNodeComp::StaticClass());
	TArray<UPrimitiveComponent*> ignoreComps;
	for (auto item : arr)
	{
		auto casted = Cast<UPrimitiveComponent>(item);
		if (!casted) { continue; }
		ignoreComps.Add(casted);
	}

	movementComp->Initalize(cameraComp, ignoreComps);
	groundShadowComp->Initalize(ignoreComps);
	deformHandeler->Initalize();

	auto meshComp = GetComponentByClass(UStaticMeshComponent::StaticClass());
	mesh = Cast<UStaticMeshComponent>(meshComp);

	if (mesh)
	{
		auto* matInterface = mesh->GetMaterial(0);
		materialInstance = UMaterialInstanceDynamic::Create(matInterface, this);
		materialInstance->SetVectorParameterValue(FName(TEXT("LightColor")), lightColor);
		materialInstance->SetVectorParameterValue(FName(TEXT("DarkColor")), darkColor);
		mesh->SetMaterial(0, materialInstance);
	}

	movementComp->OnJump.AddDynamic(this, &ASlimePawn::OnJump);
	movementComp->OnDash.AddDynamic(this, &ASlimePawn::OnDash);
	movementComp->OnMove.AddDynamic(this, &ASlimePawn::OnMove);
	movementComp->OnLand.AddDynamic(this, &ASlimePawn::OnLand);
	movementComp->OnGroundedChanged.AddDynamic(groundShadowComp, &UGroundShadowComp::SetShadowVisible);

	auto healhComp = GetComponentByClass(USlimeHealthSystem::StaticClass());
	auto healthSystem = Cast<USlimeHealthSystem>(healhComp);
	if (healthSystem)
	{
		healthSystem->OnHealthChange.AddDynamic(this, &ASlimePawn::UpdateSlimeSize);
	}

	GetWorldTimerManager().SetTimer(FixedTickHandle, this, &ASlimePawn::FixedTick, fixedTickDeltaTime, true, 0.0f);
}

void ASlimePawn::FixedTick()
{
	OnFixedTick(fixedTickDeltaTime);

	movementComp->UpdateMovement(fixedTickDeltaTime);
	cameraDriver->UpdateCamera(fixedTickDeltaTime);
	deformHandeler->UpdateDeformSprings(fixedTickDeltaTime);

	if (!movementComp->IsGrounded())
	{
		groundShadowComp->UpdateShadow(fixedTickDeltaTime);
	}

}

void ASlimePawn::SpawnSlimeTrailDecal(FVector location, FRotator rotation, FVector size, USceneComponent* attachTarget, float lifeTime)
{
	auto temp = worldPtr->SpawnActor<ASlimeTrailDecal>(slimeTrailBP, location, rotation);
	temp->AttachToComponent(attachTarget, FAttachmentTransformRules::KeepWorldTransform);
	temp->Initalize(size, lifeTime, lightColor, darkColor);
}


void ASlimePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAxis("CameraHorizontal", cameraDriver, &UCameraDriverComp::ReadCameraHorizontal);
	InputComponent->BindAxis("CameraVertical", cameraDriver, &UCameraDriverComp::ReadCameraVertical);
	InputComponent->BindAxis("CameraZoom", cameraDriver, &UCameraDriverComp::ReadCameraZoom);
	InputComponent->BindAction("FreeCameraLook", IE_Pressed, cameraDriver, &UCameraDriverComp::ToggleCameraFreeLook);

	InputComponent->BindAxis("MoveVertical", movementComp, &UMovementComp::ReadVertical);
	InputComponent->BindAxis("MoveHorizontal", movementComp, &UMovementComp::ReadHorizontal);
	InputComponent->BindAction("JumpInput", IE_Pressed, movementComp, &UMovementComp::InputPressJump);
	InputComponent->BindAction("JumpInput", IE_Released, movementComp, &UMovementComp::InputReleaseJump);
	InputComponent->BindAction("DashInput", IE_Pressed, movementComp, &UMovementComp::InputPressDash);
	InputComponent->BindAction("DashInput", IE_Released, movementComp, &UMovementComp::InputReleaseDash);
	InputComponent->BindAction("DashAttack", IE_Pressed, attackComp, &UAttackComponent::QueueDashAttack);
	InputComponent->BindAction("GroundSlam", IE_Pressed, attackComp, &UAttackComponent::QueueGroundSlam);
}
