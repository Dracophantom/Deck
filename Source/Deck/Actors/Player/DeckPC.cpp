// Fill out your copyright notice in the Description page of Project Settings.

#include "DeckPC.h"
#include "Actors/Systems/Grid/GridManager.h"
#include "DeckGM.h"

void ADeckPC::SetupInputComponent()
{
	Super::SetupInputComponent();
	//Bind actions
	InputComponent->BindAction("Select", IE_Pressed, this, &ADeckPC::SelectTile);
	InputComponent->BindAction("Backstep", IE_Pressed, this, &ADeckPC::StartBackStep).bConsumeInput = false;
	InputComponent->BindAction("Backstep", IE_Released, this, &ADeckPC::Backstep).bConsumeInput = false;
	//InputComponent->BindAction("Inspect", IE_Pressed, this, &ADeckPC::SelectTile);
}

//Select a Tile
void ADeckPC::SelectTile()
{
	//If CurrentCharacter is Invalid, abort
	if (!CurrentCharacter)
		return;

	//Deproject the Mouse
	FVector MousePos;
	FVector MouseDir;

	this->DeprojectMousePositionToWorld(MousePos, MouseDir);

	FHitResult Hit;

	//Ray trace from screen
	if (GetWorld()->LineTraceSingleByChannel(Hit, MousePos, MousePos + MouseDir * 9000.0f, ECollisionChannel::ECC_Visibility))
	{
		//If a Grid is hit, continue
		AGridManager* Grid = Cast<AGridManager>(Hit.GetActor());
		if (Grid)
		{
			//Select based on CurrentState
			switch (CurrentState)
			{
			case ECurrentState::CS_Inspection:
				//Inspection does nothing right now
				break;
			case ECurrentState::CS_Movement:
			{
				//If Movement, find a Path to the hit Node
				TArray<int> Path = Grid->FindPath(CurrentCharacter->GetActorLocation(), Hit.Location, CurrentCharacter->Stats->VerticalMovement);

				//If Movement can start, clear all Highlights
				if (CurrentCharacter->StartMovement(Path))
					Grid->ClearHighlights();
			}
			break;
			case ECurrentState::CS_Action:
			{
				//If Action, find if the Tile is targetable
				if (Grid->IsTileTargetable(Hit.Location))
				{
					//If it is, clear highlights
					Grid->ClearHighlights();

					//Detects Tiles to Hit
					TilesToHit = Grid->DetectAttackTiles(CurrentCharacter->GetActorLocation(), Hit.Location, CurrentCharacter->Hand[CurrentCardRef]->HitPattern, CurrentCharacter->Hand[CurrentCardRef]->HitMin, CurrentCharacter->Hand[CurrentCardRef]->HitMax, CurrentCharacter->Hand[CurrentCardRef]->bFromCaster);

					//Proceed to Confirm Action
					ConfirmAction();
				}
			}
			break;
			case ECurrentState::CS_Action_C:
			{
				//If Action_C(onfirm), check if Tile is targetable
				if (Grid->IsTileTargetable(Hit.Location))
				{
					//If it is, get the Game Mode
					if (ADeckGM* GM = Cast<ADeckGM>(GetWorld()->GetAuthGameMode()))
					{
						//Create the Gameplay Effects from the selected Card
						for (int i = 0; i < CurrentCharacter->Hand[CurrentCardRef]->Effects.Num(); i++)
						{
							AGE_CardBase* Effect = GetWorld()->SpawnActor<AGE_CardBase>(CurrentCharacter->Hand[CurrentCardRef]->Effects[i]->GetDefaultObject()->GetClass());
							Effect->TilesToCheck = TilesToHit;
							Effect->SetProperties(CurrentCharacter->Hand[CurrentCardRef]->CardProperties);
							GM->AddNewEffect(Effect);
						}

						//Play an Animation if there is one
						if (CurrentCharacter->Hand[CurrentCardRef]->Animation)
						{
							CurrentCharacter->Mesh->GetAnimInstance()->Montage_Play(CurrentCharacter->Hand[CurrentCardRef]->Animation);
						}

						CurrentState = ECurrentState::CS_Pending;

						//Clear Highlights
						Grid->ClearHighlights();
						//Start Effects
						GM->StartNewEffect();
					}
				}
			}
			break;
			}
		}
	}
}

