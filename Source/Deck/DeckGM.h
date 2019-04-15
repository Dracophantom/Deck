// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Actors/Board/DeckPawnBase.h"
#include "Actors/Systems/Gameplay/GE_CardBase.h"
#include "Actors/Systems/Grid/GridManager.h"
#include "DeckGM.generated.h"

/**
 * 
 */
UCLASS()
class DECK_API ADeckGM : public AGameModeBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		void StartCombat();

	UFUNCTION(BlueprintCallable)
		void EndCombat();

	UFUNCTION(BlueprintCallable)
		void NextCharacter();

	UFUNCTION(BlueprintCallable)
		void AddNewCharacter(ADeckPawnBase* NewCharacter);
	
	UFUNCTION(BlueprintCallable)
		void RemoveCharacter(ADeckPawnBase* RemovedCharacter);

	UFUNCTION(BlueprintCallable)
		void AddNewEffect(AGE_CardBase* NewEffect);

	UFUNCTION(BlueprintCallable)
		void StartNewEffect(int EffectIndex = 0);

	UFUNCTION(BlueprintCallable)
		void RemoveEffect(AGE_CardBase* EffectToRemove);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		ADeckPawnBase* GetCurrentCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TArray<ADeckPawnBase*> Characters;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
		int ActiveCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TArray<AGE_CardBase*> ActiveEffects;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
		AGridManager* GridRef;
};
