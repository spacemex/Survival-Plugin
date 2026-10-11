// Copyright All Rights Reserved. Deven Shona / Space_Mex

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CharacterEffectTypes.generated.h"

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
};
