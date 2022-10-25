#pragma once

#include "CoreMinimal.h"


UENUM(Blueprintable)
enum class ENodeLocation : uint8
{
	None,
	Front,
	Back,
	Left,
	Right,
	Top,
	Bottom
};