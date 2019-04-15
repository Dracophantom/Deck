// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Systems/Gameplay/GameplayEffect.h"
#include "GE_CardBase.generated.h"

/**
 * Gameplay Effect specific to Cards
 */
UCLASS()
class DECK_API AGE_CardBase : public AGameplayEffect
{
	GENERATED_BODY()

public:
	//Override for StartEffect Implementation
	virtual void StartEffect_Implementation() override;

	//Override for FinishEffect Implementation
	virtual void FinishEffect_Implementation() override;

	//Tiles for the Effect to Check
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay Effect", meta = (ExposeOnSpawn = true))
		TArray<int> TilesToCheck;

	//Whether the Effect is currently Active
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Effect")
		bool bIsActive = false;
};
