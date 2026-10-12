// Copyright All Rights Reserved. Deven Shona / Space_Mex


#include "Types/SurvivalGameplayTags.h"

namespace SurvivalGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Movement_State, "Movement.State");
	UE_DEFINE_GAMEPLAY_TAG(Movement_State_Idle, "Movement.State.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Movement_State_Walking, "Movement.State.Walking");
	UE_DEFINE_GAMEPLAY_TAG(Movement_State_Jogging, "Movement.State.Jogging");
	UE_DEFINE_GAMEPLAY_TAG(Movement_State_Sprinting, "Movement.State.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(Movement_State_Crouching, "Movement.State.Crouching");
	UE_DEFINE_GAMEPLAY_TAG(Movement_State_Swimming, "Movement.State.Swimming");
	UE_DEFINE_GAMEPLAY_TAG(Movement_State_Falling, "Movement.State.Falling");

	UE_DEFINE_GAMEPLAY_TAG(Movement_Restriction, "Movement.Restriction");
	UE_DEFINE_GAMEPLAY_TAG(Movement_Restriction_NoJump, "Movement.Restriction.NoJump");
	UE_DEFINE_GAMEPLAY_TAG(Movement_Restriction_NoCrouch, "Movement.Restriction.NoCrouch");
	UE_DEFINE_GAMEPLAY_TAG(Movement_Restriction_NoMove, "Movement.Restriction.NoMove");
	UE_DEFINE_GAMEPLAY_TAG(Movement_Restriction_NoSprint, "Movement.Restriction.NoSprint");

	UE_DEFINE_GAMEPLAY_TAG(Status_Effect, "Status.Effect");

	UE_DEFINE_GAMEPLAY_TAG(Status_Effect_Exhausted, "Status.Effect.Exhausted");
	UE_DEFINE_GAMEPLAY_TAG(Status_Effect_Stunned, "Status.Effect.Stunned");
	UE_DEFINE_GAMEPLAY_TAG(Status_Effect_Slowed, "Status.Effect.Slowed");
	UE_DEFINE_GAMEPLAY_TAG(Status_Effect_Hasted, "Status.Effect.Hasted");
	UE_DEFINE_GAMEPLAY_TAG(Status_Effect_Poisoned, "Status.Effect.Poisoned");

	UE_DEFINE_GAMEPLAY_TAG(Attribute, "Attribute");

	UE_DEFINE_GAMEPLAY_TAG(Attribute_MovementSpeed, "Attribute.MovementSpeed");
}
