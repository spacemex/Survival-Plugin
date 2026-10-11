// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Characters/BaseCharacter.h"

#include "Components/CharacterEffectComponent.h"
#include "Components/LocomotionComponent.h"
#include "Components/StatusComponent.h"
#include "Components/SurvivalCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer.SetDefaultSubobjectClass<USurvivalCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName)
)
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
	LocomotionComponent = CreateDefaultSubobject<ULocomotionComponent>(TEXT("LocomotionComponent"));
	CharacterEffectComponent = CreateDefaultSubobject<UCharacterEffectComponent>(TEXT("EffectComponent"));

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = false;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	UCharacterMovementComponent* Movement = GetCharacterMovement();

	if (Movement)
	{
		Movement->MaxWalkSpeed = GetJogSpeed();
		Movement->MaxWalkSpeedCrouched = GetCrouchSpeed();
	}
}
