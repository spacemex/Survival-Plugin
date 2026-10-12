// Copyright All Rights Reserved. Deven Shona / Space_Mex


#include "Actors/Characters/BaseCharacter.h"

#include "Components/CharacterEffectComponent.h"
#include "Components/LocomotionComponent.h"
#include "Components/StatusComponent.h"
#include "Components/SurvivalCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Types/SurvivalGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"


void ABaseCharacter::HandlePeriodicEffect(FGameplayTag EffectTag, float Magnitude, int32 StackCount)
{
	if (!HasAuthority())
	{
		return;
	}

	using namespace SurvivalGameplayTags;

	if (EffectTag == Status_Effect_Poisoned)
	{
		UGameplayStatics::ApplyDamage(this, FMath::Max(0.f, Magnitude), GetController(), this,
		                              UDamageType::StaticClass());
	}
}

ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer.SetDefaultSubobjectClass<USurvivalCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
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

	if (HasAuthority() && CharacterEffectComponent)
	{
		CharacterEffectComponent->OnEffectPeriodicNative.AddUObject(this, &ABaseCharacter::HandlePeriodicEffect);
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();

	if (Movement)
	{
		Movement->MaxWalkSpeed = GetJogSpeed();
		Movement->MaxWalkSpeedCrouched = GetCrouchSpeed();
	}
}
