// Copyright All Rights Reserved. Deven Shona / Space_Mex

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LocomotionComponent.generated.h"


UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class SURVIVALTEMPLATE_API ULocomotionComponent : public UActorComponent
{
private:
	GENERATED_BODY()
	
public:
	ULocomotionComponent();
};
