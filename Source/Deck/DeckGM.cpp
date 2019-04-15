// Fill out your copyright notice in the Description page of Project Settings.

#include "DeckGM.h"
#include "Actors/Player/CameraPawn.h"
#include "Engine.h"

void ADeckGM::StartCombat()
{
	if (Characters.Num() > 0)
	{
		ActiveCharacter = 0;
		Characters[ActiveCharacter]->StartTurn();

		if (ACameraPawn* Pawn = Cast<ACameraPawn>(GetWorld()->GetFirstPlayerController()->GetPawn()))
		{
			Pawn->SetNewFocus(Characters[ActiveCharacter]);
		}
	}
}

void ADeckGM::EndCombat()
{

}

void ADeckGM::NextCharacter()
{
	ActiveCharacter = (ActiveCharacter + 1 < Characters.Num() ? ActiveCharacter++ : 0);

	if (Characters[ActiveCharacter])
	{
		Characters[ActiveCharacter]->StartTurn();

		if (ACameraPawn* Pawn = Cast<ACameraPawn>(GetWorld()->GetFirstPlayerController()->GetPawn()))
		{
			Pawn->SetNewFocus(Characters[ActiveCharacter]);
		}
	}
}

void ADeckGM::AddNewCharacter(ADeckPawnBase* NewCharacter)
{
	if (!Characters.Contains(NewCharacter))
	{
		int Speed = NewCharacter->Stats->Speed;

		for (int i = 0; i < Characters.Num(); i++)
		{
			if (Characters[i])
			{
				if (Characters[i]->Stats->Speed < Speed)
				{
					Characters.Insert(NewCharacter, i);
					return;
				}
			}
		}

		Characters.Add(NewCharacter);
	}
}

void ADeckGM::RemoveCharacter(ADeckPawnBase* RemovedCharacter)
{
	if (Characters.Contains(RemovedCharacter))
	{
		Characters.Remove(RemovedCharacter);
	}
}

void ADeckGM::AddNewEffect(AGE_CardBase* NewEffect)
{
	ActiveEffects.Add(NewEffect);
}

void ADeckGM::StartNewEffect(int EffectIndex)
{
	if (EffectIndex < ActiveEffects.Num() && EffectIndex > -1)
	{
		if (!ActiveEffects[EffectIndex]->bIsActive)
			ActiveEffects[EffectIndex]->StartEffect();
	}
}

void ADeckGM::RemoveEffect(AGE_CardBase* EffectToRemove)
{
	ActiveEffects.Remove(EffectToRemove);

	if (ActiveEffects.Num() == 0)
	{
		Characters[ActiveCharacter]->FinishAction();
	}
	else
	{
		StartNewEffect(0);
	}
}

ADeckPawnBase* ADeckGM::GetCurrentCharacter()
{
	if (ActiveCharacter > -1 && ActiveCharacter < Characters.Num())
	{
		return Characters[ActiveCharacter];
	}
	
	return nullptr;
}