// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPawn.h"
#include "Engine.h"


void AEnemyPawn::StartTurn()
{
	//Parent call
	Super::StartTurn();

	AddUI();

	GetWorld()->GetTimerManager().SetTimer(PassTimer, this, &AEnemyPawn::EndTurn, 5.0f, false);
}

void AEnemyPawn::EndTurn()
{
	RemoveUI();

	GetWorld()->GetTimerManager().ClearTimer(PassTimer);

	Super::EndTurn();
}