// Fill out your copyright notice in the Description page of Project Settings.

#include "DeckCard.h"

//Set the type of Input the Card will consume
void UDeckCard::SetConsumedInput(ESlotType Input)
{
	ConsumedInput = Input;
}