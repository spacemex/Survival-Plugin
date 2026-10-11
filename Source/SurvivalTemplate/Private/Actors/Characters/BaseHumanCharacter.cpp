// Copyright All Rights Reserved. Deven Shona / Space_Mex


#include "Actors/Characters/BaseHumanCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

ABaseHumanCharacter::ABaseHumanCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
}
