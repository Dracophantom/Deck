// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Systems/GridEditor/TileModifier.h"
#include "Components/BoxComponent.h"
#include "TM_Ramp.generated.h"


UENUM(BlueprintType)
enum class ETilt : uint8
{
	T_XOnly			UMETA(DisplayName = "X & -X"),
	T_YOnly			UMETA(DisplayName = "Y & -Y")
};
/**
 * 
 */
UCLASS()
class DECK_API ATM_Ramp : public ATileModifier
{
	GENERATED_BODY()

public:
	ATM_Ramp();

	virtual void OnConstruction(const FTransform &Transform) override;

	virtual void Destroyed() override;

	FRotator GetRotation();

	FVector GetScaleAdjustment(float TileSize);

	virtual void UpdateModifier() override;

	virtual void ClampToNeighbour(int Index) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box")
		UBoxComponent* Box;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ramp")
		ETilt RampDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ramp", meta = (ClampMin = 0.0f))
		float MinHeight = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ramp")
		float MidHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ramp")
		float MaxHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ramp", meta = (ClampMin = -45.0f, ClampMax = 45.0f))
		float Angle = 0.0f;
};
