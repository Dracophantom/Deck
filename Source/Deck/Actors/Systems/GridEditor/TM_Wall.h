// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Systems/GridEditor/TileModifier.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "TM_Wall.generated.h"

UENUM(BlueprintType, meta = (Bitflags))
enum class EDirectionFlags : uint8
{
	D_PosX	UMETA(DisplayName = "+X"),
	D_PosY	UMETA(DisplayName = "+Y"),
	D_NegX	UMETA(DisplayName = "-X"),
	D_NegY	UMETA(DisplayName = "-Y")
};

/**
 * 
 */
UCLASS()
class DECK_API ATM_Wall : public ATileModifier
{
	GENERATED_BODY()

public:
	ATM_Wall();

	virtual void OnConstruction(const FTransform &Transform) override;

	void InstantiateWalls(float TileSize);

	UFUNCTION(BlueprintCallable)
	void ToggleWall(EDirectionFlags Direction);

	virtual void UpdateModifier() override;

	virtual void ClampToNeighbour(int Index) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		UInstancedStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Direction", meta = (Bitmask, BitmaskEnum = "EDirectionFlags"))
		int32 DirectionFlags;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Wall", meta = (ClampMin = 1.0f))
		float Thickness = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Wall", meta = (ClampMin = 1.0f))
		float Height = 1.0f;
};
