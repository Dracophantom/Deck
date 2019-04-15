// Fill out your copyright notice in the Description page of Project Settings.

#include "GE_StatusBase.h"

void AGE_StatusBase::StartEffect_Implementation()
{
	Super::StartEffect_Implementation();

	//If TurnsRemaining = 0, Finish
	if (TurnsRemaining == 0)
		FinishEffect();
}

void AGE_StatusBase::UpdateEffect_Implementation()
{
	//If TurnsRemaining > 0, tick it down
	if (TurnsRemaining > 0)
	{
		TurnsRemaining--;

		//If TurnsRemaining = 0, Finish
		if (TurnsRemaining == 0)
		{
			FinishEffect();
		}
	}
}