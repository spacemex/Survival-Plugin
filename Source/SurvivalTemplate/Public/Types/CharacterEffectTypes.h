// Copyright All Rights Reserved. Deven Shona / Space_Mex

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CharacterEffectTypes.generated.h"

UENUM()
enum class ECharacterModifierOperation : uint8
{
	Add,
	Multiply
};

USTRUCT(BlueprintType)
struct SURVIVALTEMPLATE_API FCharacterEffectModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier")
	FGameplayTag AttributeTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier")
	ECharacterModifierOperation Operation = ECharacterModifierOperation::Multiply;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier")
	float Magnitude = 1.f;
};

USTRUCT(BlueprintType)
struct SURVIVALTEMPLATE_API FCharacterEffectSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Effect")
	FGameplayTag EffectTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Effect")
	FGameplayTagContainer GrantedTags;
	// 0 Grants Indefinite - Until Removed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect", meta=(ClampMin="0.0"))
	float DurationSeconds = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect", meta=(ClampMin="1"))
	int32 MaxStacks = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect")
	bool bRefreshDurationOnReapply = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect|Modifiers")
	TArray<FCharacterEffectModifier> Modifiers;
	// 0 Disables Periodic Execution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect|Periodic", meta=(ClampMin="0.0"))
	float PeriodicIntervalSeconds = 0.f;
	// Magnitude Of Each Execution Before Applying Stack Count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect|Periodic")
	float PeriodicMagnitudePerStack = 0.f;
};

USTRUCT(BlueprintType)
struct SURVIVALTEMPLATE_API FActiveCharacterEffect
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Effect")
	FGameplayTag EffectTag;
	UPROPERTY(BlueprintReadOnly, Category="Effect")
	FGameplayTagContainer GrantedTags;
	UPROPERTY(BlueprintReadOnly, Category="Effect")
	int32 StackCount = 1;
	UPROPERTY(BlueprintReadOnly, Category="Effect")
	int32 MaxStacks = 1;
	UPROPERTY(BlueprintReadOnly, Category="Effect")
	float DurationSeconds = 0.f;
	/**
	 * Server World Time When This Effect Expires.
	 * <P>
	 * 0 Indicates Indefinite Effect
	 */
	UPROPERTY(BlueprintReadOnly, Category="Effect")
	float EndServerTimeSeconds = 0.f;
	UPROPERTY(BlueprintReadOnly, Category="Effect")
	bool bRefreshDurationOnReapply = true;


	UPROPERTY(BlueprintReadOnly, Category="Effect|Modifiers")
	TArray<FCharacterEffectModifier> Modifiers;
	UPROPERTY(BlueprintReadOnly, Category="Effect|Periodic")
	float PeriodicIntervalSeconds = 0.f;
	UPROPERTY(BlueprintReadOnly, Category="Effect|Periodic")
	float PeriodicMagnitudePerStack = 0.f;
};
