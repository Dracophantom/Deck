// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Actors/Player/DeckCard.h"
#include "Actors/Board/DeckPawnBase.h"
#include "Actors/Systems/Grid/GridWall.h"
#include "GridManager.generated.h"

//Node Connection
//- Keeps Track of an Index and the Score tied to that Index
USTRUCT(BlueprintType)
struct FNodeConnection
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
		int Index;

	UPROPERTY(VisibleAnywhere)
		int Score = 1;
};

//Graph Node
// - Keeps track of important data for each Tile
USTRUCT(BlueprintType)
struct FGraphNode
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
		FVector Position = FVector(0.0f);

	UPROPERTY(VisibleAnywhere)
		TArray<FNodeConnection> Connections;

	UPROPERTY(VisibleAnywhere)
		bool bIsTargetable = false;

	UPROPERTY(VisibleAnywhere)
		bool bIsOccupied = false;
};

//Used for Pathfinding
struct SearchNode
{
	int gScore;
	int fScore;
	int NodeCameFrom = -1;
};

UCLASS()
class DECK_API AGridManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Import a board from a file
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Development")
		void ImportBoard();

	/*****************
	*Location Finding*
	*****************/
	//Find the Index closest to the specified Location, returns -1 on fail
	UFUNCTION(BlueprintCallable)
		int GetIndexFromLocation(FVector Location);

	//Find the Location of the Tile at Index
	UFUNCTION(BlueprintCallable)
		FVector GetLocationFromIndex(int Index);

	//Get the closest Index Location to the specified Location
	UFUNCTION(BlueprintCallable)
		FVector GetClosestTileLocation(FVector Location);

	/*************
	*Path Finding*
	*************/

	//Heuristic Estimate for pathfinding
	int HeuristicEstimate(int StartIndex, int Goal);

	//Using the map, reconstruct the best path
	TArray<int> ReconstructPath(TArray<SearchNode> Map, int Current);

	//Find a path between the two locations using a Height tolerance
	UFUNCTION(BlueprintCallable)
		TArray<int> FindPath(FVector Start, FVector End, float MaxHeight = 500.0f);

	/****************
	*Highlight Tiles*
	****************/

	//Highlight the Tiles that could be reached through Movement within the MovementRange and Height tolerance
	UFUNCTION(BlueprintCallable)
		void HighlightMovement(FVector Start, int MovementRange, float MaxHeight = 500.0f);

	//Highlight the Tiles that can be reached by the specified Attack Pattern
	//To Do - Add Height Tolerance
	UFUNCTION(BlueprintCallable)
		void HighlightAttackPlacement(FVector Start, EPatternType Pattern, int Min, int Max);

	//Detect the Tiles that will be reached from the specified Attack Person
	//To Do - Add Height Tolerance
	UFUNCTION(BlueprintCallable)
		TArray<int> DetectAttackTiles(FVector CasterPos, FVector Location, EPatternType Pattern, int Min, int Max, bool FromCaster = false);

	//Clear the Highlight instances
	UFUNCTION(BlueprintCallable)
		void ClearHighlights();

	//Check if the Tile at Location is Targetable
	UFUNCTION(BlueprintCallable)
		bool IsTileTargetable(FVector Location);

	//Try to add a Highlight at the Index, return true if successful
	bool HighlightTile(int Index);

	//Instanced Static Mesh for the Grid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		UInstancedStaticMeshComponent* GridMesh;

	//Instanced Static Mesh for Highlights
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		UInstancedStaticMeshComponent* TileHighlights;

	//The X-size of the Grid
	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	int SizeX = 0;

	//The Y-size of the Grid
	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	int SizeY = 0;

	//The Tile Size
	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	float TileSize = 1.0f;

	//The Array of Grid tiles
	UPROPERTY()
	TArray<FGraphNode> Grid;

	//File Path to Read/Write to (uses Content directory by default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Development")
		FString FilePath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Development")
		TSubclassOf<AGridWall> WallType;
};
