// Copyright All Rights Reserved. Deven Shona / Space_Mex

#pragma once

#include "CoreMinimal.h"
#include "Components/StatusComponent.h"
#include "GameFramework/Character.h"
#include "Types/MovementTypes.h"
#include "BaseCharacter.generated.h"

UCLASS(Abstract)
class SURVIVALTEMPLATE_API ABaseCharacter : public ACharacter
{
private:
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStatusComponent> StatusComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	float WalkSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	float JogSpeed = 400.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	float SprintSpeed = 600.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	float CrouchSpeed = 75.f;

public:
	ABaseCharacter();

	UFUNCTION(BlueprintPure, Category="Movement|Getter")
	virtual float GetWalkSpeed() const { return WalkSpeed; }

	UFUNCTION(BlueprintPure, Category="Movement|Getter")
	virtual float GetJogSpeed() const { return JogSpeed; }

	UFUNCTION(BlueprintPure, Category="Movement|Getter")
	virtual float GetSprintSpeed() const { return SprintSpeed; }

	UFUNCTION(BlueprintPure, Category="Movement|Getter")
	virtual float GetCrouchSpeed() const { return CrouchSpeed; }

	UFUNCTION(BlueprintPure, Category="Components")
	TObjectPtr<UStatusComponent> GetStatusComponent() const { return StatusComponent; }
};
