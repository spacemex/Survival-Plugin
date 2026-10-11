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

bool UStatusComponent::SetExclusiveStatusTag(FGameplayTag ParentTag, FGameplayTag NewTag)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return false;
	if (!ParentTag.IsValid() || !NewTag.IsValid()) return false;
	if (NewTag == ParentTag || !NewTag.MatchesTag(ParentTag)) return false;

	TArray<FGameplayTag> TagsToRemove;

	for (const auto& Tag : StatusTags.GetGameplayTagArray())
	{
		if (Tag != NewTag && Tag != ParentTag && Tag.MatchesTag(ParentTag))
		{
			TagsToRemove.Add(Tag);
		}
	}

	const bool bNeedsAdd = !StatusTags.HasTagExact(NewTag);

	if (TagsToRemove.IsEmpty() && !bNeedsAdd) return false;

	for (const auto& Tag : TagsToRemove)
	{
		StatusTags.RemoveTag(Tag);
	}

	if (bNeedsAdd)
	{
		StatusTags.AddTag(NewTag);
	}

	for (const auto& Tag : TagsToRemove)
	{
		NotifyTagChanged(Tag, false);
	}

	if (bNeedsAdd)
	{
		NotifyTagChanged(NewTag, true);
	}

	return true;
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

FGameplayTag UStatusComponent::GetExclusiveStatusTag(FGameplayTag ParentTag) const
{
	if (!ParentTag.IsValid()) return FGameplayTag();

	for (const auto& Tag : StatusTags.GetGameplayTagArray())
	{
		if (Tag != ParentTag && Tag.MatchesTag(ParentTag))
		{
			return Tag;
		}
	}

	return FGameplayTag();
}

FGameplayTagContainer UStatusComponent::GetStatusTags() const
{
	return StatusTags;
}
