// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Systems/Gameplay/GameplayEffect.h"
#include "GE_HandInteraction.generated.h"

/**
 * 
 */
UCLASS()
class DECK_API AGE_HandInteraction : public AGameplayEffect
{
	GENERATED_BODY()
	
public:
	//Handle HandInteractions
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void HandInteraction(const TArray<int>& CardRefs);
		virtual void HandInteraction_Implementation(const TArray<int>& CardRefs);

	//Finish the Gameplay Effect
	virtual void FinishEffect_Implementation() override;

	//Source for the Interaction
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Hand Interaction")
		AGameplayEffect* Source;
	/*
	Discard
	- Because of Effect
	- Because of End of Turn

	Select Card
	- Add Additional Tag
	*/
};
