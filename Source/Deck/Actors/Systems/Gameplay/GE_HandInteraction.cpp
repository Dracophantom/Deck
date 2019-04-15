// Fill out your copyright notice in the Description page of Project Settings.

#include "GE_HandInteraction.h"
#include "GE_CardBase.h"
#include "GE_StatusBase.h"
#include "Actors/Player/DeckPC.h"
#include "Engine.h"

//Handle HandInteractions
void AGE_HandInteraction::HandInteraction_Implementation(const TArray<int>& CardRefs)
{
}

//Finish the Gameplay Effect
void AGE_HandInteraction::FinishEffect_Implementation()
{
	//Get a valid PC
	if (ADeckPC* PC = Cast<ADeckPC>(GetWorld()->GetFirstPlayerController()))
	{
		//Call FinishCardInteraction
		PC->FinishCardInteraction();
	}

	//If there is a Source
	if (Source)
	{
		//Is the Source a Card Effect?
		if (Cast<AGE_CardBase>(Source))
		{
			//Finish the Effect
			Source->FinishEffect();
		}
		//Is the Source a Status Effect?
		else if (AGE_StatusBase* Status = Cast<AGE_StatusBase>(Source))
		{
			//Update it
			Status->UpdateEffect();
		}
	}
	else
	{
		//Assume it was the PC's End-of-Turn Discard
		if (ADeckPC* PC = Cast<ADeckPC>(GetWorld()->GetFirstPlayerController()))
		{
			PC->EndTurn();
		}
	}

	Super::FinishEffect_Implementation();
}