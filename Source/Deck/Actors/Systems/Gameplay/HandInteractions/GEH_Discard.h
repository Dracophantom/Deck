// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Systems/Gameplay/GE_HandInteraction.h"
#include "GEH_Discard.generated.h"

/**
 * 
 */
UCLASS()
class DECK_API AGEH_Discard : public AGE_HandInteraction
{
	GENERATED_BODY()

public:

	//Handle HandInteractions
	virtual void HandInteraction_Implementation(const TArray<int>& CardRefs) override;
};
