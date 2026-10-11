// Copyright All Rights Reserved. Deven Shona / Space_Mex

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "StatusComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatusTagChanged, FGameplayTag, Tag, bool, bAdded);

UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class SURVIVALTEMPLATE_API UStatusComponent : public UActorComponent
{
private:
	GENERATED_BODY()

	UPROPERTY(ReplicatedUsing=OnRep_StatusTags)
	FGameplayTagContainer StatusTags;

	UFUNCTION()
	void OnRep_StatusTags(const FGameplayTagContainer& PreviousTags);

	void NotifyTagChanged(FGameplayTag Tag, bool bAdded);

public:
	UStatusComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category="Status")
	FOnStatusTagChanged OnStatusTagChanged;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Status")
	bool SetExclusiveStatusTag(FGameplayTag ParentTag, FGameplayTag NewTag);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Status")
	bool AddStatusTag(const FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Status")
	bool RemoveStatusTag(const FGameplayTag Tag);

	UFUNCTION(BlueprintPure, Category="Status")
	bool HasStatusTag(const FGameplayTag Tag) const;

	UFUNCTION(BlueprintPure, Category="Status")
	FGameplayTag GetExclusiveStatusTag(FGameplayTag ParentTag) const;

	UFUNCTION(BlueprintPure, Category="Status")
	FGameplayTagContainer GetStatusTags() const;
};
