// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Board/DeckPawnBase.h"
#include "EnemyPawn.generated.h"

/**
 * 
 */
UCLASS()
class DECK_API AEnemyPawn : public ADeckPawnBase
{
	GENERATED_BODY()
	
public:

	//Handle the Pawn's start of turn
	virtual void StartTurn() override;
	//Handle the Pawn's start of turn
	virtual void EndTurn() override;

	//Add ControlUI in Blueprint
	UFUNCTION(BlueprintImplementableEvent)
		void AddUI();

	//RemoveUI in Blueprint
	UFUNCTION(BlueprintImplementableEvent)
		void RemoveUI();

	FTimerHandle PassTimer;
};
