// Fill out your copyright notice in the Description page of Project Settings.

#include "GEH_Discard.h"
#include "Engine.h"
#include "Actors/Player/DeckPC.h"


//Handle HandInteractions
void AGEH_Discard::HandInteraction_Implementation(const TArray<int>& CardRefs)
{
	Super::HandInteraction_Implementation(CardRefs);

	//Discard the selected cards on a valid PC's current character
	if (ADeckPC* PC = Cast<ADeckPC>(GetWorld()->GetFirstPlayerController()))
	{
		PC->CurrentCharacter->DiscardCards(CardRefs);
	}
}