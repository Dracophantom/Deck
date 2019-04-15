// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Systems/GridEditor/TileModifier.h"
#include "Actors/Systems/GridEditor/GridManager_Editor.h"
#include "Components/BoxComponent.h"
#include "TM_Null.generated.h"

/**
 * 
 */
UCLASS()
class DECK_API ATM_Null : public ATileModifier
{
	GENERATED_BODY()

public:
	ATM_Null();

	//Handle OnConstruction
	virtual void OnConstruction(const FTransform &Transform) override;

	//Override Destroyed to remove influence
	virtual void Destroyed() override;

	//Box to show area affected
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box")
		UBoxComponent* Box;

	//Grid Reference
	AGridManager_Editor* GridRef;
};