void ADeckPC::StartBackStep()
{
	GetWorld()->GetTimerManager().SetTimer(BackstepTimer, this, &ADeckPC::ClearBackstep, InputDelay, false);
}

void ADeckPC::ClearBackstep()
{
	GetWorld()->GetTimerManager().ClearTimer(BackstepTimer);
}

//Go back a step in the Menu
void ADeckPC::Backstep()
{
	//Confirm Game Mode is valid
	ADeckGM* GM = Cast<ADeckGM>(GetWorld()->GetAuthGameMode());

	if (!GM || !GetWorld()->GetTimerManager().IsTimerActive(BackstepTimer))
		return;

	GetWorld()->GetTimerManager().ClearTimer(BackstepTimer);

	//Select based on CurrentState
	///***POTENTIAL BUG*** - If waiting on Effects to finish, may be able to circumvent
	switch (CurrentState)
	{
	case ECurrentState::CS_Inspection:
		//Nothing for Inspection
		break;
	case ECurrentState::CS_Movement:
	{
		//If Movement, clear Highlights and return to Inspection
		CurrentState = ECurrentState::CS_Inspection;
		GM->GridRef->ClearHighlights();
	}
	break;
	case ECurrentState::CS_Action:
	{
		//If Action, clear Highlights and return to Inspection
		CurrentState = ECurrentState::CS_Inspection;
		GM->GridRef->ClearHighlights();
		//Clear CurrentCardRef
		CurrentCardRef = -1;
	}
	break;
	case ECurrentState::CS_Action_C:
	{
		//If Action_C, clear Highlights
		GM->GridRef->ClearHighlights();

		//Redo the Action for the CurrentCardRef
		StartAction(CurrentCardRef);

		//Return to Action
		CurrentState = ECurrentState::CS_Action;
	}
	break;
	case ECurrentState::CS_Discard:
	{
		//If Discard, return to Inspection
		///***POTENTIAL BUG*** - If Discard is invoked by an Effect
		//CurrentState = ECurrentState::CS_Inspection;
	}
	break;
	}
}

//Start the Player's turn
void ADeckPC::StartTurn()
{
	CurrentState = ECurrentState::CS_Inspection;
}

//End the Player's turn
void ADeckPC::EndTurn()
{
	CurrentCharacter->EndTurn();
}

//Start Movement step
void ADeckPC::StartMove()
{
	//Must be in Inspection
	if (CurrentState == ECurrentState::CS_Inspection)
	{
		//Set Current State
		CurrentState = ECurrentState::CS_Movement;

		//Highlight Movement Tiles
		if (ADeckGM* GM = Cast<ADeckGM>(GetWorld()->GetAuthGameMode()))
		{
			GM->GridRef->HighlightMovement(CurrentCharacter->GetActorLocation(), CurrentCharacter->Stats->AvailableMovement, CurrentCharacter->Stats->VerticalMovement);
		}
	}
}

//Finish the Movement step
void ADeckPC::FinishMove()
{
	//Set CurrentState from Movement to Inspection
	if (CurrentState == ECurrentState::CS_Movement)
	{
		CurrentState = ECurrentState::CS_Inspection;
	}
}

