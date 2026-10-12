#include "Components/SurvivalCharacterMovementComponent.h"

#include "Actors/Characters/BaseCharacter.h"
#include "Components/StatusComponent.h"
#include "Types/SurvivalGameplayTags.h"
#include "Components/CharacterEffectComponent.h"

#include "GameFramework/Character.h"

// Saved movement information 
class FSavedMove_Survival : public FSavedMove_Character
{
public:
	using Super = FSavedMove_Character;

	bool bSavedWantsToWalk = false;
	bool bSavedWantsToSprint = false;

	virtual void Clear() override
	{
		Super::Clear();

		bSavedWantsToWalk = false;
		bSavedWantsToSprint = false;
	}

	virtual uint8 GetCompressedFlags() const override
	{
		uint8 Result = Super::GetCompressedFlags();

		if (bSavedWantsToWalk)
		{
			Result |= FLAG_Custom_0;
		}

		if (bSavedWantsToSprint)
		{
			Result |= FLAG_Custom_1;
		}

		return Result;
	}

	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override
	{
		const FSavedMove_Survival* Other = static_cast<const FSavedMove_Survival*>(NewMove.Get());

		if (bSavedWantsToWalk != Other->bSavedWantsToWalk ||
			bSavedWantsToSprint != Other->bSavedWantsToSprint)
		{
			return false;
		}

		return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
	}

	virtual void SetMoveFor(ACharacter* Character, float DT, const FVector& NewAccel,
	                        FNetworkPredictionData_Client_Character& ClientData) override
	{
		Super::SetMoveFor(Character, DT, NewAccel, ClientData);

		const USurvivalCharacterMovementComponent* Movement = Cast<USurvivalCharacterMovementComponent>(
			Character->GetCharacterMovement());

		if (!Movement) return;

		const EMovementGait Gait = Movement->GetDesiredGait();

		bSavedWantsToWalk = Gait == EMovementGait::MG_Walking;
		bSavedWantsToSprint = Gait == EMovementGait::MG_Sprinting;
	}

	virtual void PrepMoveFor(ACharacter* Character) override
	{
		Super::PrepMoveFor(Character);

		USurvivalCharacterMovementComponent* Movement = Cast<USurvivalCharacterMovementComponent>(
			Character->GetCharacterMovement());

		if (!Movement) return;

		EMovementGait Gait = EMovementGait::MG_Jogging;

		if (bSavedWantsToWalk)
		{
			Gait = EMovementGait::MG_Walking;
		}
		else if (bSavedWantsToSprint)
		{
			Gait = EMovementGait::MG_Sprinting;
		}

		Movement->SetDesiredGait(Gait);
	}
};

namespace
{
	bool HasMovementRestriction(const ACharacter* Character, const FGameplayTag& RestrictionTag)
	{
		const ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(Character);

		if (!BaseCharacter) return false;

		const UStatusComponent* Status = BaseCharacter->GetStatusComponent();

		return Status && Status->HasStatusTag(RestrictionTag);
	}
}

// Client prediction
class FNetworkPredictionData_Client_Survival : public FNetworkPredictionData_Client_Character
{
public:
	explicit FNetworkPredictionData_Client_Survival(const UCharacterMovementComponent& ClientMovement) :
		FNetworkPredictionData_Client_Character(ClientMovement)
	{
	}

	virtual FSavedMovePtr AllocateNewMove() override
	{
		return FSavedMovePtr(new FSavedMove_Survival());
	}
};

USurvivalCharacterMovementComponent::USurvivalCharacterMovementComponent()
{
	bWantsToWalk = false;
	bWantsToSprint = false;
}

void USurvivalCharacterMovementComponent::SetDesiredGait(EMovementGait NewGait)
{
	if (!CharacterOwner || (!CharacterOwner->HasAuthority() && !CharacterOwner->IsLocallyControlled())) return;

	bWantsToWalk = NewGait == EMovementGait::MG_Walking;
	bWantsToSprint = NewGait == EMovementGait::MG_Sprinting;
}

EMovementGait USurvivalCharacterMovementComponent::GetDesiredGait() const
{
	if (bWantsToWalk) return EMovementGait::MG_Walking;
	if (bWantsToSprint) return EMovementGait::MG_Sprinting;

	return EMovementGait::MG_Jogging;
}

EMovementGait USurvivalCharacterMovementComponent::GetEffectiveGait() const
{
	const EMovementGait Desired = GetDesiredGait();

	if (Desired != EMovementGait::MG_Sprinting)
	{
		return Desired;
	}

	using namespace SurvivalGameplayTags;

	if (HasMovementRestriction(CharacterOwner, Movement_Restriction_NoSprint) ||
		HasMovementRestriction(CharacterOwner, Movement_Restriction_NoMove))
	{
		return EMovementGait::MG_Jogging;
	}

	return Desired;
}

float USurvivalCharacterMovementComponent::GetMaxSpeed() const
{
	using namespace SurvivalGameplayTags;

	if (HasMovementRestriction(CharacterOwner, Movement_Restriction_NoMove))
	{
		return 0.f;
	}

	if (!IsMovingOnGround())
	{
		return Super::GetMaxSpeed();
	}

	const ABaseCharacter* Character = Cast<ABaseCharacter>(CharacterOwner);

	if (!Character)
	{
		return Super::GetMaxSpeed();
	}

	float BaseSpeed = Character->GetJogSpeed();

	if (IsCrouching())
	{
		BaseSpeed = Character->GetCrouchSpeed();
	}
	else
	{
		switch (GetEffectiveGait())
		{
		case EMovementGait::MG_Walking: BaseSpeed = Character->GetWalkSpeed();
			break;
		case EMovementGait::MG_Sprinting: BaseSpeed = Character->GetSprintSpeed();
			break;
		case EMovementGait::MG_Jogging:
		default: BaseSpeed = Character->GetJogSpeed();
			break;
		}
	}

	const UCharacterEffectComponent* Effects = Character->GetCharacterEffectComponent();

	if (Effects)
	{
		BaseSpeed = Effects->GetModifiedAttributeValue(Attribute_MovementSpeed, BaseSpeed);
	}

	return FMath::Max(0.f, BaseSpeed);
}

void USurvivalCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToWalk = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

bool USurvivalCharacterMovementComponent::CanAttemptJump() const
{
	using namespace SurvivalGameplayTags;

	if (HasMovementRestriction(CharacterOwner, Movement_Restriction_NoJump))
	{
		return false;
	}

	if (HasMovementRestriction(CharacterOwner, Movement_Restriction_NoMove))
	{
		return false;
	}

	return Super::CanAttemptJump();
}

bool USurvivalCharacterMovementComponent::CanCrouchInCurrentState() const
{
	using namespace SurvivalGameplayTags;

	if (HasMovementRestriction(CharacterOwner, Movement_Restriction_NoCrouch))
	{
		return false;
	}

	if (HasMovementRestriction(CharacterOwner, Movement_Restriction_NoMove))
	{
		return false;
	}

	return Super::CanCrouchInCurrentState();
}

float USurvivalCharacterMovementComponent::GetMaxAcceleration() const
{
	if (HasMovementRestriction(CharacterOwner, SurvivalGameplayTags::Movement_Restriction_NoMove))
	{
		return 0.f;
	}

	return Super::GetMaxAcceleration();
}

FNetworkPredictionData_Client* USurvivalCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (!ClientPredictionData)
	{
		USurvivalCharacterMovementComponent* MutableThis = const_cast<USurvivalCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Survival(*this);
	}

	return ClientPredictionData;
}
