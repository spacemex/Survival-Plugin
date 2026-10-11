// Copyright All Rights Reserved. Deven Shona / Space_Mex

#pragma once

#include "CoreMinimal.h"
#include "MovementTypes.generated.h"

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	MS_Idle,
	MS_Walking,
	MS_Jogging,
	MS_Sprinting,
	MS_Crouching,
	MS_Swimming,
	MS_Falling,
};

UENUM(BlueprintType)
enum class EMovementGait : uint8
{
	MG_Walking,
	MG_Jogging,
	MG_Sprinting
};
