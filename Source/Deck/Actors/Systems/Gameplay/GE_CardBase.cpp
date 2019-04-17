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

//Check for an Actor at specified tile
AActor* AGE_CardBase::CheckTile(int Tile)
{
	//Get a Valid GM to check for a Grid
	if (ADeckGM* GM = Cast<ADeckGM>(GetWorld()->GetAuthGameMode()))
	{
		if (GM->GridRef)
		{
			FVector Start = GM->GridRef->GetLocationFromIndex(Tile) + FVector(0.0f, 0.0f, 500.0f);
			FVector End = Start - FVector(0.0f, 0.0f, 1000.0f);

			FHitResult Hit;

			if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Pawn))
			{
				return Hit.GetActor();
			}
		}
	}

	return NULL;
}