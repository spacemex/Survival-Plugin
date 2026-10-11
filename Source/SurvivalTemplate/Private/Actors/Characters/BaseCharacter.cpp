// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Characters/BaseCharacter.h"


ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);
	
	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
}