//Start an Action
void ADeckPC::StartAction(int ActionIndex)
{
	//If CurrentState is Inspection
	if (CurrentState == ECurrentState::CS_Inspection || CurrentState == ECurrentState::CS_Action_C)
	{
		//If GM is Valid
		ADeckGM* GM = Cast<ADeckGM>(GetWorld()->GetAuthGameMode());
		if (!GM)
			return;

		//Update CurrentState
		CurrentState = ECurrentState::CS_Action;

		//Save CurrentCardRef
		CurrentCardRef = ActionIndex;

		//Highlight Attack Placement
		GM->GridRef->HighlightAttackPlacement(CurrentCharacter->GetActorLocation(), CurrentCharacter->Hand[CurrentCardRef]->HitPattern, CurrentCharacter->Hand[CurrentCardRef]->PlacementMin, CurrentCharacter->Hand[CurrentCardRef]->PlacementMax);
	}
}

//Confirm an Action
void ADeckPC::ConfirmAction()
{
	//Set CurrentState from Action to Action_C
	if (CurrentState == ECurrentState::CS_Action)
	{
		CurrentState = ECurrentState::CS_Action_C;
	}
}

//Current Action is Finished
void ADeckPC::ActionFinished()
{
	//Set CurrentState from Action_C to Inspection
	if (CurrentState == ECurrentState::CS_Pending)
	{
		CurrentState = ECurrentState::CS_Inspection;

		//Consume appropriate Input
		switch (CurrentCharacter->Hand[CurrentCardRef]->ConsumedInput)
		{
		case ESlotType::ST_Action:
			CurrentCharacter->RemoveAction();
			break;
		case ESlotType::ST_Bonus:
			CurrentCharacter->RemoveBonusAction();
			break;
		}

		//Remove used Card
		CurrentCharacter->DiscardUsedCard(CurrentCardRef);

		//Clear CurrentCardRef
		CurrentCardRef = -1;
	}
}

//Start selecting cards to Discard
void ADeckPC::DiscardSelection(FGameplayTagContainer TargetTags, int Min, int Max)
{ 
	//Set Min and Max required, Max can not be less than Min and Min can not be less than 0
	MinCardsRequired = FMath::Max(0, Min);
	MaxCardsRequired = FMath::Max(Min, Max);

	//Set Tags for cards that can be picked
	CardTags = TargetTags;

	//Enter Discard state
	CurrentState = ECurrentState::CS_Discard;
}

//Start selecting cards
void ADeckPC::CardSelection(FGameplayTagContainer TargetTags, int Min, int Max)
{
	//Set Min and Max required, Max can not be less than Min and Min can not be less than 0
	MinCardsRequired = FMath::Max(0, Min);
	MaxCardsRequired = FMath::Max(Min, Max);

	//Set Tags for cards that can be picked
	CardTags = TargetTags;

	//Enter SelectCards state
	CurrentState = ECurrentState::CS_SelectCards;
}

//Add a Selected card to hand interactions
void ADeckPC::AddSelectedCard(int CardRef)
{
	//Confirm Card is valid, then add
	if (!SelectCardRefs.Contains(CardRef) && CurrentCardRef != CardRef)
	{
		SelectCardRefs.Add(CardRef);
	}
}

//Remove a Selected card from hand interactions
void ADeckPC::RemoveSelectedCard(int CardRef)
{
	//Confirm Card is valid, then remove
	if (SelectCardRefs.Contains(CardRef) && CurrentCardRef != CardRef)
	{
		SelectCardRefs.Remove(CardRef);
	}
}

//Finish current hand interaction
void ADeckPC::FinishCardInteraction()
{
	//Must be in Discard or SelectCards
	if (CurrentState == ECurrentState::CS_Discard || CurrentState == ECurrentState::CS_SelectCards)
	{
		//If CurrentCardRef is not -1; it was part of a Card's effect
		if (CurrentCardRef != -1)
		{
			//Return to Action_C until all Card effects finish
			CurrentState = ECurrentState::CS_Action_C;
		}
		//Otherwise it was evoked by an outside source
		else
		{
			//Return to Inspection
			CurrentState = ECurrentState::CS_Inspection;
		}

		//Clear the Selected Cards
		SelectCardRefs.Empty();
	}
}