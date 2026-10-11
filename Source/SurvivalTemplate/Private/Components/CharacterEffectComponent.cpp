#include "Components/CharacterEffectComponent.h"

#include "Actors/Characters/BaseCharacter.h"
#include "Components/StatusComponent.h"
#include "Types/SurvivalGameplayTags.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

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

	if (!Spec.EffectTag.IsValid() || Spec.EffectTag == SurvivalGameplayTags::Status_Effect || !Spec.EffectTag.
		MatchesTag(SurvivalGameplayTags::Status_Effect))
	{
		return false;
	}

	for (const FGameplayTag& Tag :
	     Spec.GrantedTags.GetGameplayTagArray())
	{
		if (Tag == Spec.EffectTag || Tag.MatchesTag(SurvivalGameplayTags::Movement_State))
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

		if (!Effect.GrantedTags.HasAllExact(Spec.GrantedTags) || !Spec.GrantedTags.HasAllExact(Effect.GrantedTags))
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
			OnEffectChanged.Broadcast(Effect.EffectTag, Effect.StackCount);

			ScheduleNextExpiration();
			GetOwner()->ForceNetUpdate();
		}

		return bChanged;
	}

	FActiveCharacterEffect NewEffect;

	NewEffect.EffectTag = Spec.EffectTag;
	NewEffect.GrantedTags = Spec.GrantedTags;
	NewEffect.StackCount = 1;
	NewEffect.MaxStacks = FMath::Max(1, Spec.MaxStacks);
	NewEffect.DurationSeconds = FMath::Max(0.f, Spec.DurationSeconds);

	NewEffect.bRefreshDurationOnReapply = Spec.bRefreshDurationOnReapply;

	NewEffect.EndServerTimeSeconds = NewEffect.DurationSeconds > 0.f ? Now + NewEffect.DurationSeconds : 0.f;

	ActiveEffects.Add(NewEffect);

	Status->GrantStatusTag(NewEffect.EffectTag);

	for (const FGameplayTag& Tag :
	     NewEffect.GrantedTags.GetGameplayTagArray())
	{
		Status->GrantStatusTag(Tag);
	}

	OnEffectChanged.Broadcast(NewEffect.EffectTag, NewEffect.StackCount);

	ScheduleNextExpiration();
	GetOwner()->ForceNetUpdate();

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
