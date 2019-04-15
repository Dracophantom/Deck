// Fill out your copyright notice in the Description page of Project Settings.

#include "GEH_AddCardTag.h"
#include "Engine.h"
#include "Actors/Player/DeckPC.h"


void AGEH_AddCardTag::HandInteraction_Implementation(const TArray<int>& CardRefs)
{
	Super::HandInteraction_Implementation(CardRefs);

	//Add a tag to the selected cards on a valid PC's current character
	if (ADeckPC* PC = Cast<ADeckPC>(GetWorld()->GetFirstPlayerController()))
	{
		PC->CurrentCharacter->AddCardTags(CardRefs, NewTag);
	}
}