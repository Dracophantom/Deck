// Fill out your copyright notice in the Description page of Project Settings.

#include "GE_CardBase.h"
#include "DeckGM.h"
#include "Engine.h"

//Override for StartEffect Implementation
void AGE_CardBase::StartEffect_Implementation()
{
	Super::StartEffect_Implementation();

	//Set Active to True
	bIsActive = true;
}

//Override for FinishEffect Implementation
void AGE_CardBase::FinishEffect_Implementation()
{
	//Get a Valid GM and try to remove the effect
	if (ADeckGM* GM = Cast<ADeckGM>(GetWorld()->GetAuthGameMode()))
	{
		GM->RemoveEffect(this);
	}

	//Call Parent FinishEffect
	Super::FinishEffect_Implementation();
}