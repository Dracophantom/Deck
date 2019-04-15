// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Systems/Gameplay/GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "GE_StatusBase.generated.h"

/**
 * Gameplay Effect specific to Status Effects
 */
UCLASS()
class DECK_API AGE_StatusBase : public AGameplayEffect, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	
public:
	//Override for StartEffect Implementation
	virtual void StartEffect_Implementation();

	//Handle Updating the Effect per turn
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void UpdateEffect();
	virtual void UpdateEffect_Implementation();

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = GameplayTags; return; }

	//The Targetted Pawn for the Effect
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status Effect", meta = (ExposeOnSpawn = true))
		class ADeckPawnBase* TargettedPawn;

	//The Gameplay Tags
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect")
		FGameplayTagContainer GameplayTags;

	//0 for one time use, -1 for infinite use
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status Effect")
		int TurnsRemaining;
};
