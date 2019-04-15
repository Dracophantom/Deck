// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GridWall.generated.h"

UCLASS()
class DECK_API AGridWall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridWall();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent)
		void CreateWalls(float TileSize, float Height, float Thickness, uint8 Directions);
	virtual void CreateWalls_Implementation(float TileSize, float Height, float Thickness, uint8 Directions);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		UInstancedStaticMeshComponent* Mesh;
};
