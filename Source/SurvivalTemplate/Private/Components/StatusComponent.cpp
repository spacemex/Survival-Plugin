// Copyright All Rights Reserved. Deven Shona / Space_Mex


#include "Components/StatusComponent.h"

#include "Net/UnrealNetwork.h"

UStatusComponent::UStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UStatusComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UStatusComponent, StatusTags);
}

void UStatusComponent::OnRep_StatusTags(const FGameplayTagContainer& PreviousTags)
{
	for (const FGameplayTag& Tag : StatusTags.GetGameplayTagArray())
	{
		if (!PreviousTags.HasTagExact(Tag))
		{
			NotifyTagChanged(Tag, true);
		}
	}

	for (const FGameplayTag& Tag : PreviousTags.GetGameplayTagArray())
	{
		if (!StatusTags.HasTagExact(Tag))
		{
			NotifyTagChanged(Tag, false);
		}
	}
}

void UStatusComponent::NotifyTagChanged(FGameplayTag Tag, bool bAdded)
{
	OnStatusTagChanged.Broadcast(Tag, bAdded);
}

bool UStatusComponent::AddStatusTag(const FGameplayTag Tag)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return false;
	if (!Tag.IsValid() || StatusTags.HasTagExact(Tag)) return false;

	StatusTags.AddTag(Tag);
	NotifyTagChanged(Tag, true);
	return true;
}

bool UStatusComponent::RemoveStatusTag(const FGameplayTag Tag)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return false;
	if (!Tag.IsValid() || !StatusTags.HasTagExact(Tag)) return false;

	StatusTags.RemoveTag(Tag);
	NotifyTagChanged(Tag, false);
	return true;
}

bool UStatusComponent::HasStatusTag(const FGameplayTag Tag) const
{
	return StatusTags.HasTag(Tag);
}

FGameplayTagContainer UStatusComponent::GetStatusTags() const
{
	return StatusTags;
}
