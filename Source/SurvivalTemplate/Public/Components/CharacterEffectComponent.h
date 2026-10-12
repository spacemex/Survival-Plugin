// Copyright All Rights Reserved. Deven Shona / Space_Mex

// ReSharper disable CppOverrideWithDifferentVisibility
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/CharacterEffectTypes.h"
#include "TimerManager.h"
#include "CharacterEffectComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterEffectChanged, FGameplayTag, EffectTag, int32, StackCount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCharacterEffectPeriodic, FGameplayTag, EffectTag, float, Magnitude,
                                               int32, StackCount);

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnCharacterEffectPeriodicNative, FGameplayTag, float, int32);


UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class SURVIVALTEMPLATE_API UCharacterEffectComponent : public UActorComponent
{
private:
	GENERATED_BODY()

	UPROPERTY(ReplicatedUsing=OnRep_ActiveEffects)
	TArray<FActiveCharacterEffect> ActiveEffects;

	FTimerHandle ExpirationTimer;

	UFUNCTION()
	void OnRep_ActiveEffects(const TArray<FActiveCharacterEffect>& PreviousEffects);

	void RemoveEffectAtIndex(int32 Index);
	void ScheduleNextExpiration();
	void ExpireEffects();

	TMap<FGameplayTag, FTimerHandle> PeriodicEffectTimers;

	void StartPeriodicTimer(const FActiveCharacterEffect& Effect);
	void StopPeriodicTimer(FGameplayTag EffectTag);
	void ExecutePeriodicEffect(FGameplayTag EffectTag);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UCharacterEffectComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category="Effects")
	FOnCharacterEffectChanged OnEffectChanged;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Effects")
	bool ApplyEffect(const FCharacterEffectSpec& Spec);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Effects")
	bool RemoveEffect(FGameplayTag EffectTag);

	UFUNCTION(BlueprintPure, Category="Effects")
	bool HasEffect(FGameplayTag EffectTag) const;

	UFUNCTION(BlueprintPure, Category="Effects")
	int32 GetEffectStackCount(FGameplayTag EffectTag) const;

	UFUNCTION(BlueprintPure, Category="Effects")
	float GetEffectRemainingTime(FGameplayTag EffectTag) const;

	UFUNCTION(BlueprintPure, Category="Effects")
	TArray<FActiveCharacterEffect> GetActiveEffects() const;

	UFUNCTION(BlueprintPure, Category="Effects|Modifiers")
	float GetModifiedAttributeValue(FGameplayTag AttributeTag, float BaseValue) const;
	
	UPROPERTY(BlueprintAssignable, Category="Effects|Periodic")
	FOnCharacterEffectPeriodic OnEffectPeriodic;

	FOnCharacterEffectPeriodicNative OnEffectPeriodicNative;
};
