#include "Components/CharacterEffectComponent.h"

#include "Actors/Characters/BaseCharacter.h"
#include "Components/StatusComponent.h"
#include "Types/SurvivalGameplayTags.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

namespace
{
	bool MatchingModifiers(const TArray<FCharacterEffectModifier>& A, const TArray<FCharacterEffectModifier>& B)
	{
		if (A.Num() != B.Num())
		{
			return false;
		}

		for (int32 Index = 0; Index < A.Num(); ++Index)
		{
			if (A[Index].AttributeTag != B[Index].AttributeTag ||
				A[Index].Operation != B[Index].Operation ||
				!FMath::IsNearlyEqual(A[Index].Magnitude, B[Index].Magnitude))
			{
				return false;
			}
		}

		return true;
	}
}

UCharacterEffectComponent::UCharacterEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCharacterEffectComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterEffectComponent, ActiveEffects);
}

void UCharacterEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		FTimerManager& Timers = GetWorld()->GetTimerManager();

		for (auto& Entry : PeriodicEffectTimers)
		{
			Timers.ClearTimer(Entry.Value);
		}
	}

	PeriodicEffectTimers.Empty();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ExpirationTimer);
	}

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());

		UStatusComponent* Status = Character ? Character->GetStatusComponent() : nullptr;

		if (Status)
		{
			for (const FActiveCharacterEffect& Effect : ActiveEffects)
			{
				Status->RevokeGrantedStatusTag(Effect.EffectTag);

				for (const FGameplayTag& Tag : Effect.GrantedTags.GetGameplayTagArray())
				{
					Status->RevokeGrantedStatusTag(Tag);
				}
			}
		}

		ActiveEffects.Empty();
	}

	Super::EndPlay(EndPlayReason);
}

bool UCharacterEffectComponent::ApplyEffect(const FCharacterEffectSpec& Spec)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !GetWorld()) return false;

	using namespace SurvivalGameplayTags;

	if (!Spec.EffectTag.IsValid() || Spec.EffectTag == Status_Effect || !Spec.EffectTag.MatchesTag(Status_Effect))
	{
		return false;
	}

	if (!FMath::IsFinite(Spec.DurationSeconds) || !FMath::IsFinite(Spec.PeriodicIntervalSeconds) || !
		FMath::IsFinite(Spec.PeriodicMagnitudePerStack) || Spec.PeriodicIntervalSeconds < 0.f || (Spec.
			PeriodicIntervalSeconds > 0.f && Spec.PeriodicIntervalSeconds < 0.1f))
	{
		return false;
	}

	for (const FGameplayTag& Tag : Spec.GrantedTags.GetGameplayTagArray())
	{
		if (Tag == Spec.EffectTag || Tag.MatchesTag(Movement_State))
		{
			return false;
		}
	}

	for (const FCharacterEffectModifier& Modifier : Spec.Modifiers)
	{
		if (!Modifier.AttributeTag.IsValid() || !FMath::IsFinite(Modifier.Magnitude))
		{
			return false;
		}

		if (Modifier.Operation == ECharacterModifierOperation::Multiply && Modifier.Magnitude < 0.f)
		{
			return false;
		}

		if (Modifier.Operation != ECharacterModifierOperation::Add && Modifier.Operation !=
			ECharacterModifierOperation::Multiply)
		{
			return false;
		}
	}

	ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());

	UStatusComponent* Status = Character ? Character->GetStatusComponent() : nullptr;

	if (!Status) return false;

	const float Now = GetWorld()->GetTimeSeconds();

	for (FActiveCharacterEffect& Effect : ActiveEffects)
	{
		if (Effect.EffectTag != Spec.EffectTag) continue;

		if (!Effect.GrantedTags.HasAllExact(Spec.GrantedTags) || !Spec.GrantedTags.HasAllExact(Effect.GrantedTags) || !
			MatchingModifiers(Effect.Modifiers, Spec.Modifiers) || Effect.MaxStacks != FMath::Max(1, Spec.MaxStacks) ||
			!FMath::IsNearlyEqual(Effect.DurationSeconds, FMath::Max(0.f, Spec.DurationSeconds)) || Effect.
			bRefreshDurationOnReapply != Spec.bRefreshDurationOnReapply || !
			FMath::IsNearlyEqual(Effect.PeriodicIntervalSeconds, Spec.PeriodicIntervalSeconds) || !FMath::IsNearlyEqual(
				Effect.PeriodicMagnitudePerStack, Spec.PeriodicMagnitudePerStack))
		{
			return false;
		}

		bool bChanged = false;

		if (Effect.StackCount < Effect.MaxStacks)
		{
			++Effect.StackCount;
			bChanged = true;
		}

		if (Effect.DurationSeconds > 0.f && Effect.bRefreshDurationOnReapply)
		{
			Effect.EndServerTimeSeconds = Now + Effect.DurationSeconds;
			bChanged = true;
		}

		if (bChanged)
		{
			const FGameplayTag Tag = Effect.EffectTag;
			const int32 Stacks = Effect.StackCount;

			ScheduleNextExpiration();
			GetOwner()->ForceNetUpdate();

			OnEffectChanged.Broadcast(Tag, Stacks);
		}

		return bChanged;
	}

	FActiveCharacterEffect NewEffect;

	NewEffect.EffectTag = Spec.EffectTag;
	NewEffect.GrantedTags = Spec.GrantedTags;
	NewEffect.Modifiers = Spec.Modifiers;

	NewEffect.StackCount = 1;
	NewEffect.MaxStacks = FMath::Max(1, Spec.MaxStacks);

	NewEffect.DurationSeconds = FMath::Max(0.f, Spec.DurationSeconds);

	NewEffect.bRefreshDurationOnReapply = Spec.bRefreshDurationOnReapply;

	NewEffect.PeriodicIntervalSeconds = Spec.PeriodicIntervalSeconds;

	NewEffect.PeriodicMagnitudePerStack = Spec.PeriodicMagnitudePerStack;

	NewEffect.EndServerTimeSeconds = NewEffect.DurationSeconds > 0.f ? Now + NewEffect.DurationSeconds : 0.f;

	ActiveEffects.Add(NewEffect);

	Status->GrantStatusTag(NewEffect.EffectTag);

	for (const FGameplayTag& Tag :
	     NewEffect.GrantedTags.GetGameplayTagArray())
	{
		Status->GrantStatusTag(Tag);
	}

	ScheduleNextExpiration();
	StartPeriodicTimer(NewEffect);

	GetOwner()->ForceNetUpdate();

	OnEffectChanged.Broadcast(NewEffect.EffectTag, 1);

	return true;
}

