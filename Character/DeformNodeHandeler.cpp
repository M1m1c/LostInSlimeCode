

#include "DeformNodeHandeler.h"
#include "DeformNodeComp.h"


UDeformNodeHandeler::UDeformNodeHandeler()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDeformNodeHandeler::Initalize()
{
	auto arr = GetOwner()->GetComponentsByClass(UDeformNodeComp::StaticClass());
	for (auto item : arr)
	{
		auto casted = Cast<UDeformNodeComp>(item);
		if (!casted) { continue; }
		deformNodes.Add(casted);
		casted->Initalize();
	}

	auto owner = GetOwner();
	for (auto& spring : deformSprings)
	{
		spring.Initalize(owner,deformNodes);
	}

	

}

void UDeformNodeHandeler::UpdateDeformSprings(float DeltaTime)
{
	if(!bSpringsActive)
	{
		return;
	}
	for (auto& spring : deformSprings)
	{
		spring.UpdateSpring(DeltaTime);
	}

	for (auto node : deformNodes)
	{
		node->UpdateNodePos(DeltaTime);
	}
}


void FDeformSpring::Initalize(AActor* Owner,  TArray<UDeformNodeComp*> deformNodes)
{

	owner = Owner;
	if (NodeARef != ENodeLocation::None) 
	{
		auto a = *deformNodes.FindByPredicate([this](UDeformNodeComp* q) {return q->GetNodeLocation() == NodeARef; });
		if (a) { nodeA = a; }
	}

	if (NodeBRef != ENodeLocation::None)
	{
		auto b = *deformNodes.FindByPredicate([this](UDeformNodeComp* q) {return q->GetNodeLocation() == NodeBRef; });
		if (b) { nodeB = b; }
	}

	if (!nodeA) { return; }

	localStartPos = nodeA->GetLocalStartPos();
}

void FDeformSpring::UpdateSpring(float deltaTime)
{

	if (!nodeA) { return; }
	auto targetLoc = owner->GetActorLocation() + owner->GetActorRotation().RotateVector(localStartPos);

	if (nodeB) 
	{ 
		auto nodeBCurrentLoc = nodeB->GetComponentLocation();
		auto nodeBTargetLoc = owner->GetActorLocation() + owner->GetActorRotation().RotateVector(nodeB->GetLocalStartPos());
		auto nodeBDist = FVector::Distance(nodeBCurrentLoc,nodeBTargetLoc);

		targetLoc = (-nodeB->GetVelocity().GetSafeNormal() * nodeBDist) + targetLoc; 
	}

	auto currentLoc = nodeA->GetComponentLocation();
	double dist = FVector::Distance(currentLoc, targetLoc);

	auto dir = (currentLoc - targetLoc).GetSafeNormal();
	auto extension = (dist / 50.f) - springRestLength;
	auto force = -springStiffnes * FMath::Pow(extension, 2.f) * dir;
	

	if (force.ContainsNaN()) { return; }
	

	nodeA->ApplyForce(force);
}
