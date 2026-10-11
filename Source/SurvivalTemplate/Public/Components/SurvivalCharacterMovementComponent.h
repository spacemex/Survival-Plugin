// Copyright All Rights Reserved. Deven Shona / Space_Mex

// ReSharper disable CppOverrideWithDifferentVisibility
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Types/MovementTypes.h"
#include "SurvivalCharacterMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class SURVIVALTEMPLATE_API USurvivalCharacterMovementComponent : public UCharacterMovementComponent
{
private:
	GENERATED_BODY()

	uint8 bWantsToWalk : 1;
	uint8 bWantsToSprint : 1;

public:
	USurvivalCharacterMovementComponent();

	void SetDesiredGait(EMovementGait NewGait);

	EMovementGait GetDesiredGait() const;
	EMovementGait GetEffectiveGait() const;

	virtual float GetMaxSpeed() const override;

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual bool CanAttemptJump() const override;
	virtual float GetMaxAcceleration() const override;
	virtual bool CanCrouchInCurrentState() const override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
};
