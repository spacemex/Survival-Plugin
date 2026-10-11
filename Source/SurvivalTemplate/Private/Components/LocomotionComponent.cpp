// Copyright All Rights Reserved. Deven Shona / Space_Mex


#include "Components/LocomotionComponent.h"

#include "Actors/Characters/BaseCharacter.h"
#include "Components/SurvivalCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Types/SurvivalGameplayTags.h"

ULocomotionComponent::ULocomotionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FGameplayTag ULocomotionComponent::GetCurrentMovementStateTag() const
{
	const ABaseCharacter* Character = CharacterOwner.Get();

	if (!Character) return FGameplayTag();

	const UStatusComponent* Status = Character->GetStatusComponent();

	if (!Status) return FGameplayTag();

	return Status->GetExclusiveStatusTag(SurvivalGameplayTags::Movement_State);
}

EMovementState ULocomotionComponent::GetCurrentMovementState() const
{
	const FGameplayTag State = GetAnimationMovementStateTag();

	using namespace SurvivalGameplayTags;

	if (State == Movement_State_Walking) return EMovementState::MS_Walking;
	if (State == Movement_State_Jogging) return EMovementState::MS_Jogging;
	if (State == Movement_State_Sprinting) return EMovementState::MS_Sprinting;
	if (State == Movement_State_Crouching) return EMovementState::MS_Crouching;
	if (State == Movement_State_Swimming) return EMovementState::MS_Swimming;
	if (State == Movement_State_Falling) return EMovementState::MS_Falling;

	return EMovementState::MS_Idle;
}

EMovementGait ULocomotionComponent::GetDesiredGait() const
{
	const ABaseCharacter* Character = CharacterOwner.Get();

	if (!Character) return EMovementGait::MG_Jogging;

	USurvivalCharacterMovementComponent* Movement = Cast<USurvivalCharacterMovementComponent>(
		Character->GetCharacterMovement());

	return Movement ? Movement->GetDesiredGait() : EMovementGait::MG_Jogging;
}


FGameplayTag ULocomotionComponent::GetAnimationMovementStateTag() const
{
	const ABaseCharacter* Character = CharacterOwner.Get();

	if (!Character)
	{
		return FGameplayTag();
	}

	if (Character->HasAuthority() || Character->IsLocallyControlled())
	{
		return CalculateMovementStateTag();
	}

	return GetCurrentMovementStateTag();
}


void ULocomotionComponent::SetDesiredGait(EMovementGait NewGait)
{
	const ABaseCharacter* Character = CharacterOwner.Get();

	if (!Character) return;

	USurvivalCharacterMovementComponent* Movement = Cast<USurvivalCharacterMovementComponent>(
		Character->GetCharacterMovement());

	if (Movement)
	{
		Movement->SetDesiredGait(NewGait);
	}
}

void ULocomotionComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterOwner = Cast<ABaseCharacter>(GetOwner());

	if (!CharacterOwner.IsValid()) return;

	if (CharacterOwner->HasAuthority())
	{
		CharacterOwner->OnCharacterMovementUpdated.AddDynamic(this, &ULocomotionComponent::HandleMovementUpdated);

		RefreshMovementState();
	}
}

void ULocomotionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CharacterOwner.IsValid())
	{
		CharacterOwner->OnCharacterMovementUpdated.RemoveDynamic(
			this, &ULocomotionComponent::HandleMovementUpdated);
	}

	Super::EndPlay(EndPlayReason);
}

void ULocomotionComponent::HandleMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector NewLocation)
{
	RefreshMovementState();
}

void ULocomotionComponent::RefreshMovementState()
{
	ABaseCharacter* Character = CharacterOwner.Get();

	if (!Character || !Character->HasAuthority()) return;

	UStatusComponent* Status = Character->GetStatusComponent();

	if (!Status) return;

	const FGameplayTag NewState = CalculateMovementStateTag();

	if (!NewState.IsValid()) return;

	Status->SetExclusiveStatusTag(SurvivalGameplayTags::Movement_State, NewState);
}

FGameplayTag ULocomotionComponent::CalculateMovementStateTag() const
{
	const ABaseCharacter* Character = CharacterOwner.Get();

	if (!Character) return FGameplayTag();

	const USurvivalCharacterMovementComponent* Movement = Cast<USurvivalCharacterMovementComponent>(
		Character->GetCharacterMovement());

	if (!Movement) return FGameplayTag();

	using namespace SurvivalGameplayTags;

	if (Movement->IsFalling()) return Movement_State_Falling;
	if (Movement->IsSwimming()) return Movement_State_Swimming;
	if (Movement->IsCrouching()) return Movement_State_Crouching;
	if (Movement->Velocity.Size2D() <= MinimumMovingSpeed) return Movement_State_Idle;

	switch (Movement->GetEffectiveGait())
	{
	case EMovementGait::MG_Walking: return Movement_State_Walking;
	case EMovementGait::MG_Sprinting: return Movement_State_Sprinting;
	case EMovementGait::MG_Jogging: return Movement_State_Jogging;
	default: return Movement_State_Jogging;
	}
}
