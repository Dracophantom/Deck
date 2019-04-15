// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Systems/GridEditor/TileModifier.h"
#include "Components/BoxComponent.h"
#include "TM_Height.generated.h"

/**
 * ATM_Height - Modifies the Height of the Tile
 */
UCLASS()
class DECK_API ATM_Height : public ATileModifier
{
	GENERATED_BODY()

public:
	ATM_Height();

	//Handle OnConstruction
	virtual void OnConstruction(const FTransform &Transform) override;

	//Override Destroyed to remove influence
	virtual void Destroyed() override;

	virtual void UpdateModifier() override;

	virtual void ClampToNeighbour(int Index) override;

	//Box to show area affected
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box")
		UBoxComponent* Box;

	//The Height this modifier adds
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Height", meta = (ClampMin = 0.0f))
		float Height = 0.0f;
};
