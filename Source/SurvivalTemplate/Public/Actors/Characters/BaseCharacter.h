// Copyright All Rights Reserved. Deven Shona / Space_Mex

#pragma once

#include "CoreMinimal.h"
#include "Components/StatusComponent.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class ULocomotionComponent;
class UStatusComponent;

UCLASS(Abstract)
class SURVIVALTEMPLATE_API ABaseCharacter : public ACharacter
{
private:
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStatusComponent> StatusComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<ULocomotionComponent> LocomotionComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	float WalkSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	float JogSpeed = 400.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	float SprintSpeed = 600.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	float CrouchSpeed = 75.f;

public:
	ABaseCharacter(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category="Movement|Getter")
	virtual float GetWalkSpeed() const { return WalkSpeed; }

	UFUNCTION(BlueprintPure, Category="Movement|Getter")
	virtual float GetJogSpeed() const { return JogSpeed; }

	UFUNCTION(BlueprintPure, Category="Movement|Getter")
	virtual float GetSprintSpeed() const { return SprintSpeed; }

	UFUNCTION(BlueprintPure, Category="Movement|Getter")
	virtual float GetCrouchSpeed() const { return CrouchSpeed; }

	UFUNCTION(BlueprintPure, Category="Components")
	UStatusComponent* GetStatusComponent() const { return StatusComponent; }

	UFUNCTION(BlueprintPure, Category="Components")
	ULocomotionComponent* GetLocomotionComponent() const { return LocomotionComponent; }
};
