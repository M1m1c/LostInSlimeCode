

#include "DeformNodeComp.h"


UDeformNodeComp::UDeformNodeComp()
{	
	PrimaryComponentTick.bCanEverTick = false;
}

void UDeformNodeComp::Initalize(float springStiffnes , float springDampening)
{
	stiffnes = springStiffnes;
	dampening = springDampening;
}

void UDeformNodeComp::ApplyForce(FVector force)
{
	velocity = (velocity + (force / mass)).GetClampedToSize(0.f,10000.f);
}

void UDeformNodeComp::UpdateNodePos(float DeltaTime)
{
	if (!owner) { return; }
	auto targetLoc = owner->GetActorLocation() + owner->GetActorRotation().RotateVector(localStartPos);
	auto currentLoc = GetComponentLocation();
	double dist = FVector::Distance(currentLoc, targetLoc);

	if (dist > 500.f) 
	{
		velocity *= 0.01f;
		SetWorldLocation(targetLoc); 
		return;
	}
	

	FHitResult hit;
	auto step = velocity * DeltaTime * lerpSpeed;
	if (step.ContainsNaN()) { return; }
	AddWorldOffset(step,true,&hit);


	if (hit.bBlockingHit && hit.GetActor() != nullptr)
	{
		//TODO give them a refenrece to a spring instead that they can call to get new force with a direction

		auto dir = (currentLoc - GetOwner()->GetActorLocation()).GetSafeNormal();
		auto extension = (dist / 50.f) - springRestLength;
		auto force = -stiffnes * FMath::Pow(extension, 2.f) * dir;
		ApplyForce(force);
		step = velocity * DeltaTime * lerpSpeed;
		step = FVector::VectorPlaneProject(step, hit.Normal);
		
		AddWorldOffset(step);	
	}

	velocity = (velocity * dampening).GetClampedToSize(0.f, 10000.f);
}

void UDeformNodeComp::BeginPlay()
{
	Super::BeginPlay();

	localStartPos = GetRelativeLocation();

	owner = GetOwner();

}

