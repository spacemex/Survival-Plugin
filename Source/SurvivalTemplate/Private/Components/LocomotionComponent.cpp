// Copyright All Rights Reserved. Deven Shona / Space_Mex


#include "Components/LocomotionComponent.h"

#include "Actors/Characters/BaseCharacter.h"
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
	const FGameplayTag State = GetCurrentMovementStateTag();

	using namespace SurvivalGameplayTags;

	if (State == Movement_State_Walking) return EMovementState::MS_Walking;
	if (State == Movement_State_Jogging) return EMovementState::MS_Jogging;
	if (State == Movement_State_Sprinting) return EMovementState::MS_Sprinting;
	if (State == Movement_State_Crouching) return EMovementState::MS_Crouching;
	if (State == Movement_State_Swimming) return EMovementState::MS_Swimming;
	if (State == Movement_State_Falling) return EMovementState::MS_Falling;

	return EMovementState::MS_Idle;
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

	const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();

	if (!Movement) return FGameplayTag();

	using namespace SurvivalGameplayTags;

	if (Movement->IsFalling()) return Movement_State_Falling;
	if (Movement->IsSwimming()) return Movement_State_Swimming;
	if (Movement->IsCrouching()) return Movement_State_Crouching;

	const float Speed = Movement->Velocity.Size2D();

	if (Speed <= MinimumMovingSpeed) return Movement_State_Idle;

	const float WalkJogThreshold = (Character->GetWalkSpeed() + Character->GetJogSpeed()) * 0.5f;
	const float JogSprintThreshold = (Character->GetJogSpeed() + Character->GetSprintSpeed()) * 0.5f;

	if (Speed < WalkJogThreshold) return Movement_State_Walking;
	if (Speed < JogSprintThreshold) return Movement_State_Jogging;

	return Movement_State_Sprinting;
}
