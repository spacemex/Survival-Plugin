// Copyright All Rights Reserved. Deven Shona / Space_Mex

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Types/MovementTypes.h"
#include "LocomotionComponent.generated.h"

class ABaseCharacter;

UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class SURVIVALTEMPLATE_API ULocomotionComponent : public UActorComponent
{
private:
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement", meta=(ClampMin="0.0"))
	float MinimumMovingSpeed = 10.f;

	UFUNCTION()
	void HandleMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector NewLocation);

	void RefreshMovementState();

	FGameplayTag CalculateMovementStateTag() const;

private:
	TWeakObjectPtr<ABaseCharacter> CharacterOwner;
	
public:
	ULocomotionComponent();
	
	UFUNCTION(BlueprintPure, Category="Movement")
	FGameplayTag GetCurrentMovementStateTag() const;
	
	UFUNCTION(BlueprintPure, Category="Movement")
	EMovementState GetCurrentMovementState() const;
};
