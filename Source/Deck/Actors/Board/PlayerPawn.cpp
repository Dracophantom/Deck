// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerPawn.h"
#include "Actors/Player/DeckPC.h"
#include "DeckGM.h"
#include "Engine.h"

//void APlayerPawn::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	//Set the Hand size to 7 and Draw Cards
	Hand.SetNum(7);
	DrawCards();
}

//Handle the Pawn's start of turn
void APlayerPawn::StartTurn()
{
	//Parent call
	Super::StartTurn();

	//If there is a ADeckPC
	if (ADeckPC* PC = Cast<ADeckPC>(GetWorld()->GetFirstPlayerController()))
	{
		//Set this as the current character
		PC->CurrentCharacter = this;

		//Draw Cards
		DrawCards();

		//Add the Control UI
		AddControlUI();

		//Set Available Actions and Bonus Actions
		AvailableActions = 1;
		AvailableBonusActions = 1;

		//Call the PC Start Turn
		PC->StartTurn();
	}
}

//Handle the Pawn's end of turn
void APlayerPawn::EndTurn()
{
	//Remove the UI
	RemoveUI();

	Super::EndTurn();
}

//Handle when the Pawn's Movement is Finished
void APlayerPawn::FinishMovement()
{
	Super::FinishMovement();

	//Get the PC and call FinishMove
	if (ADeckPC* PC = Cast<ADeckPC>(GetWorld()->GetFirstPlayerController()))
	{
		PC->FinishMove();
	}
}

//Draw new Cards
void APlayerPawn::DrawCards()
{
	//If the Deck size is 0, abort
	if (Deck.Num() == 0)
		return;

	//Loop through all Cards
	for (int i = 0; i < Hand.Num(); i++)
	{
		//If the current card is invaid
		if (!Hand[i])
		{
			//Set DeckRef to -1
			int DeckRef = -1;

			//Loop until a valid card is found *Add check for empty deck
			do
			{
				//Random number
				DeckRef = FMath::RandRange(0, Deck.Num() - 1);

				//Check if there are cards of that type to draw
				if (Deck[DeckRef].InDeck < 1)
				{
					//Otherwise set DeckRef to -1
					DeckRef = -1;
				}

			} while (DeckRef == -1);

			//Create a new card and add it
			UDeckCard* DrawnCard = Cast<UDeckCard>(NewObject<UDeckCard>(this, Deck[DeckRef].Card->GetDefaultObject()->GetClass()));
			Hand[i] = DrawnCard;
			//Remove from the Deck
			--Deck[DeckRef].InDeck;
		}
	}
}

//Handle when the Pawn finishes an Action
void APlayerPawn::FinishAction()
{
	//Call the PC ActionFinished
	if (ADeckPC* PC = Cast<ADeckPC>(GetWorld()->GetFirstPlayerController()))
	{
		PC->ActionFinished();
	}
}

//Add an Available Action
void APlayerPawn::AddAction()
{
	AvailableActions++;
}

//Remove an Available Action
void APlayerPawn::RemoveAction()
{
	AvailableActions--;
}

//Add an Available Bonus Action
void APlayerPawn::AddBonusAction()
{
	AvailableBonusActions++;
}

//Remove an Available Bonus Action
void APlayerPawn::RemoveBonusAction()
{
	AvailableBonusActions--;
}

//Discard specified Cards
void APlayerPawn::DiscardCards(TArray<int> CardRefs)
{
	//Loop through all the CardRefs
	for (int i = 0; i < CardRefs.Num(); i++)
	{
		//If the Ref is Valid
		if (CardRefs[i] > -1 && CardRefs[i] < Hand.Num())
		{
			if (Hand[CardRefs[i]])
			{
				//Some way to trigger removing
				Hand[i] = nullptr;
			}
		}
	}
}

//Add a a tag to selected Cards
void APlayerPawn::AddCardTags(TArray<int> CardRefs, FGameplayTag TargetTag)
{
	//Loop through all Card Refs
	for (int i = 0; i < CardRefs.Num(); i++)
	{
		//Check that the Card is valid
		if (CardRefs[i] > -1 && CardRefs[i] < Hand.Num())
		{
			if (Hand[CardRefs[i]])
			{
				//Add the Tag
				Hand[CardRefs[i]]->GameplayTags.AddTag(TargetTag);
			}
		}
	}
}

//Discard a Card after use
void APlayerPawn::DiscardUsedCard(int CardRef)
{
	//Check that the Card is valid
	if (CardRef > -1 && CardRef < Hand.Num())
	{
		if (Hand[CardRef])
		{
			//Nullptr to 'discard'
			Hand[CardRef] = nullptr;
		}
	}
}