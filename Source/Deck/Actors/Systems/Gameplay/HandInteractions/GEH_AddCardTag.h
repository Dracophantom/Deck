// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Systems/Gameplay/GE_HandInteraction.h"
#include "GameplayTags.h"
#include "GEH_AddCardTag.generated.h"

/**
 * 
 */
UCLASS()
class DECK_API AGEH_AddCardTag : public AGE_HandInteraction
{
	GENERATED_BODY()
	
public:
	//Handle HandInteractions
	virtual void HandInteraction_Implementation(const TArray<int>& CardRefs) override;

	//The Tag that the effect will add
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Hand Interaction", meta = (ExposeOnSpawn = true))
		FGameplayTag NewTag;
};