bool UCharacterEffectComponent::RemoveEffect(FGameplayTag EffectTag)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return false;

	for (int32 Index = 0; Index < ActiveEffects.Num(); ++Index)
	{
		if (ActiveEffects[Index].EffectTag == EffectTag)
		{
			RemoveEffectAtIndex(Index);
			ScheduleNextExpiration();
			return true;
		}
	}

	return false;
}

void UCharacterEffectComponent::RemoveEffectAtIndex(int32 Index)
{
	if (!ActiveEffects.IsValidIndex(Index)) return;

	const FActiveCharacterEffect RemovedEffect = ActiveEffects[Index];

	StopPeriodicTimer(RemovedEffect.EffectTag);
	ActiveEffects.RemoveAt(Index);

	ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());

	UStatusComponent* Status = Character ? Character->GetStatusComponent() : nullptr;

	if (Status)
	{
		Status->RevokeGrantedStatusTag(RemovedEffect.EffectTag);

		for (const FGameplayTag& Tag : RemovedEffect.GrantedTags.GetGameplayTagArray())
		{
			Status->RevokeGrantedStatusTag(Tag);
		}
	}

	OnEffectChanged.Broadcast(RemovedEffect.EffectTag, 0);

	if (GetOwner())
	{
		GetOwner()->ForceNetUpdate();
	}
}

void UCharacterEffectComponent::ScheduleNextExpiration()
{
	if (!GetWorld() || !GetOwner() || !GetOwner()->HasAuthority()) return;

	FTimerManager& Timers = GetWorld()->GetTimerManager();

	Timers.ClearTimer(ExpirationTimer);

	const float Now = GetWorld()->GetTimeSeconds();

	float NextExpiration = TNumericLimits<float>::Max();

	for (const FActiveCharacterEffect& Effect : ActiveEffects)
	{
		if (Effect.EndServerTimeSeconds <= 0.f) continue;

		NextExpiration = FMath::Min(NextExpiration, Effect.EndServerTimeSeconds);
	}

	if (NextExpiration == TNumericLimits<float>::Max()) return;

	const float Delay = FMath::Max(0.01f, NextExpiration - Now);

	Timers.SetTimer(ExpirationTimer, this, &UCharacterEffectComponent::ExpireEffects, Delay, false);
}

