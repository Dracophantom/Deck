// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Actors/Player/DeckCard.h"
#include "Actors/Board/DeckPawnBase.h"
#include "Actors/Systems/Grid/GridManager.h"
#include "GridManager_Editor.generated.h"

//Graph Nodes specifically for the Editor
//- Stores additional information to help with constant rebuilding
USTRUCT(BlueprintType)
struct FGraphNodeEditor
{
	GENERATED_BODY()

	//The Position of the Node
	UPROPERTY(VisibleAnywhere)
		FVector Position = FVector(0.0f);

	//The Rotation of the Node
	UPROPERTY(VisibleAnywhere)
		FRotator Rotation = FRotator(0.0f);

	//The Scale of the Node
	UPROPERTY(VisibleAnywhere)
		FVector Scale = FVector(1.0f);

	//Array of Connection data
	UPROPERTY(VisibleAnywhere)
		TArray<FNodeConnection> Connections;

	//Is the Tile Targetable?
	UPROPERTY(VisibleAnywhere)
		bool bIsTargetable = false;
};

//GridManager for use in Editor only
//Allows for dynamic creation, exporting and importing of Maps
UCLASS()
class DECK_API AGridManager_Editor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridManager_Editor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	//Override Construction to update and remake the Grid when values are changed
	virtual void OnConstruction(const FTransform &Transform) override;

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

	/************
	*Development*
	************/
	
	//Construct the Connections used for Pathfinding
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Development")
		void ConstructBoard();

	//Calculate the Height Scores between the two specified Indices
	void SetHeightScores(int FirstIndex, int SecondIndex);

	//Export the current state of the board to use in a final map
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Development")
		void ExportBoard();

	//Construct and then Export the file
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Development")
		void ExportAndConstructBoard();

	//Import a board from a file
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Development")
		void ImportBoard();

	//Update the Transformation of the specified Index
	UFUNCTION(BlueprintCallable)
		void UpdateIndexTransform(int Index, FRotator NewRotation, float NewHeight = 0.0f, FVector Scale = FVector(1.0f));

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

	//Instanced Static Mesh for the Grid
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
		UInstancedStaticMeshComponent* CollisionPanels;

	//Instanced Static Mesh for Highlights
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		UInstancedStaticMeshComponent* TileHighlights;

	//The X-size of the Grid
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = 0))
		int SizeX = 0;

	//The Y-size of the Grid
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = 0))
		int SizeY = 0;

	//The Tile Size
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = 0.0f))
		float TileSize = 1.0f;

	//The Array of Grid tiles
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
		TArray<FGraphNodeEditor> Grid;

	//File Path to Read/Write to (uses Content directory by default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Development")
		FString FilePath;

	//Flag for whether or not the the current build has the appropriate Connection data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Development")
		bool Outdated = false;
};
