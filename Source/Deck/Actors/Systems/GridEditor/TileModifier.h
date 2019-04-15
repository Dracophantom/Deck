// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileModifier.generated.h"

UCLASS()
class DECK_API ATileModifier : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATileModifier();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void UpdateModifier() {}

	UFUNCTION(BlueprintCallable)
		virtual void ClampToNeighbour(int Index) {}

	//The Score of the Modifier
	UPROPERTY(BlueprintReadWrite, Category = "Path Finding")
	int Score;

	//The Modifier's current index
	UPROPERTY(BlueprintReadOnly, Category = "Tile")
	int CurrentIndex;
};