void UCharacterEffectComponent::ExpireEffects()
{
	if (!GetWorld() || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();

	for (int32 Index = ActiveEffects.Num() - 1; Index >= 0; --Index)
	{
		const FActiveCharacterEffect& Effect = ActiveEffects[Index];

		if (Effect.EndServerTimeSeconds > 0.f && Effect.EndServerTimeSeconds <= Now)
		{
			RemoveEffectAtIndex(Index);
		}
	}

	ScheduleNextExpiration();
}

bool UCharacterEffectComponent::HasEffect(FGameplayTag EffectTag) const
{
	for (const FActiveCharacterEffect& Effect : ActiveEffects)
	{
		if (Effect.EffectTag == EffectTag) return true;
	}

	return false;
}

int32 UCharacterEffectComponent::GetEffectStackCount(FGameplayTag EffectTag) const
{
	for (const FActiveCharacterEffect& Effect : ActiveEffects)
	{
		if (Effect.EffectTag == EffectTag)
		{
			return Effect.StackCount;
		}
	}

	return 0;
}

float UCharacterEffectComponent::GetEffectRemainingTime(FGameplayTag EffectTag) const
{
	if (!GetWorld())
		return 0.f;

	for (const FActiveCharacterEffect& Effect : ActiveEffects)
	{
		if (Effect.EffectTag != EffectTag) continue;

		if (Effect.EndServerTimeSeconds <= 0.f) return -1.f;
		double ServerTime = GetWorld()->GetTimeSeconds();

		if (!GetOwner() || !GetOwner()->HasAuthority())
		{
			AGameStateBase* GameState = GetWorld()->GetGameState();

			if (GameState)
			{
				ServerTime = GameState->GetServerWorldTimeSeconds();
			}
		}

		return FMath::Max(0.f, static_cast<float>(Effect.EndServerTimeSeconds - ServerTime));
	}

	return 0.f;
}

TArray<FActiveCharacterEffect> UCharacterEffectComponent::GetActiveEffects() const
{
	return ActiveEffects;
}

float UCharacterEffectComponent::GetModifiedAttributeValue(FGameplayTag AttributeTag, float BaseValue) const
{
	if (!AttributeTag.IsValid())
	{
		return BaseValue;
	}

	float Additive = 0.f;
	float Multiplier = 1.f;

	for (const FActiveCharacterEffect& Effect : ActiveEffects)
	{
		for (const FCharacterEffectModifier& Modifier :
		     Effect.Modifiers)
		{
			if (Modifier.AttributeTag != AttributeTag)
			{
				continue;
			}

			switch (Modifier.Operation)
			{
			case ECharacterModifierOperation::Add:
				Additive += Modifier.Magnitude * Effect.StackCount;
				break;
			case ECharacterModifierOperation::Multiply:
				Multiplier *= FMath::Pow(Modifier.Magnitude, Effect.StackCount);
				break;
			}
		}
	}

	return (BaseValue + Additive) * Multiplier;
}

void UCharacterEffectComponent::OnRep_ActiveEffects(const TArray<FActiveCharacterEffect>& PreviousEffects)
{
	for (const FActiveCharacterEffect& Effect : ActiveEffects)
	{
		const FActiveCharacterEffect* Previous = PreviousEffects.FindByPredicate(
			[&Effect](const FActiveCharacterEffect& Old)
			{
				return Old.EffectTag == Effect.EffectTag;
			});

		if (!Previous || Previous->StackCount != Effect.StackCount || !FMath::IsNearlyEqual(
			Previous->EndServerTimeSeconds, Effect.EndServerTimeSeconds))
		{
			OnEffectChanged.Broadcast(Effect.EffectTag, Effect.StackCount);
		}
	}

	for (const FActiveCharacterEffect& Previous : PreviousEffects)
	{
		const bool bStillActive = ActiveEffects.ContainsByPredicate([&Previous](const FActiveCharacterEffect& Effect)
		{
			return Effect.EffectTag == Previous.EffectTag;
		});

		if (!bStillActive)
		{
			OnEffectChanged.Broadcast(Previous.EffectTag, 0);
		}
	}
}

void UCharacterEffectComponent::StartPeriodicTimer(const FActiveCharacterEffect& Effect)
{
	if (!GetWorld() || !GetOwner() || !GetOwner()->HasAuthority() || Effect.PeriodicIntervalSeconds <= 0.f)
	{
		return;
	}

	FTimerHandle& Handle = PeriodicEffectTimers.FindOrAdd(Effect.EffectTag);
	FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &UCharacterEffectComponent::ExecutePeriodicEffect,
	                                                        Effect.EffectTag);

	GetWorld()->GetTimerManager().SetTimer(Handle, Delegate, Effect.PeriodicIntervalSeconds, true);
}

void UCharacterEffectComponent::StopPeriodicTimer(FGameplayTag EffectTag)
{
	if (FTimerHandle* Handle =
		PeriodicEffectTimers.Find(EffectTag))
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(*Handle);
		}

		PeriodicEffectTimers.Remove(EffectTag);
	}
}

void UCharacterEffectComponent::ExecutePeriodicEffect(FGameplayTag EffectTag)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !GetWorld())
	{
		return;
	}

	const FActiveCharacterEffect* Effect = ActiveEffects.FindByPredicate(
		[EffectTag](const FActiveCharacterEffect& Entry)
		{
			return Entry.EffectTag == EffectTag;
		});

	if (!Effect || Effect->PeriodicIntervalSeconds <= 0.f)
	{
		return;
	}

	if (Effect->EndServerTimeSeconds > 0.f && GetWorld()->GetTimeSeconds() >= Effect->EndServerTimeSeconds)
	{
		return;
	}

	const int32 Stacks = Effect->StackCount;
	const float Magnitude = Effect->PeriodicMagnitudePerStack * Stacks;

	OnEffectPeriodicNative.Broadcast(EffectTag, Magnitude, Stacks);
	OnEffectPeriodic.Broadcast(EffectTag, Magnitude, Stacks);
}
