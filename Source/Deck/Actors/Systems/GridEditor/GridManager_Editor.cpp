// Fill out your copyright notice in the Description page of Project Settings.

#include "GridManager_Editor.h"
#include "Actors/Systems/GridEditor/TM_Ramp.h"
#include "Actors/Systems/GridEditor/TM_Wall.h"
#include "Actors/Systems/GridEditor/TM_Null.h"
#include "Actors/Systems/GridEditor/TM_Height.h"
#include "DeckGM.h"
#include "CoreMisc.h"
#include "Engine.h"
#include "stdio.h"

#define GridModifier ECC_GameTraceChannel2

// Sets default values
AGridManager_Editor::AGridManager_Editor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//Create the Grid Mesh as the RootComponent
	GridMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Grid Mesh"));
	RootComponent = GridMesh;

	//Load a default mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh>GridMeshAsset(TEXT("StaticMesh'/Game/Art/Systems/GridBase.GridBase'"));
	UStaticMesh* GridAsset = GridMeshAsset.Object;
	GridMesh->SetStaticMesh(GridAsset);

	//Set Collision response to ECC_GameTraceChannel1 (Grid)
	GridMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Block);

	CollisionPanels = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Collision Panels"));
	CollisionPanels->SetupAttachment(GridMesh);
	CollisionPanels->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	CollisionPanels->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Block);

	//Create and attach the Tile Highlights
	TileHighlights = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Tile Highlights"));
	TileHighlights->SetupAttachment(GridMesh);
	TileHighlights->SetRelativeLocation(FVector(0.0f, 0.0f, 0.1f));

	//Load a default mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh>HighlightMeshAsset(TEXT("StaticMesh'/Game/Art/Systems/GridPanel.GridPanel'"));
	UStaticMesh* HighlightAsset = HighlightMeshAsset.Object;
	TileHighlights->SetStaticMesh(HighlightAsset);

	//Disable collision
	TileHighlights->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

//Override Construction to update and remake the Grid when values are changed
void AGridManager_Editor::OnConstruction(const FTransform &Transform)
{
	//Parent OnConstruction
	Super::OnConstruction(Transform);

	//If the Total Size is not equal to the number of Tiles
	if (SizeX * SizeY != Grid.Num())
	{
		//Set Outdated to True
		Outdated = true;
		//Clear the Instances
		GridMesh->ClearInstances();
		CollisionPanels->ClearInstances();
		TileHighlights->ClearInstances();
		//Empty the Grid
		Grid.Empty();

		//Populate Grid with default values
		//To do - Set up some way of keeping data
		{
			for (int i = 0; i < SizeY; i++)
			{
				for (int j = 0; j < SizeX; j++)
				{
					FGraphNodeEditor Node;
					Node.Position = FVector(j * TileSize, i * TileSize, 0.0f);
					Node.Rotation = FRotator(0.0f);
					Node.Scale = FVector(TileSize);
					Grid.Add(Node);
				}
			}
		}

		//Determine base connections
		{
			for (int i = 0; i < Grid.Num(); i++)
			{
				//Check for i - SizeX - i.e. i = 4, check 0 (where SizeX = 4)
				if (i > SizeX - 1)
				{
					FNodeConnection Connection;
					Connection.Index = i - SizeX;
					Grid[i].Connections.Add(Connection);
				}

				//Check for i + SizeX - i.e. i = 4, check 8 (where SizeX = 4)
				if (i < Grid.Num() - SizeX - 2)
				{
					FNodeConnection Connection;
					Connection.Index = i + SizeX;
					Grid[i].Connections.Add(Connection);
				}

				//Check for i + 1 - i.e. i = 4, check 5
				if (i % SizeX != SizeX - 1)
				{
					FNodeConnection Connection;
					Connection.Index = i + 1;
					Grid[i].Connections.Add(Connection);
				}

				//Check for i - 1 - i.e. i = 4, check 3
				if (i % SizeX != 0)
				{
					FNodeConnection Connection;
					Connection.Index = i - 1;
					Grid[i].Connections.Add(Connection);
				}
			}
		}

		//Create the Instances
		for (int i = 0; i < Grid.Num(); i++)
		{
			GridMesh->AddInstance(FTransform(Grid[i].Rotation.Quaternion(), Grid[i].Position, Grid[i].Scale));
			CollisionPanels->AddInstance(FTransform(FRotator(0.0f), FVector(Grid[i].Position.X, Grid[i].Position.Y, 0.0f), FVector(TileSize)));
		}
	}
	else
	{
		//Check that there are tiles
		if (Grid.Num())
		{
			//Get the Transform at the first Instance
			FTransform InstanceTransform;
			GridMesh->GetInstanceTransform(0, InstanceTransform);

			//If the Scale.Z (never changes) is the same, just Update Instances. Otherwise calculate a new Scale
			if (InstanceTransform.GetScale3D().Z == TileSize)
			{
				//Loop through and update every Instance
				for (int i = 0; i < Grid.Num(); i++)
				{
					GridMesh->UpdateInstanceTransform(i, FTransform(Grid[i].Rotation.Quaternion(), Grid[i].Position, Grid[i].Scale));
					CollisionPanels->UpdateInstanceTransform(i, FTransform(FRotator(0.0f), FVector(Grid[i].Position.X, Grid[i].Position.Y, 0.0f), FVector(TileSize)));
				}
			}
			else
			{
				//Set Outdated to true
				Outdated = true;

				//Loop through every Instance and update with a recalculated Scale
				for (int i = 0; i < Grid.Num(); i++)
				{
					//Recalculate Scale
					float OldSize = InstanceTransform.GetScale3D().Z;
					FVector SizeModifier = InstanceTransform.GetScale3D() / OldSize;
					Grid[i].Scale = SizeModifier * TileSize;

					//Recalculate Locations
					FVector NewLocation;
					NewLocation.X = (i % SizeX) * TileSize;
					NewLocation.Y = (i / SizeX) * TileSize;
					NewLocation.Z = Grid[i].Position.Z;

					Grid[i].Position = NewLocation;

					///
					//Add method for updating Modifiers HERE
					///

					GridMesh->UpdateInstanceTransform(i, FTransform(Grid[i].Rotation.Quaternion(), Grid[i].Position, Grid[i].Scale));
					CollisionPanels->UpdateInstanceTransform(i, FTransform(FRotator(0.0f), FVector(Grid[i].Position.X, Grid[i].Position.Y, 0.0f), FVector(TileSize)));
				}
			}
		}
	}
}

// Called when the game starts or when spawned
void AGridManager_Editor::BeginPlay()
{
	Super::BeginPlay();
}

//Find the Index closest to the specified Location, returns -1 on fail
int AGridManager_Editor::GetIndexFromLocation(FVector Location)
{
	//Convert the Location to be Relative to the Grid
	FVector RelativeLoc = Location - this->GetActorLocation();

	//Calculate the minimum XValue (Relative X divided by TileSize and truncuate - i.e. 470/300 = 1)
	int XVal = (int)(FMath::TruncToFloat(RelativeLoc.X) / TileSize);

	//Check if Relative X % TileSize is greater than TileSize/2 - Checks for between tiles
	if (((int)RelativeLoc.X % (int)TileSize) > (TileSize * 0.5f))
	{
		XVal++;
	}

	//Calculate the minimum YValue
	int YVal = (int)(FMath::TruncToFloat(RelativeLoc.Y) / TileSize);

	//Check for between Tiles
	if (((int)RelativeLoc.Y % (int)TileSize) > (TileSize * 0.5f))
	{
		YVal++;
	}

	//Check that the resulting Index does not exceed the Grid size and return it, otherwise return -1
	if ((XVal + (SizeX * YVal)) < Grid.Num())
		return (XVal + (SizeX * YVal));
	else
		return -1;
}

//Find the Location of the Tile at Index
FVector AGridManager_Editor::GetLocationFromIndex(int Index)
{
	//Check that the Index is within bounds, return the World location, otherwise return 0
	if (Index > -1 && Index < Grid.Num())
		return this->GetActorLocation() + Grid[Index].Position;
	else
		return FVector(0.0f);
}

//Get the closest Index Location to the specified Location
FVector AGridManager_Editor::GetClosestTileLocation(FVector Location)
{
	//Convert the Location to be relative to the Grid
	FVector RelativeLoc = Location - this->GetActorLocation();

	//Calculate the minimum XValue (Relative X divided by TileSize and truncuate - i.e. 470/300 = 1)
	int XVal = (int)(FMath::TruncToFloat(RelativeLoc.X) / TileSize);

	//Check if Relative X % TileSize is greater than TileSize/2 - Checks for between tiles
	if (((int)RelativeLoc.X % (int)TileSize) > (TileSize * 0.5f))
	{
		XVal++;
	}

	//Calculate the minimum YValue
	int YVal = (int)(FMath::TruncToFloat(RelativeLoc.Y) / TileSize);

	//Check that the resulting Index does not exceed the Grid size and return it, otherwise return -1
	if (((int)RelativeLoc.Y % (int)TileSize) > (TileSize * 0.5f))
	{
		YVal++;
	}

	//Check that the resulting Index does not exceed the Grid size and return its World Location, otherwise return a 0 vector
	if ((XVal + (SizeX * YVal)) < Grid.Num())
		return this->GetActorLocation() + Grid[(XVal + (SizeX * YVal))].Position;
	else
		return FVector(0.0f);
}

//Construct the Connections used for Pathfinding
//Check the different TileMods and then create the Scores when this button is pressed
//TileMods can update the Instances but not the scores, allows for it to be done all at once without having to worry about the dynamic score changes
void AGridManager_Editor::ConstructBoard()
{
	//Set Outdated to False (data is built here)
	Outdated = false;

	//Reset all connection scores
	for (int i = 0; i < Grid.Num(); i++)
	{
		for (int j = 0; j < Grid[i].Connections.Num(); j++)
		{
			Grid[i].Connections[j].Score = 0;
		}
	}

	//Check all Null Modifiers and set Connections to -1
	for (TActorIterator<ATM_Null> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ATM_Null* TileMod = *ActorItr;
		int TargetIndex = TileMod->CurrentIndex;

		//Check that Target Index is valid
		if (TargetIndex > -1 && TargetIndex < Grid.Num())
		{
			//Loop through all of the Index's connections
			for (int i = 0; i < Grid[TargetIndex].Connections.Num(); i++)
			{
				//Set the connection score to -1
				Grid[TargetIndex].Connections[i].Score = -1;
				//Save the currently checked Index
				int CheckingIndex = Grid[TargetIndex].Connections[i].Index;

				//Loop through all of the connections of the Index being checked to find the Target Index
				for (int j = 0; j < Grid[CheckingIndex].Connections.Num(); j++)
				{
					if (Grid[CheckingIndex].Connections[j].Index == TargetIndex)
					{
						//Set score to -1
						Grid[CheckingIndex].Connections[j].Score = -1;
					}
				}
			}
		}
	}

	//Create Score based on Height Difference (default to 1)
	//Loop through each Tile
	for (int i = 0; i < Grid.Num(); i++)
	{
		//Loop through the Tile's connections
		for (int j = 0; j < Grid[i].Connections.Num(); j++)
		{
			//If the Score == 0 (not null), set the score between the two indices
			if (Grid[i].Connections[j].Score == 0)
			{
				int CheckingIndex = Grid[i].Connections[j].Index;

				SetHeightScores(i, CheckingIndex);
			}
		}
	}

	//Check all Walls, where Score < Wall update the score to use the Wall's
	for (TActorIterator<ATM_Wall> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		//Get the Current Wall and its Index
		ATM_Wall* TileMod = *ActorItr;
		int TargetIndex = TileMod->CurrentIndex;

		//Search all of the connections at the Target Index
		for (int i = 0; i < Grid[TargetIndex].Connections.Num(); i++)
		{
			//If the score is -1, check the next
			if (Grid[TargetIndex].Connections[i].Score == -1)
				break;

			//Set the Checking Index
			int CheckingIndex = Grid[TargetIndex].Connections[i].Index;

			//Check the +X direction
			if (CheckingIndex - TargetIndex == 1)
			{
				//Check that the +X flag is set on the Wall
				if (TileMod->DirectionFlags & (1 << (int32)(EDirectionFlags::D_PosX)))
				{
					//Search through the Checking Index's connections to find the Target Index
					for (int j = 0; j < Grid[CheckingIndex].Connections.Num(); j++)
					{
						if (Grid[CheckingIndex].Connections[j].Index == TargetIndex)
						{
							//If the Target Index is higher than the CheckingIndex then add the pre-existing Score to the Wall Score
							//(Height + Wall) - i.e. If Target is at 500 and Checking is at 300 and the Wall adds 200, then the score will be be 400 (500-300+200)
							if (Grid[TargetIndex].Position.Z > Grid[CheckingIndex].Position.Z)
							{
								Grid[TargetIndex].Connections[i].Score = TileMod->Score + Grid[TargetIndex].Connections[i].Score;
								Grid[CheckingIndex].Connections[j].Score = TileMod->Score + Grid[CheckingIndex].Connections[j].Score;
							}
							else if (TileMod->Score > Grid[TargetIndex].Connections[i].Score)
							{
								Grid[TargetIndex].Connections[i].Score = TileMod->Score;
								Grid[CheckingIndex].Connections[j].Score = TileMod->Score;
							}
						}
					}
				}
			} //Check the -X direction
			else if (CheckingIndex - TargetIndex == -1)
			{
				//Check that the +X flag is set on the Wall
				if (TileMod->DirectionFlags & (1<<(int32)(EDirectionFlags::D_NegX)))
				{
					//Search through the Checking Index's connections to find the Target Index
					for (int j = 0; j < Grid[CheckingIndex].Connections.Num(); j++)
					{
						if (Grid[CheckingIndex].Connections[j].Index == TargetIndex)
						{
							//If the Target Index is higher than the CheckingIndex then add the pre-existing Score to the Wall Score
							//(Height + Wall) - i.e. If Target is at 500 and Checking is at 300 and the Wall adds 200, then the score will be be 400 (500-300+200)
							if (Grid[TargetIndex].Position.Z > Grid[CheckingIndex].Position.Z)
							{
								Grid[TargetIndex].Connections[i].Score = TileMod->Score + Grid[TargetIndex].Connections[i].Score;
								Grid[CheckingIndex].Connections[j].Score = TileMod->Score + Grid[CheckingIndex].Connections[j].Score;
							}
							else if (TileMod->Score > Grid[TargetIndex].Connections[i].Score)
							{
								Grid[TargetIndex].Connections[i].Score = TileMod->Score;
								Grid[CheckingIndex].Connections[j].Score = TileMod->Score;
							}
						}
					}
				}
			} //Check the +Y direction
			else if (CheckingIndex - TargetIndex == SizeX)
			{
				//Check that the +Y flag is set on the Wall
				if (TileMod->DirectionFlags & (1 << (int32)(EDirectionFlags::D_PosY)))
				{
					//Search through the Checking Index's connections to find the Target Index
					for (int j = 0; j < Grid[CheckingIndex].Connections.Num(); j++)
					{
						if (Grid[CheckingIndex].Connections[j].Index == TargetIndex)
						{
							//If the Target Index is higher than the CheckingIndex then add the pre-existing Score to the Wall Score
							//(Height + Wall) - i.e. If Target is at 500 and Checking is at 300 and the Wall adds 200, then the score will be be 400 (500-300+200)
							if (Grid[TargetIndex].Position.Z > Grid[CheckingIndex].Position.Z)
							{
								Grid[TargetIndex].Connections[i].Score = TileMod->Score + Grid[TargetIndex].Connections[i].Score;
								Grid[CheckingIndex].Connections[j].Score = TileMod->Score + Grid[CheckingIndex].Connections[j].Score;
							}
							else if (TileMod->Score > Grid[TargetIndex].Connections[i].Score)
							{
								Grid[TargetIndex].Connections[i].Score = TileMod->Score;
								Grid[CheckingIndex].Connections[j].Score = TileMod->Score;
							}
						}
					}
				}
			} //Check the -Y direction
			else if (CheckingIndex - TargetIndex == -SizeX)
			{
				//Check that the -Y flag is set on the Wall
				if (TileMod->DirectionFlags & (1 << (int32)(EDirectionFlags::D_NegY)))
				{
					//Search through the Checking Index's connections to find the Target Index
					for (int j = 0; j < Grid[CheckingIndex].Connections.Num(); j++)
					{
						if (Grid[CheckingIndex].Connections[j].Index == TargetIndex)
						{
							//If the Target Index is higher than the CheckingIndex then add the pre-existing Score to the Wall Score
							//(Height + Wall) - i.e. If Target is at 500 and Checking is at 300 and the Wall adds 200, then the score will be be 400 (500-300+200)
							if (Grid[TargetIndex].Position.Z > Grid[CheckingIndex].Position.Z)
							{
								Grid[TargetIndex].Connections[i].Score = TileMod->Score + Grid[TargetIndex].Connections[i].Score;
								Grid[CheckingIndex].Connections[j].Score = TileMod->Score + Grid[CheckingIndex].Connections[j].Score;
							}
							else if (TileMod->Score > Grid[TargetIndex].Connections[i].Score)
							{
								Grid[TargetIndex].Connections[i].Score = TileMod->Score;
								Grid[CheckingIndex].Connections[j].Score = TileMod->Score;
							}
						}
					}
				}
			}
		}
	}

}

//Calculate the Height Scores between the two specified Indices
void AGridManager_Editor::SetHeightScores(int FirstIndex, int SecondIndex)
{
	//Check if either Tile has a rotation to it, otherwise use default height calculations
	if (Grid[FirstIndex].Rotation != FRotator(0.0f) || Grid[SecondIndex].Rotation != FRotator(0.0f))
	{
		//Find the Max and Min heights for both indices - Default to their 'Mid Height' which can be used if one lacks a rotation
		float FirstIndexMax = Grid[FirstIndex].Position.Z;
		float FirstIndexMin = Grid[FirstIndex].Position.Z;
		float SecondIndexMax = Grid[SecondIndex].Position.Z;
		float SecondIndexMin = Grid[SecondIndex].Position.Z;
		
		//Calculate Max and Mins for First Index
		{
			float Diff;
			//If Pitch != 0, calculate using the Pitch. Otherwise check the Roll
			if (Grid[FirstIndex].Rotation.Pitch != 0.0f)
			{
				Diff = FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Grid[FirstIndex].Rotation.Pitch))) * (TileSize * 0.5f);
				FirstIndexMax += Diff;
				FirstIndexMin -= Diff;
			}
			else if (Grid[FirstIndex].Rotation.Roll != 0.0f)
			{
				Diff = FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Grid[FirstIndex].Rotation.Roll))) * (TileSize * 0.5f);
				FirstIndexMax += Diff;
				FirstIndexMin -= Diff;
			}
		}

		//Calculate Max and Mins for Second Index
		{
			float Diff;
			//If Pitch != 0, calculate using the Pitch. Otherwise check the Roll
			if (Grid[SecondIndex].Rotation.Pitch != 0.0f)
			{
				Diff = FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Grid[SecondIndex].Rotation.Pitch))) * (TileSize * 0.5f);
				SecondIndexMax += Diff;
				SecondIndexMin -= Diff;
			}
			else if (Grid[SecondIndex].Rotation.Roll != 0.0f)
			{
				Diff = FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Grid[SecondIndex].Rotation.Roll))) * (TileSize * 0.5f);
				SecondIndexMax += Diff;
				SecondIndexMin -= Diff;
			}
		}

		//Return Score defaulted to basic Height difference
		int ReturnScore = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - Grid[SecondIndex].Position.Z));

		//If SecondIndex is +X from First Index...
		if (SecondIndex - FirstIndex == 1)
		{
			/*Scenarios
			- FirstIndex has +Pitch and SecondIndex has +Pitch = FirstMax versus SecondMin
			- FirstIndex has +Pitch and SecondIndex has -Pitch = FirstMax versus SecondMax
			- FirstIndex has +Pitch and SecondIndex has no Pitch = FirstMax versus SecondMid

			- FirstIndex has -Pitch and SecondIndex has +Pitch = FirstMin versus SecondMin
			- FirstIndex has -Pitch and SecondIndex has -Pitch = FirstMin versus SecondMax
			- FirstIndex has -Pitch and SecondIndex has no Pitch = FirstMin versus SecondMid

			- FirstIndex has no Pitch and SecondIndex has +Pitch = FirstMid versus SecondMin
			- FirstIndex has no Pitch and SecondIndex has -Pitch = FirstMid versus SecondMax
			- FirstIndex has no Pitch and SecondIndex has no Pitch = FirstMid versus SecondMid
			*/
			if (Grid[FirstIndex].Rotation.Pitch > 0.0f)
			{
				if (Grid[SecondIndex].Rotation.Pitch > 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMax - SecondIndexMin));
				}
				else if (Grid[SecondIndex].Rotation.Pitch < 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMax - SecondIndexMax));
				}
				else if (Grid[SecondIndex].Rotation.Pitch == 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMax - Grid[SecondIndex].Position.Z));
				}
			}
			else if (Grid[FirstIndex].Rotation.Pitch < 0.0f)
			{
				if (Grid[SecondIndex].Rotation.Pitch > 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMin - SecondIndexMin));
				}
				else if (Grid[SecondIndex].Rotation.Pitch < 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMin - SecondIndexMax));
				}
				else if (Grid[SecondIndex].Rotation.Pitch == 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMin - Grid[SecondIndex].Position.Z));
				}
			}
			else if (Grid[FirstIndex].Rotation.Pitch == 0.0f)
			{
				if (Grid[SecondIndex].Rotation.Pitch > 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - SecondIndexMin));
				}
				else if (Grid[SecondIndex].Rotation.Pitch < 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - SecondIndexMax));
				}
				else if (Grid[SecondIndex].Rotation.Pitch == 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - Grid[SecondIndex].Position.Z));
				}
			}
		}//If SecondIndex is -X from First Index...
		else if (SecondIndex - FirstIndex == -1)
		{
			/*Scenarios
			- FirstIndex has +Pitch and SecondIndex has +Pitch = FirstMin versus SecondMax
			- FirstIndex has +Pitch and SecondIndex has -Pitch = FirstMin versus SecondMin
			- FirstIndex has +Pitch and SecondIndex has no Pitch = FirstMin versus SecondMid

			- FirstIndex has -Pitch and SecondIndex has +Pitch = FirstMax versus SecondMax
			- FirstIndex has -Pitch and SecondIndex has -Pitch = FirstMax versus SecondMin
			- FirstIndex has -Pitch and SecondIndex has no Pitch = FirstMax versus SecondMid

			- FirstIndex has no Pitch and SecondIndex has +Pitch = FirstMid versus SecondMax
			- FirstIndex has no Pitch and SecondIndex has -Pitch = FirstMid versus SecondMin
			- FirstIndex has no Pitch and SecondIndex has no Pitch = FirstMid versus SecondMid
			*/
			if (Grid[FirstIndex].Rotation.Pitch > 0.0f)
			{
				if (Grid[SecondIndex].Rotation.Pitch > 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMin - SecondIndexMax));
				}
				else if (Grid[SecondIndex].Rotation.Pitch < 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMin - SecondIndexMin));
				}
				else if (Grid[SecondIndex].Rotation.Pitch == 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMin - Grid[SecondIndex].Position.Z));
				}
			}
			else if (Grid[FirstIndex].Rotation.Pitch < 0.0f)
			{
				if (Grid[SecondIndex].Rotation.Pitch > 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMax - SecondIndexMax));
				}
				else if (Grid[SecondIndex].Rotation.Pitch < 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMax - SecondIndexMin));
				}
				else if (Grid[SecondIndex].Rotation.Pitch == 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMax - Grid[SecondIndex].Position.Z));
				}
			}
			else if (Grid[FirstIndex].Rotation.Pitch == 0.0f)
			{
				if (Grid[SecondIndex].Rotation.Pitch > 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - SecondIndexMax));
				}
				else if (Grid[SecondIndex].Rotation.Pitch < 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - SecondIndexMin));
				}
				else if (Grid[SecondIndex].Rotation.Pitch == 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - Grid[SecondIndex].Position.Z));
				}
			}
		}//If SecondIndex is +Y from First Index...
		else if (SecondIndex - FirstIndex == SizeX)
		{
			/*Scenarios
			- FirstIndex has +Roll and SecondIndex has +Roll = FirstMin versus SecondMax
			- FirstIndex has +Roll and SecondIndex has -Roll = FirstMin versus SecondMin
			- FirstIndex has +Roll and SecondIndex has no Roll = FirstMin versus SecondMid

			- FirstIndex has -Roll and SecondIndex has +Roll = FirstMax versus SecondMax
			- FirstIndex has -Roll and SecondIndex has -Roll = FirstMax versus SecondMin
			- FirstIndex has -Roll and SecondIndex has no Roll = FirstMax versus SecondMid

			- FirstIndex has no Roll and SecondIndex has +Roll = FirstMid versus SecondMax
			- FirstIndex has no Roll and SecondIndex has -Roll = FirstMid versus SecondMin
			- FirstIndex has no Roll and SecondIndex has no Roll = FirstMid versus SecondMid
			*/
			if (Grid[FirstIndex].Rotation.Roll > 0.0f)
			{
				if (Grid[SecondIndex].Rotation.Roll > 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMin - SecondIndexMax));
				}
				else if (Grid[SecondIndex].Rotation.Roll < 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMin - SecondIndexMin));
				}
				else if (Grid[SecondIndex].Rotation.Roll == 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMin - Grid[SecondIndex].Position.Z));
				}
			}
			else if (Grid[FirstIndex].Rotation.Roll < 0.0f)
			{
				if (Grid[SecondIndex].Rotation.Roll > 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMax - SecondIndexMax));
				}
				else if (Grid[SecondIndex].Rotation.Roll < 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMax - SecondIndexMin));
				}
				else if (Grid[SecondIndex].Rotation.Roll == 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMax - Grid[SecondIndex].Position.Z));
				}
			}
			else if (Grid[FirstIndex].Rotation.Roll == 0.0f)
			{
				if (Grid[SecondIndex].Rotation.Roll > 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - SecondIndexMax));
				}
				else if (Grid[SecondIndex].Rotation.Roll < 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - SecondIndexMin));
				}
				else if (Grid[SecondIndex].Rotation.Roll == 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - Grid[SecondIndex].Position.Z));
				}
			}
		}
		else if (SecondIndex - FirstIndex == -SizeX)
		{
			/*Scenarios
			- FirstIndex has +Roll and SecondIndex has +Roll = FirstMax versus SecondMin
			- FirstIndex has +Roll and SecondIndex has -Roll = FirstMax versus SecondMax
			- FirstIndex has +Roll and SecondIndex has no Roll = FirstMax versus SecondMid

			- FirstIndex has -Roll and SecondIndex has +Roll = FirstMin versus SecondMin
			- FirstIndex has -Roll and SecondIndex has -Roll = FirstMin versus SecondMax
			- FirstIndex has -Roll and SecondIndex has no Roll = FirstMin versus SecondMid

			- FirstIndex has no Roll and SecondIndex has +Roll = FirstMid versus SecondMin
			- FirstIndex has no Roll and SecondIndex has -Roll = FirstMid versus SecondMax
			- FirstIndex has no Roll and SecondIndex has no Roll = FirstMid versus SecondMid
			*/
			if (Grid[FirstIndex].Rotation.Roll > 0.0f)
			{
				if (Grid[SecondIndex].Rotation.Roll > 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMax - SecondIndexMin));
				}
				else if (Grid[SecondIndex].Rotation.Roll < 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMax - SecondIndexMax));
				}
				else if (Grid[SecondIndex].Rotation.Roll == 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMax - Grid[SecondIndex].Position.Z));
				}
			}
			else if (Grid[FirstIndex].Rotation.Roll < 0.0f)
			{
				if (Grid[SecondIndex].Rotation.Roll > 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMin - SecondIndexMin));
				}
				else if (Grid[SecondIndex].Rotation.Roll < 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMin - SecondIndexMax));
				}
				else if (Grid[SecondIndex].Rotation.Roll == 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(FirstIndexMin - Grid[SecondIndex].Position.Z));
				}
			}
			else if (Grid[FirstIndex].Rotation.Roll == 0.0f)
			{
				if (Grid[SecondIndex].Rotation.Roll > 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - SecondIndexMin));
				}
				else if (Grid[SecondIndex].Rotation.Roll < 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - SecondIndexMax));
				}
				else if (Grid[SecondIndex].Rotation.Roll == 0.0f)
				{
					ReturnScore = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - Grid[SecondIndex].Position.Z));
				}
			}
		}

		//Loop through all of FirstIndex's connections to find the SecondIndex
		for (int i = 0; i < Grid[FirstIndex].Connections.Num(); i++)
		{
			if (Grid[FirstIndex].Connections[i].Index == SecondIndex)
			{
				//Set the Score
				Grid[FirstIndex].Connections[i].Score = ReturnScore;
			}
		}
		//Loop through all of SecondIndex's connections to find the FirstIndex
		for (int i = 0; i < Grid[SecondIndex].Connections.Num(); i++)
		{
			if (Grid[SecondIndex].Connections[i].Index == FirstIndex)
			{
				//Set the Score
				Grid[SecondIndex].Connections[i].Score = ReturnScore;
			}
		}

	} //If there is no Rotation, use a default Height check
	else
	{
		//Loop through all of FirstIndex's connections to find the SecondIndex
		for (int i = 0; i < Grid[FirstIndex].Connections.Num(); i++)
		{
			if (Grid[FirstIndex].Connections[i].Index == SecondIndex)
			{
				//Score is the Abs of the difference in height, clamped down to 1
				Grid[FirstIndex].Connections[i].Score = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - Grid[SecondIndex].Position.Z));
			}
		}
		//Loop through all of SecondIndex's connections to find the FirstIndex
		for (int i = 0; i < Grid[SecondIndex].Connections.Num(); i++)
		{
			if (Grid[SecondIndex].Connections[i].Index == FirstIndex)
			{
				//Score is the Abs of the difference in height, clamped down to 1
				Grid[SecondIndex].Connections[i].Score = FMath::Max(1, (int)FMath::Abs(Grid[FirstIndex].Position.Z - Grid[SecondIndex].Position.Z));
			}
		}
	}
}

//Export the current state of the board to use in a final map
void AGridManager_Editor::ExportBoard()
{
	//The final output
	FString Output;

	//If the map is outdated, aport with a warning message
	if (!Outdated)
	{
		//SizeX
		Output.Append(FString::FromInt(SizeX) + TEXT("\n"));
		//Size Y
		Output.Append(FString::FromInt(SizeY) + TEXT("\n"));
		//Tile Size
		Output.Append(FString::SanitizeFloat(TileSize));

		//Loop through each Tile
		for (int i = 0; i < Grid.Num(); i++)
		{
			//The current Tile's data
			FString LineData;
			//Index
			LineData.Append(TEXT("\n"));
			LineData.Append(FString::FromInt(i) + TEXT("|"));

			//Position
			LineData.Append(FString::SanitizeFloat(Grid[i].Position.X) + TEXT("|") + FString::SanitizeFloat(Grid[i].Position.Y) + TEXT("|") + FString::SanitizeFloat(Grid[i].Position.Z) + TEXT("|"));

			//Rotation
			LineData.Append(FString::SanitizeFloat(Grid[i].Rotation.Pitch) + TEXT("|") + FString::SanitizeFloat(Grid[i].Rotation.Yaw) + TEXT("|") + FString::SanitizeFloat(Grid[i].Rotation.Roll) + TEXT("\n"));

			//Loop through all the Connections
			for (int j = 0; j < Grid[i].Connections.Num(); j++)
			{
				//Index
				LineData.Append(FString::FromInt(Grid[i].Connections[j].Index) + TEXT("|"));
				//Connection Score
				LineData.Append(FString::FromInt(Grid[i].Connections[j].Score));

				if (j < Grid[i].Connections.Num() - 1)
				{
					LineData.Append(TEXT("|"));
				}
			}

			//Append the data to the Output
			Output.Append(LineData);
		}

		//Loop through each Wall
		for (TActorIterator<ATM_Wall> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			//Get the Current Wall
			ATM_Wall* Wall = *ActorItr;
			//The Current Wall's data
			FString LineData;

			//Check that the Wall is at a valid Index
			if (Wall->CurrentIndex > -1 && Wall->CurrentIndex < Grid.Num())
			{
				LineData.Append(TEXT("\n"));
				//Save Index
				LineData.Append(FString::FromInt(Wall->CurrentIndex) + TEXT("|"));
				//Save Height and Thickness
				LineData.Append(FString::SanitizeFloat(Wall->Height) + TEXT("|") + FString::SanitizeFloat(Wall->Thickness));

				//Check every Directional Flag
				for (int32 i = (int32)EDirectionFlags::D_PosX; i <= (int32)EDirectionFlags::D_NegY; i++)
				{
					LineData.Append(TEXT("|"));
					//If the Flag is active, add 1 (True). Otherwise add 0 (False)
					if (Wall->DirectionFlags & (1 << i))
					{
						LineData.Append(TEXT("1"));
					}
					else
					{
						LineData.Append(TEXT("0"));
					}
				}
			}

			//Append the data to the Output
			Output.Append(LineData);
		}

		//Save to the File within the Project's Content Directory
		if (FFileHelper::SaveStringToFile(Output, *(FPaths::ProjectContentDir() + FilePath)))
		{

			UE_LOG(LogTemp, Warning, TEXT("File Saved!"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("FAILED! File Not Saved"));
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("FAIELD TO EXPORT - Something went wrong!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FAILED! Board is out of date"));
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("FAILED TO EXPORT - Grid is out of date!"));
	}
}

//Construct and then Export the file
void AGridManager_Editor::ExportAndConstructBoard()
{
	//Construct
	ConstructBoard();

	//Export
	ExportBoard();
}

//Import a board from a file
void AGridManager_Editor::ImportBoard()
{
	//The Output from the File
	FString Output;

	//Clear all Instances and empty the Grid
	Grid.Empty();
	GridMesh->ClearInstances();
	TileHighlights->ClearInstances();

	//Load the File from the Project Content Directory
	if (FFileHelper::LoadFileToString(Output, *(FPaths::ProjectContentDir() + FilePath)))
	{
		//Break the Output into Lines based on the \n symbol
		TArray<FString> Lines;
		Output.ParseIntoArray(Lines, TEXT("\n"));

		//Set SizeX, SizeY and TileSize using the first 3 Lines
		SizeX = FCString::Atoi(*Lines[0]);
		SizeY = FCString::Atoi(*Lines[1]);
		TileSize = FCString::Atof(*Lines[2]);
		
		//Starting at the 4th line, loop through for SizeX*SizeY *2 times while going up every 2 lines
		//1st line - Node Transformation data, 2nd line - Node Connection data
		for (int i = 3; i < 2 + (SizeX * SizeY) * 2; i += 2)
		{
			//Create a NewNode
			FGraphNodeEditor NewNode;
			//Split the current line into the appropriate data
			TArray<FString> SampleLines;
			Lines[i].ParseIntoArray(SampleLines, TEXT("|"));

			//Check that i+1 doesn't go out of bounds
			if (i + 1 < Lines.Num())
			{
				//The correct data should break into 7 parts
				if (SampleLines.Num() == 7)
				{
					//Create defaults for Index, IndexPosition and IndexRotation
					int Index = -1;
					FVector IndexPosition = FVector(0.0f);
					FRotator IndexRotation = FRotator(0.0f);

					//Set Index
					Index = FCString::Atoi(*SampleLines[0]);
					//Set Position
					IndexPosition.X = FCString::Atof(*SampleLines[1]);
					IndexPosition.Y = FCString::Atof(*SampleLines[2]);
					IndexPosition.Z = FCString::Atof(*SampleLines[3]);
					//Set Rotation
					IndexRotation.Pitch = FCString::Atof(*SampleLines[4]);
					IndexRotation.Yaw = FCString::Atof(*SampleLines[5]);
					IndexRotation.Roll = FCString::Atof(*SampleLines[6]);

					//Set the Node's Position and Rotation
					NewNode.Position = IndexPosition;
					NewNode.Rotation = IndexRotation;
				}

				//Parse the next line
				Lines[i + 1].ParseIntoArray(SampleLines, TEXT("|"));
				
				{
					//Loop through the Lines data, 2 at a time (Connection Index|Connection Score)
					for (int j = 0; j < SampleLines.Num(); j += 2)
					{
						//Check that the 2nd data to make sure it's valid
						if (j + 1 < SampleLines.Num())
						{
							//Create a new Connection
							FNodeConnection NewConnection;

							//Parse out the data for the Connection Index and Score
							int ConnectedIndex = -1;
							int ConnectedScore = -1;
							ConnectedIndex = FCString::Atoi(*SampleLines[j]);
							ConnectedScore = FCString::Atoi(*SampleLines[j+1]);

							//Set and add the new Connection
							NewConnection.Index = ConnectedIndex;
							NewConnection.Score = ConnectedScore;
							NewNode.Connections.Add(NewConnection);
						}
					}
				}

				//Set the Node's Scale to 0
				NewNode.Scale = FVector(0.0f);

				{
					//Loop through the Node's connections while the Scale is still 0
					for (int j = 0; j < NewNode.Connections.Num() && NewNode.Scale == FVector(0.0f); j++)
					{
						//If a score is found, Scale = TileSize
						if (NewNode.Connections[j].Score != -1)
							NewNode.Scale = FVector(TileSize);
					}

					//If the Scale is TileSize and the Tile has a rotation to it
					if (NewNode.Scale == FVector(TileSize) && NewNode.Rotation != FRotator(0.0f))
					{
						//Check if the Rotation is Pitch or Roll
						if (NewNode.Rotation.Pitch != 0.0f)
						{
							//Calculate a modified Scale
							float ScaleModifier = (TileSize / FMath::Cos(FMath::DegreesToRadians(NewNode.Rotation.Pitch))) / TileSize;
							NewNode.Scale = FVector(TileSize * ScaleModifier, TileSize, TileSize);
						}
						else if (NewNode.Rotation.Roll != 0.0f)
						{
							//Calculate a modified Scale
							float ScaleModifier = (TileSize / FMath::Cos(FMath::DegreesToRadians(NewNode.Rotation.Roll))) / TileSize;
							NewNode.Scale = FVector(TileSize, TileSize * ScaleModifier, TileSize);
						}
					}
				}

				//Add the Node
				Grid.Add(NewNode);
			}
			else
			{
				//Abort if invalid data is found
				UE_LOG(LogTemp, Warning, TEXT("ERROR WITH IMPORT - Early abort"));
				return;
			}
		}

		//Retrieving Wall data
		//LineCount should start after the Node data
		int LineCount = 3 + (SizeX * SizeY) * 2;
		//Loop through until the LineCount exceeds the number of Lines
		while (LineCount < Lines.Num())
		{
			//Parse the current line
			TArray<FString> SampleLines;
			Lines[LineCount].ParseIntoArray(SampleLines, TEXT("|"));

			//The Sample data should be 7 exactly if valid
			if (SampleLines.Num() == 7)
			{
				//Save the Index
				int Index = FCString::Atoi(*SampleLines[0]);

				//Check that the Index is valid
				if (Index > -1 && Index < Grid.Num())
				{
					//Save the Height and Thickness of the Wall
					float Height = FCString::Atof(*SampleLines[1]);
					float Thickness = FCString::Atof(*SampleLines[2]);
					uint8 DirectionFlags = 0;

					//Check which DirectionFlags should be enabled
					for (int32 i = (int32)EDirectionFlags::D_PosX; i <= (int32)EDirectionFlags::D_NegY; ++i)
					{
						if (3 + i < SampleLines.Num())
						{
							if (FCString::Atoi(*SampleLines[3 + i]) == 1)
							{
								//Set Direction Flag
								DirectionFlags |= (1 << i);
							}
						}
					}

					//Create a Wall and set its data
					ATM_Wall* WallMod = GetWorld()->SpawnActor<ATM_Wall>(FVector(Grid[Index].Position.X, Grid[Index].Position.Y, Grid[Index].Position.Z) + this->GetActorLocation(), FRotator(0.0f));
					WallMod->CurrentIndex = Index;
					WallMod->Height = Height;
					WallMod->Thickness = Thickness;
					WallMod->DirectionFlags = DirectionFlags;
					WallMod->InstantiateWalls(TileSize);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("ERROR WITH WALL AT INDEX: %d"), Index);
				}

			}

			//Increment LineCount
			LineCount++;
		}

		//Placement for Null, Ramp and Height modifiers
		//Loop through all the Tiles
		for (int i = 0; i < Grid.Num(); i++)
		{
			//Save a reference to the node
			FGraphNodeEditor CurrentNode = Grid[i];

			//If the Node has a Z element it is either a Ramp or Height
			if (Grid[i].Position.Z != 0.0f)
			{
				//Check for a Rotation
				if (Grid[i].Rotation.Pitch != 0.0f || Grid[i].Rotation.Roll != 0.0f)
				{
					//If there is a Rotation, create a Ramp
					ATM_Ramp* RampMod = GetWorld()->SpawnActor<ATM_Ramp>(FVector(CurrentNode.Position.X, CurrentNode.Position.Y, CurrentNode.Position.Z * 0.5f) + this->GetActorLocation(), FRotator(0.0f));
					//Set the Ramp's Index
					RampMod->CurrentIndex = i;

					//Check for Pitch or Roll
					if (CurrentNode.Rotation.Pitch != 0.0f)
					{
						//Set Rotation data
						RampMod->RampDirection = ETilt::T_XOnly;
						RampMod->Angle = CurrentNode.Rotation.Pitch;
					}
					else if (CurrentNode.Rotation.Roll != 0.0f)
					{
						//Set Rotation data
						RampMod->RampDirection = ETilt::T_YOnly;
						RampMod->Angle = CurrentNode.Rotation.Roll;
					}

					//Set MinHeight
					RampMod->MinHeight = CurrentNode.Position.Z - (FMath::Abs(FMath::Tan(FMath::DegreesToRadians(RampMod->Angle))) * TileSize) * 0.5f;
					//Set the BoxExtent and Ramp Location
					RampMod->Box->SetBoxExtent(FVector(TileSize*0.5f, TileSize*0.5f, (CurrentNode.Position.Z + (CurrentNode.Position.Z - RampMod->MinHeight))*0.5f));
					RampMod->SetActorLocation(FVector(CurrentNode.Position.X, CurrentNode.Position.Y, (CurrentNode.Position.Z + (CurrentNode.Position.Z - RampMod->MinHeight))*0.5f) + this->GetActorLocation());
				}
				else
				{
					//If there is no Rotation, create a Height
					ATM_Height* HeightMod = GetWorld()->SpawnActor<ATM_Height>(FVector(CurrentNode.Position.X, CurrentNode.Position.Y, CurrentNode.Position.Z * 0.5f) + this->GetActorLocation(), FRotator(0.0f));
					//Set the Index
					HeightMod->CurrentIndex = i;
					//Set the Height
					HeightMod->Height = CurrentNode.Position.Z;
					//Set the Box Extent
					HeightMod->Box->SetBoxExtent(FVector(TileSize*0.5f, TileSize*0.5f, CurrentNode.Position.Z * 0.5f));
				}
			}
			else
			{
				//int8 for checking success
				uint8 Success = 1;

				//Search all Connections
				for (int j = 0; j < Grid[i].Connections.Num(); j++)
				{
					//If any connections have a score not equal to -1
					if (Grid[i].Connections[j].Score != -1)
					{
						//Fail and abort loop
						Success = 0;
						break;
					}
				}

				//If all connections are -1
				if (Success)
				{
					//Create a Null
					ATM_Null* NullMod = GetWorld()->SpawnActor<ATM_Null>(FVector(CurrentNode.Position.X, CurrentNode.Position.Y, TileSize * 0.5f) + this->GetActorLocation(), FRotator(0.0f));
					//Set Index
					NullMod->CurrentIndex = i;
					//Set GridRef
					NullMod->GridRef = this;
					//Set BoxExtents
					NullMod->Box->SetBoxExtent(FVector(TileSize*0.5f));
				}
			}

			//Return the Current Node (prevents data from being overwritten)
			Grid[i] = CurrentNode;

			//Create an Instance on the GridMesh using the Tile's data
			GridMesh->AddInstance(FTransform(Grid[i].Rotation, Grid[i].Position, Grid[i].Scale));
		}
	}
}

//Update the Transformation of the specified Index
void AGridManager_Editor::UpdateIndexTransform(int Index, FRotator NewRotation, float NewHeight, FVector Scale)
{
	//Check that the Index is valid
	if (Index > -1 && Index < Grid.Num())
	{
		//Set Outdated to true
		Outdated = true;

		//Set the Tile's Z position
		Grid[Index].Position.Z = NewHeight;
		//Set the Tile's rotation
		Grid[Index].Rotation = NewRotation;
		//Set the Tile's scale
		Grid[Index].Scale = FVector(TileSize * Scale.X, TileSize * Scale.Y, TileSize * Scale.Z);
		//Update the Instance at Index
		GridMesh->UpdateInstanceTransform(Index, FTransform(NewRotation, Grid[Index].Position, Grid[Index].Scale), false, true);
	}
}

//Heuristic Estimate for pathfinding
int AGridManager_Editor::HeuristicEstimate(int StartIndex, int Goal)
{
	//Get the X-distance
	int XVal = FMath::Abs(StartIndex % SizeX - Goal % SizeX);
	//Get the Y-distance
	int YVal = FMath::Abs((StartIndex - (StartIndex % SizeX)) / SizeX - (Goal - (Goal % SizeX)) / SizeX);
	//Get the Z-distance (scaled down by half to be less impactful)
	//-Update to use Height
	int ZVal = 0;

	//Return the sum of all 3
	return XVal + YVal + ZVal;
}

//Using the map, reconstruct the best path
TArray<int> AGridManager_Editor::ReconstructPath(TArray<SearchNode> Map, int Current)
{
	//Path to Return
	TArray<int> Path;
	//Add the Current index to the Path
	Path.Add(Current);

	//While there is a NodeCameFrom on the Current index, loop
	while (Map[Current].NodeCameFrom != -1)
	{
		//Set the Current to the CameFrom
		Current = Map[Current].NodeCameFrom;
		//Add Current to the start of the Path
		Path.Insert(Current, 0);
	}

	//Return the Path
	return Path;
}

//Find a path between the two locations using a Height tolerance (A*)
TArray<int> AGridManager_Editor::FindPath(FVector Start, FVector End, float MaxHeight)
{
	//Get the Start and End Indices from locations
	int StartIndex = GetIndexFromLocation(Start);
	int EndIndex = GetIndexFromLocation(End);

	//Check that both are valid
	if (StartIndex == -1 || EndIndex == -1)
		return TArray<int>();

	//Clamp the MaxHeight (safety)
	MaxHeight = FMath::Max(1.0f, MaxHeight);

	//Set up Closed set, Open set and Map
	TArray<int> ClosedSet;
	TArray<int> OpenSet;
	OpenSet.Add(StartIndex);
	TArray<SearchNode> Map;
	Map.SetNum(Grid.Num());
	Map[StartIndex].gScore = 0;
	Map[StartIndex].fScore = HeuristicEstimate(StartIndex, EndIndex);

	//While there is a node to evaluate
	while (OpenSet.Num() != 0)
	{
		//Set CurrentNode to the first in the list
		int CurrentNode = OpenSet[0];

		//Loop through from the 2nd until the end
		for (int i = 1; i < OpenSet.Num(); i++)
		{
			//Find the Node with the lowest score
			if (Map[OpenSet[i]].fScore < Map[CurrentNode].fScore)
			{
				CurrentNode = OpenSet[i];
			}
		}

		//Check if Current is the End
		if (CurrentNode == EndIndex)
		{
			//If it is, reconstruct the path and return
			return ReconstructPath(Map, CurrentNode);
		}

		//Remove the Current Node from the Open set and move it to the Closed set
		OpenSet.Remove(CurrentNode);
		ClosedSet.Add(CurrentNode);

		//Check all the Connections for the Current Node
		for (int i = 0; i < Grid[CurrentNode].Connections.Num(); i++)
		{
			//Save a reference to the currently evaluated neighbour
			int Neighbour = Grid[CurrentNode].Connections[i].Index;

			//If the Closed set does not contain the Neighbour
			if (!ClosedSet.Contains(Neighbour))
			{
				//Check that the Score is valid (not -1 and within Height tolerance)
				if (Grid[CurrentNode].Connections[i].Score != -1 && Grid[CurrentNode].Connections[i].Score <= (int)MaxHeight)
				{
					//Make a temp score
					int tempGScore = Map[CurrentNode].gScore + Grid[CurrentNode].Connections[i].Score;

					//Check that the Open set doesn't contain the Neighbour
					if (!OpenSet.Contains(Neighbour))
					{
						//Add the Neighbour to the Openset
						OpenSet.Add(Neighbour);

						//Make the Map data for the current node
						Map[Neighbour].NodeCameFrom = CurrentNode;
						Map[Neighbour].gScore = tempGScore;
						Map[Neighbour].fScore = tempGScore + HeuristicEstimate(Neighbour, EndIndex);
					}
					else if (tempGScore < Map[Neighbour].gScore)
					{
						//If the temp score is less than the previously evaluated score
						//Update with the new, lower score
						Map[Neighbour].NodeCameFrom = CurrentNode;
						Map[Neighbour].gScore = tempGScore;
						Map[Neighbour].fScore = tempGScore + HeuristicEstimate(Neighbour, EndIndex);
					}
				}
			}
		}
	}

	//Return an empty array on failure
	return TArray<int>();
}

//Highlight the Tiles that could be reached through Movement within the MovementRange and Height tolerance
void AGridManager_Editor::HighlightMovement(FVector Start, int MovementRange, float MaxHeight)
{
	//Get the Start Index
	int StartIndex = GetIndexFromLocation(Start);

	//Check that the Start Index is valid
	if (StartIndex == -1)
		return;

	//Clamp the MaxHeight to 1
	MaxHeight = FMath::Max(1.0f, MaxHeight);

	//Create an Open and Closed set
	TArray<int> ClosedSet;
	TArray<int> OpenSet;
	OpenSet.Add(StartIndex);

	//Loop based on the Movement Range
	for (int i = 0; i < MovementRange; i++)
	{
		//Index's being added
		TArray<int> NextPass;

		//While the Open Set has data
		while (OpenSet.Num())
		{
			//Check the first node
			int CurrentNode = OpenSet[0];
			//Remove the Current Node from the Open Set and add it to the Closed Set
			OpenSet.Remove(CurrentNode);
			ClosedSet.Add(CurrentNode);

			//Loop through all of the Node's connections
			for (int j = 0; j < Grid[CurrentNode].Connections.Num(); j++)
			{
				//Check that the Node is valid
				if (Grid[CurrentNode].Connections[j].Score != -1 && Grid[CurrentNode].Connections[j].Score <= (int)MaxHeight)
				{
					//Check that the Neighbour has not already been evaluated
					int Neighbour = Grid[CurrentNode].Connections[j].Index;
					if (!ClosedSet.Contains(Neighbour) && !NextPass.Contains(Neighbour))
					{
						//Add it to the Next Pass
						NextPass.Add(Neighbour);
					}
				}
			}
		}

		//Set OpenSet to the Next Pass
		OpenSet = NextPass;
	}

	//Append the Open Set from the last check to the end of the Closed Set
	ClosedSet.Append(OpenSet);

	//Create a Tile Highlight at each instance of the Closed Set
	for (int i = 0; i < ClosedSet.Num(); i++)
	{
		FTransform Transform;
		GridMesh->GetInstanceTransform(ClosedSet[i], Transform);
		TileHighlights->AddInstance(Transform);
	}
}

//Highlight the Tiles that can be reached by the specified Attack Pattern
void AGridManager_Editor::HighlightAttackPlacement(FVector Start, EPatternType Pattern, int Min, int Max)
{
	//Get the Start Index from the Location
	int StartIndex = GetIndexFromLocation(Start);

	//Switch based on the Pattern used
	switch (Pattern)
	{
		//For a Line
	case EPatternType::PT_Line :
		{
			//Min is clamped to 1 at the lowest
			Min = FMath::Max(1, Min);
			//Max can not be lower than Min
			Max = FMath::Max(Max, Min);

			//Loop from Min to Max
			for (int i = Min; i <= Max; i++)
			{
				//+X with edge check
				int Index = StartIndex + i;
				if (i < (SizeX - (StartIndex % SizeX)))
				{
					HighlightTile(Index);
				}

				//+Y
				Index = StartIndex + SizeX * i;
				HighlightTile(Index);

				//-X with edge check
				Index = StartIndex - i;
				if (i <= (StartIndex % SizeX))
				{
					HighlightTile(Index);
				}

				//-Y
				Index = StartIndex - SizeX * i;
				HighlightTile(Index);
			}
		}
		break;
		//For a Diagonal
	case EPatternType::PT_Diagonal:
		{
			//Min is clamped to 1 at the lowest
			Min = FMath::Max(1, Min);
			//Max can not be lower than Min
			Max = FMath::Max(Max, Min);

			//Loop from Min to Max
			for (int i = Min; i <= Max; i++)
			{
				//+X + Y with edge check
				int Index = StartIndex + i + SizeX * i;
				if (i < (SizeX - (StartIndex % SizeX)))
				{
					HighlightTile(Index);
				}

				//-X + Y with edge check
				Index = StartIndex - i + SizeX * i;
				if (i <= (StartIndex % SizeX))
				{
					HighlightTile(Index);
				}

				//+X - Y with edge check
				Index = StartIndex + i - SizeX * i;
				if (i < (SizeX - (StartIndex % SizeX)))
				{
					HighlightTile(Index);
				}

				//-X - Y with edge check
				Index = StartIndex - i - SizeX * i;
				if (i <= (StartIndex % SizeX))
				{
					HighlightTile(Index);
				}
			}
		}
		break;
		//For a Diamond
	case EPatternType::PT_Diamond:
		{
			//If the Min is 0
			if (Min == 0)
			{
				//Highlight Start Index
				HighlightTile(StartIndex);
				//Max can not be lower than Min
				Max = FMath::Max(Max, Min);
				//Min can not be lower than 1
				Min = FMath::Max(1, Min);
			}
			else
			{
				//Otherwise, check Min and Max values
				Min = FMath::Max(1, Min);
				Max = FMath::Max(Max, Min);
			}

			//Loop from Min to Max
			for (int i = Min; i <= Max; i++)
			{
				int Index;

				//Fill in the straight line segments
				//+X with edge check
				Index = StartIndex + i;
				if (i < (SizeX - (StartIndex % SizeX)))
				{
					HighlightTile(Index);
				}

				//-X with edge check
				Index = StartIndex - i;
				if (i <= (StartIndex % SizeX))
				{
					HighlightTile(Index);
				}

				//+Y
				Index = StartIndex + SizeX * i;
				HighlightTile(Index);

				//-Y
				Index = StartIndex - SizeX * i;
				HighlightTile(Index);

				//Fill in the quadrants
				//Loop from 1 to i
				for (int j = 1; j < i; j++)
				{
					//Top Right section
					Index = StartIndex + (i - j) + (SizeX * j);

					//Edge Check
					if ((i - j) < (SizeX - (StartIndex % SizeX)))
					{
						HighlightTile(Index);
					}

					//Top Left section
					Index = StartIndex + (i - j) - (SizeX * j);

					//Edge Check
					if ((i - j) < (SizeX - (StartIndex % SizeX)))
					{
						HighlightTile(Index);
					}

					//Bottom Right section
					Index = StartIndex - (i - j) + (SizeX * j);

					//Edge Check
					if ((i-j) <= (StartIndex % SizeX))
					{
						HighlightTile(Index);
					}

					//Bottom Left
					Index = StartIndex - (i - j) - (SizeX * j);
					//Edge Check
					if ((i - j) <= (StartIndex % SizeX))
					{
						HighlightTile(Index);
					}
				}
			}
		}
		break;
		//For a Square
	case EPatternType::PT_Square:
	{
		//If Min is 0
		if (Min == 0)
		{
			//Highlight the Start
			HighlightTile(StartIndex);
			//Clamp Max to Min and Min to 1
			Max = FMath::Max(Max, Min);
			Min = FMath::Max(1, Min);
		}
		else
		{
			//Clamp Min to 1 and Max to Min
			Min = FMath::Max(1, Min);
			Max = FMath::Max(Max, Min);
		}

		//Loop from Min to Max
		for (int i = Min; i <= Max; i++)
		{
			int Index;

			//Check the Straight and Diagonal tiles
			//+X
			Index = StartIndex + i;
			//Edge Check
			if (i < (SizeX - (StartIndex % SizeX)))
			{
				HighlightTile(Index);
			}

			//-X
			Index = StartIndex - i;
			//Edge Check
			if (i <= (StartIndex % SizeX))
			{
				HighlightTile(Index);
			}

			//+Y
			Index = StartIndex + SizeX * i;
			HighlightTile(Index);

			//-Y
			Index = StartIndex - SizeX * i;
			HighlightTile(Index);

			//+X + Y
			Index = StartIndex + i + SizeX * i;

			//Edge Check
			if (i < (SizeX - (StartIndex % SizeX)))
			{
				HighlightTile(Index);
			}

			//-X + Y
			Index = StartIndex - i + SizeX * i;
			//Edge Check
			if (i <= (StartIndex % SizeX))
			{
				HighlightTile(Index);
			}

			//+X - Y
			Index = StartIndex + i - SizeX * i;
			//Edge Check
			if (i < (SizeX - (StartIndex % SizeX)))
			{
				HighlightTile(Index);
			}

			//-X - Y
			Index = StartIndex - i - SizeX * i;
			//Edge Check
			if ( i <= StartIndex % SizeX)
			{
				HighlightTile(Index);
			}

			//Loop from 1 to i to fill in the space between
			for (int j = 1; j < i; j++)
			{
				//+X Row Right (-Y)
				Index = StartIndex + i + (SizeX * j);
				//Edge Check
				if (i < (SizeX - (StartIndex % SizeX)))
				{
					HighlightTile(Index);
				}
				//+Y Column Top (+X)
				Index = StartIndex + j + (SizeX * i);
				//Edge Check
				if (j < (SizeX - (StartIndex % SizeX)))
				{
					HighlightTile(Index);
				}

				//+X Row Left (-Y)
				Index = StartIndex + i - (SizeX * j);
				//Edge Check
				if (i < (SizeX - (StartIndex % SizeX)))
				{
					HighlightTile(Index);
				}

				//-Y Column Top (+X)
				Index = StartIndex + j - (SizeX * i);
				//Edge Check
				if (j < (SizeX - (StartIndex % SizeX)))
				{
					HighlightTile(Index);
				}

				//-X Row Right (+Y)
				Index = StartIndex - i + (SizeX * j);
				//Edge Check
				if (i <= (StartIndex % SizeX))
				{
					HighlightTile(Index);
				}

				//+Y Column Bottom (-X)
				Index = StartIndex - j + (SizeX * i);
				//Edge Check
				if (j <= (StartIndex % SizeX))
				{
					HighlightTile(Index);
				}

				//-X Row Left (-Y)
				Index = StartIndex - i - (SizeX * j);
				//Edge Check
				if (i <= (StartIndex % SizeX))
				{
					HighlightTile(Index);
				}

				//-Y Column Bottom (-X)
				Index = StartIndex - j - (SizeX * i);
				//Edge Check
				if (j <= (StartIndex % SizeX))
				{
					HighlightTile(Index);
				}
			}
		}
	}
	break;
	//For a Spread
	case EPatternType::PT_Spread:
	{
		//Min can't be lower than 1, Max can't be lower than Min
		Min = FMath::Max(1, Min);
		Max = FMath::Max(Max, Min);

		//Loop from Min to Max
		for (int i = Min; i <= Max; i++)
		{
			int Index;

			//Check on the Straights
			//+X
			Index = StartIndex + i;
			//Edge Check
			if (i < (SizeX - (StartIndex % SizeX)))
			{
				HighlightTile(Index);
			}

			//+Y
			Index = StartIndex + i * SizeX;
			HighlightTile(Index);

			//-X
			Index = StartIndex - i;
			//Edge Check
			if (i <= (StartIndex % SizeX))
			{
				HighlightTile(Index);
			}

			//-Y
			Index = StartIndex - i * SizeX;
			HighlightTile(Index);
			
			//Loop from 1 to i
			for (int j = 1; j < i; j++)
			{
				//Top Row
				//Top Right
				//Check the +Y area on the +X direction
				Index = StartIndex + i + j * SizeX;
				if (i < (SizeX - (StartIndex % SizeX)))
				{
					HighlightTile(Index);
				}

				//Top Left
				//Check the -Y area on the +X direction
				Index = StartIndex + i - j * SizeX;
				if (i < (SizeX - (StartIndex % SizeX)))
				{
					HighlightTile(Index);
				}

				//Bottom Row
				//Bottom Right
				//Check the +Y area on the -X direction
				Index = StartIndex - i + j * SizeX;
				if (i <= (StartIndex % SizeX))
				{
					HighlightTile(Index);
				}

				//Bottom Left
				//Check the -Y area on the -X direction
				Index = StartIndex - i - j * SizeX;
				if (i <= (StartIndex % SizeX))
				{
					HighlightTile(Index);
				}

				//Right Column
				//Top Right
				//Check the +X area on the +Y direction
				Index = StartIndex + j + i * SizeX;
				if (j < (SizeX - (StartIndex % SizeX)))
				{
					HighlightTile(Index);
				}

				//Bottom Right
				//Check the -X area on the +Y direction
				Index = StartIndex - j + i * SizeX;
				if (j <= (StartIndex % SizeX))
				{
					HighlightTile(Index);
				}

				//Left Column
				//Top Left
				//Check the +X area on the -Y direction
				Index = StartIndex + j - i * SizeX;
				if (j < (SizeX - (StartIndex % SizeX)))
				{
					HighlightTile(Index);
				}

				//Bottom Left
				//Check the -X area on the -Y direction
				Index = StartIndex - j - i * SizeX;
				if (j <= (StartIndex % SizeX))
				{
					HighlightTile(Index);
				}
			}
		}
	}
	break;
	}
}

//Detect the Tiles that will be reached from the specified Attack Person
TArray<int> AGridManager_Editor::DetectAttackTiles(FVector CasterPos, FVector Location, EPatternType Pattern, int Min, int Max, bool FromCaster)
{
	int StartIndex;

	//If FromCaster, use the CasterPos as the Start Index
	//Otherwise use the Location
	if (FromCaster)
		StartIndex = GetIndexFromLocation(CasterPos);
	else
		StartIndex = GetIndexFromLocation(Location);

	//The Tiles that will be hit
	TArray<int> HitTiles;

	//Select the Tiles based on the Pattern used
	switch (Pattern)
	{
	//For a Line
	case EPatternType::PT_Line:
	{
		//Check if it's FromCaster or not
		if (FromCaster)
		{
			//Min can not be less than 1, Max can not be less than Min
			Min = FMath::Max(1, Min);
			Max = FMath::Max(Max, Min);
			//Get the Target Index from the Location
			int TargetIndex = GetIndexFromLocation(Location);

			int Index;

			//If TargetIndex - StartIndex > 0 then it's either +X or -Y
			if (TargetIndex - StartIndex > 0)
			{
				//+X if Target-Start < SizeX
				if (TargetIndex - StartIndex < SizeX)
				{
					//Loop from Min to Max
					for (int i = Min; i <= Max; i++)
					{
						Index = StartIndex + i;
						//Edge Check
						if (i < (SizeX - (StartIndex % SizeX)))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
						else
							break;
					}
				}//+Y
				else
				{
					//Loop from Min to Max
					for (int i = Min; i <= Max; i++)
					{
						Index = StartIndex + i * SizeX;
						if (HighlightTile(Index))
							HitTiles.Add(Index);
						else
							break;
					}
				}
			}
			else
			{
				//-X f |Target-Start| < SizeX
				if (FMath::Abs(TargetIndex - StartIndex) < SizeX)
				{
					//Loop from Min to Max
					for (int i = Min; i <= Max; i++)
					{
						Index = StartIndex - i;
						//Edge Check
						if (i <= (StartIndex % SizeX))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
						else
							break;
					}
				} //+Y otherwise
				else
				{
					for (int i = Min; i <= Max; i++)
					{
						Index = StartIndex - i * SizeX;
						if (HighlightTile(Index))
							HitTiles.Add(Index);
						else
							break;
					}
				}
			}
		}
		else
		{
			//If not FromCaster
			//Min can not be less than 1, Max can not be less than Min
			Min = FMath::Max(0, Min);
			Max = FMath::Max(Max, Min);
			//Get CasterIndex
			int CasterIndex = GetIndexFromLocation(CasterPos);

			int Index;
			//+X and +Y if Start-Caster > 0
			if (StartIndex - CasterIndex > 0)
			{
				//+X if Start-Caster < SizeX
				if (StartIndex - CasterIndex < SizeX)
				{
					//Loop from Min to Max
					for (int i = Min; i <= Max; i++)
					{
						Index = StartIndex + i;
						//Edge Check
						if (i < (SizeX - (StartIndex % SizeX)))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
						else
							break;
					}
				}//+Y
				else
				{
					//Loop from Min to Max
					for (int i = Min; i <= Max; i++)
					{
						Index = StartIndex + i * SizeX;
						if (HighlightTile(Index))
							HitTiles.Add(Index);
						else
							break;
					}
				}
			}
			else
			{
				//-X if |Start-Caster| < SizeX
				if (FMath::Abs(StartIndex - CasterIndex) < SizeX)
				{
					//Loop from Min to Max
					for (int i = Min; i <= Max; i++)
					{
						Index = StartIndex - i;
						//Edge Check
						if (i <= (StartIndex % SizeX))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
						else
							break;
					}
				} //-Y
				else
				{
					//Loop from Min to Max
					for (int i = Min; i <= Max; i++)
					{
						Index = StartIndex - i * SizeX;
						if (HighlightTile(Index))
							HitTiles.Add(Index);
						else
							break;
					}
				}
			}
		}
	}
	break;
	//For Diagonal
	case EPatternType::PT_Diagonal:
	{
		//Check if FromCaster
		if (FromCaster)
		{
			//Min can not be less than 1, Max can not be less than Min
			Min = FMath::Max(1, Min);
			Max = FMath::Max(Max, Min);
			//Set TargetIndex from Location
			int TargetIndex = GetIndexFromLocation(Location);

			//+X/+Y or -X/+Y if Target > Start
			if (TargetIndex > StartIndex)
			{
				//+X check
				if (TargetIndex > (TargetIndex / SizeX - StartIndex / SizeX) * SizeX + StartIndex)
				{
					//From Min to Max
					for (int i = Min; i <= Max; i++)
					{
						int Index = StartIndex + i + i * SizeX;
						//Edge Check
						if (i < (SizeX - (StartIndex % SizeX)))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
						else
							break;
					}
				}//-X otherwise
				else
				{
					//From Min to Max
					for (int i = Min; i <= Max; i++)
					{
						int Index = StartIndex - i + i * SizeX;
						//Edge Check
						if (i <= (StartIndex % SizeX))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
							HitTiles.Add(Index);
						}
						else
							break;
					}
				}
			}//+X/-Y or -X/-Y if otherwise
			else
			{
				//+X Check
				if (TargetIndex > StartIndex- (StartIndex / SizeX - TargetIndex / SizeX) * SizeX)
				{
					//From Min to Max
					for (int i = Min; i <= Max; i++)
					{
						int Index = StartIndex + i - i * SizeX;
						//Edge Check
						if (i < (SizeX - (StartIndex % SizeX)))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
						else
							break;
					}
				}//-X Otherwise
				else
				{
					//From Min to Max
					for (int i = Min; i <= Max; i++)
					{
						int Index = StartIndex - i - i * SizeX;
						//Edge Check
						if (i <= (StartIndex % SizeX))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
						else
							break;
					}
				}
			}
		}
		else
		{
			//Min can not be less than 1, Max can not be less than Min
			Min = FMath::Max(0, Min);
			Max = FMath::Max(Max, Min);
			//Get CasterIndex from CasterPos
			int CasterIndex = GetIndexFromLocation(CasterPos);

			//+X/+Y or -X/+Y if Start > Caster
			if (StartIndex > CasterIndex)
			{
				//+X Check
				if (StartIndex > (StartIndex / SizeX - CasterIndex / SizeX) * SizeX + CasterIndex)
				{
					//From Min to Max
					for (int i = Min; i <= Max; i++)
					{
						int Index = StartIndex + i + i * SizeX;
						//Edge Check
						if (i < (SizeX - (StartIndex % SizeX)))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
						else
							break;
					}
				}//-X otherwise
				else
				{
					//From Min to Max
					for (int i = Min; i <= Max; i++)
					{
						int Index = StartIndex - i + i * SizeX;
						//Edge Check
						if (i <= (StartIndex % SizeX))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
						else
							break;
					}
				}
			}//+X/-Y or -X/-Y
			else
			{
				//+X Check
				if (StartIndex > CasterIndex - (CasterIndex / SizeX - StartIndex / SizeX) * SizeX)
				{
					//From Min to Max
					for (int i = Min; i <= Max; i++)
					{
						int Index = StartIndex + i - i * SizeX;
						//Edge Check
						if (i < (SizeX - (StartIndex % SizeX)))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
						else
							break;
					}
				}//-X Otherwise
				else
				{
					//From Min to Max
					for (int i = Min; i <= Max; i++)
					{
						int Index = StartIndex - i - i * SizeX;
						//Edge Check
						if (i <= (StartIndex % SizeX))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
						else
							break;
					}
				}
			}
		}

	}
	break;
	//For a Diamond
	case EPatternType::PT_Diamond:
	{
		//If Min = 0
		if (Min == 0)
		{
			//Get Start tile
			if (HighlightTile(StartIndex))
				HitTiles.Add(StartIndex);

			//Max can not be less than Min, Min can not be less than 1
			Max = FMath::Max(Max, Min);
			Min = FMath::Max(1, Min);
		}
		else
		{
			//Min can't be less than 1, Max can't be less than Min
			Min = FMath::Max(1, Min);
			Max = FMath::Max(Max, Min);
		}

		//From Min to Max
		for (int i = Min; i <= Max; i++)
		{
			int Index;

			//Check Straights
			Index = StartIndex + i;

			//Edge Check
			if (i < (SizeX - (StartIndex % SizeX)))
			{
				if (HighlightTile(Index))
					HitTiles.Add(Index);
			}

			Index = StartIndex - i;

			//Edge Check
			if (i <= (StartIndex % SizeX))
			{
				if (HighlightTile(Index))
					HitTiles.Add(Index);
			}

			Index = StartIndex + SizeX * i;
			if (HighlightTile(Index))
				HitTiles.Add(Index);

			Index = StartIndex - SizeX * i;
			if (HighlightTile(Index))
				HitTiles.Add(Index);

			//From 1 to i
			for (int j = 1; j < i; j++)
			{
				//Top Right (+X/+Y)
				Index = StartIndex + (i - j) + (SizeX * j);
				//Edge Check
				if ((i - j) < (SizeX - (StartIndex % SizeX)))
				{
					if (HighlightTile(Index))
						HitTiles.Add(Index);
				}

				//Top Left (+X/-Y)
				Index = StartIndex + (i - j) - (SizeX * j);
				//Edge Check
				if ((i - j) < (SizeX - (StartIndex % SizeX)))
				{
					if (HighlightTile(Index))
						HitTiles.Add(Index);
				}

				//Bottom Right (-X/+Y)
				Index = StartIndex - (i - j) + (SizeX * j);
				//Edge Check
				if ((i - j) < (StartIndex % SizeX))
				{
					if (HighlightTile(Index))
						HitTiles.Add(Index);
				}

				//Bottom Left (-X/-Y)
				Index = StartIndex - (i - j) - (SizeX * j);
				//Edge Check
				if ((i - j) < (StartIndex % SizeX))
				{
					if (HighlightTile(Index))
						HitTiles.Add(Index);
				}
			}
		}
	}
	break;
	//For a Square
	case EPatternType::PT_Square:
	{
		//Check if Min = 0
		if (Min == 0)
		{
			//Check first tile
			if (HighlightTile(StartIndex))
				HitTiles.Add(StartIndex);

			//Max can't be less than Min, Min can't be less than 1
			Max = FMath::Max(Max, Min);
			Min = FMath::Max(1, Min);
		}
		else
		{
			//Min can't be less than 1, Max can't be less than Max
			Min = FMath::Max(1, Min);
			Max = FMath::Max(Max, Min);
		}

		//From Min to Max
		for (int i = Min; i <= Max; i++)
		{
			int Index;

			//Line and Diagonal checks
			//+X
			Index = StartIndex + i;
			//Edge Check
			if (i < (SizeX - (StartIndex % SizeX)))
			{
				if (HighlightTile(Index))
					HitTiles.Add(Index);
			}

			//-X
			Index = StartIndex - i;
			//Edge Check
			if (i <= (StartIndex % SizeX))
			{
				if (HighlightTile(Index))
					HitTiles.Add(Index);
			}

			//+Y
			Index = StartIndex + SizeX * i;
			if (HighlightTile(Index))
				HitTiles.Add(Index);

			//-Y
			Index = StartIndex - SizeX * i;
			if (HighlightTile(Index))
				HitTiles.Add(Index);

			//+X/+Y
			Index = StartIndex + i + SizeX * i;
			//Edge Check
			if (i < (SizeX - (StartIndex % SizeX)))
			{
				if (HighlightTile(Index))
					HitTiles.Add(Index);
			}

			//-X/+Y
			Index = StartIndex - i + SizeX * i;
			//Edge Check
			if (i <= (StartIndex % SizeX))
			{
				if (HighlightTile(Index))
					HitTiles.Add(Index);
			}

			//+X/-Y
			Index = StartIndex + i - SizeX * i;
			//Edge Check
			if (i < (SizeX - (StartIndex % SizeX)))
			{
				if (HighlightTile(Index))
					HitTiles.Add(Index);
			}

			//-X/-Y
			Index = StartIndex - i - SizeX * i;
			//Edge Check
			if (i <= (StartIndex % SizeX))
			{
				if (HighlightTile(Index))
					HitTiles.Add(Index);
			}

			//From 1 to i
			for (int j = 1; j < i; j++)
			{
				//Top Row Right (+X/+Y)
				Index = StartIndex + i + (SizeX * j);
				//Edge Check
				if (i < (SizeX - (StartIndex % SizeX)))
				{
					if (HighlightTile(Index))
						HitTiles.Add(Index);
				}
				//Right Column Top (+X/+Y)
				Index = StartIndex + j + (SizeX * i);
				//Edge Check
				if (j < (SizeX - (StartIndex % SizeX)))
				{
					if (HighlightTile(Index))
						HitTiles.Add(Index);
				}

				//Top Row Left (+X/-Y)
				Index = StartIndex + i - (SizeX * j);
				//Edge Check
				if (i < (SizeX - (StartIndex % SizeX)))
				{
					if (HighlightTile(Index))
						HitTiles.Add(Index);
				}
				//Left Column Top (+X/-Y)
				Index = StartIndex + j - (SizeX * i);
				//Edge Check
				if (j < (SizeX - (StartIndex % SizeX)))
				{
					if (HighlightTile(Index))
						HitTiles.Add(Index);
				}

				//Bottom Row Right (-X/+Y)
				Index = StartIndex - i + (SizeX * j);
				//Edge Check
				if (i <= (StartIndex % SizeX))
				{
					if (HighlightTile(Index))
						HitTiles.Add(Index);
				}
				//Right Column Bottom (-X/+Y)
				Index = StartIndex - j + (SizeX * i);
				if (j <= (StartIndex % SizeX))
				{
					if (HighlightTile(Index))
						HitTiles.Add(Index);
				}

				//Bottom Row Left (-X/-Y)
				Index = StartIndex - i - (SizeX * j);
				//Edge Check
				if (i <= (StartIndex % SizeX))
				{
					if (HighlightTile(Index))
						HitTiles.Add(Index);
				}
				//Left Column Bottom (-X/-Y)
				Index = StartIndex - j - (SizeX * i);
				//Edge Check
				if (j <= (StartIndex % SizeX))
				{
					if (HighlightTile(Index))
						HitTiles.Add(Index);
				}
			}
		}
	}
	break;
	//For a Spread
	case EPatternType::PT_Spread:
	{
		//If FromCaster
		if (FromCaster)
		{
			//Min clamps to 1, Max clamps to 1
			Min = FMath::Max(1, Min);
			Max = FMath::Max(Max, Min);
			//Get TargetIndex from Location
			int TargetIndex = GetIndexFromLocation(Location);

			//+X if the angle from (Loc-Caster) to +X is less than 45
			if (FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FVector::ForwardVector, (Location - CasterPos).GetSafeNormal()))) < 45.0f)
			{
				//From Min to Max
				for (int i = Min; i <= Max; i++)
				{
					int Index;

					//+X
					Index = StartIndex + i;
					//Edge Check
					if (i < (SizeX - (StartIndex % SizeX)))
					{
						if (HighlightTile(Index))
							HitTiles.Add(Index);
					}

					//From 1 to i
					for (int j = 1; j < i; j++)
					{
						//Top Row
						//Top Right (+Y)
						Index = StartIndex + i + j * SizeX;
						if (i < (SizeX - (StartIndex % SizeX)))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}

						//Top Left (-Y)
						Index = StartIndex + i - j * SizeX;
						if (i < (SizeX - (StartIndex % SizeX)))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
					}
				}
			}
			//Bottom if the Angle is > 135
			else if (FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FVector::ForwardVector, (Location - CasterPos).GetSafeNormal()))) > 135.0f)
			{
				//From Min to Max
				for (int i = Min; i <= Max; i++)
				{
					int Index;

					//-X
					Index = StartIndex - i;
					//Edge Check
					if (i <= (StartIndex % SizeX))
					{
						if (HighlightTile(Index))
							HitTiles.Add(Index);
					}

					//From 1 to i
					for (int j = 1; j < i; j++)
					{
						//Bottom Row
						//Bottom Right (+Y)
						Index = StartIndex - i + j * SizeX;
						if (i <= (StartIndex % SizeX))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}

						//Bottom Left (-Y)
						Index = StartIndex - i - j * SizeX;
						if (i <= (StartIndex % SizeX))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
					}
				}
			}//Otherwise it's Left or Right
			else
			{
				//+Y if Target-Start > 0
				if (TargetIndex - StartIndex > 0)
				{
					//From Min to Max
					for (int i = Min; i <= Max; i++)
					{
						int Index;

						//+Y
						Index = StartIndex + i * SizeX;
						if (HighlightTile(Index))
							HitTiles.Add(Index);

						//From 1 to i
						for (int j = 1; j < i; j++)
						{

							//Right Column
							//Top Right (+X)
							Index = StartIndex + j + i * SizeX;
							//Edge Check
							if (j < (SizeX - (StartIndex % SizeX)))
							{
								if (HighlightTile(Index))
									HitTiles.Add(Index);
							}

							//Bottom Right (-X)
							Index = StartIndex - j + i * SizeX;
							//Edge Check
							if (j <= (StartIndex % SizeX))
							{
								if (HighlightTile(Index))
									HitTiles.Add(Index);
							}
						}
					}
				} //Left
				else
				{
					//From Min to Max
					for (int i = Min; i <= Max; i++)
					{
						int Index;

						//-Y
						Index = StartIndex - i * SizeX;
						if (HighlightTile(Index))
							HitTiles.Add(Index);

						for (int j = 1; j < i; j++)
						{
							//Left Column
							//Top Left (+X)
							Index = StartIndex + j - i * SizeX;
							//Edge Check
							if (j < (SizeX - (StartIndex % SizeX)))
							{
								if (HighlightTile(Index))
									HitTiles.Add(Index);
							}

							//Bottom Left (-X)
							Index = StartIndex - j - i * SizeX;
							//Edge Check
							if (j <= (StartIndex % SizeX))
							{
								if (HighlightTile(Index))
									HitTiles.Add(Index);
							}
						}
					}
				}
			}
		}
		else
		{
			//Get Caster Index from CasterPos
			int CasterIndex = GetIndexFromLocation(CasterPos);
			//Min can not be less than 0, Max can not be less than Min
			Min = FMath::Max(0, Min);
			Max = FMath::Max(Max, Min);

			//+X direction (using the angle between Loc-Caster and +X)
			if (FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FVector::ForwardVector, (Location - CasterPos).GetSafeNormal()))) < 45.0f)
			{
				//From Min to Max
				for (int i = Min; i <= Max; i++)
				{
					int Index;

					//+X
					Index = StartIndex + i;
					//Edge Check
					if (i < (SizeX - (StartIndex % SizeX)))
					{
						if (HighlightTile(Index))
							HitTiles.Add(Index);
					}

					//From 1 to i
					for (int j = 1; j <= i; j++)
					{
						//Top Row
						//Top Right (+Y)
						Index = StartIndex + i + j * SizeX;
						if (i < (SizeX - (StartIndex % SizeX)))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}

						//Top Left (-Y)
						Index = StartIndex + i - j * SizeX;
						if (i < (SizeX - (StartIndex % SizeX)))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
					}
				}
			}
			//-X direction (using the angle between Loc-Caster and +X)
			else if (FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FVector::ForwardVector, (Location-CasterPos).GetSafeNormal()))) > 135.0f)
			{
				//From Min to Max
				for (int i = Min; i <= Max; i++)
				{
					int Index;

					//-X
					Index = StartIndex - i;
					//Edge Check
					if (i <= (StartIndex % SizeX))
					{
						if (HighlightTile(Index))
							HitTiles.Add(Index);
					}

					for (int j = 1; j <= i; j++)
					{
						//Bottom Row
						//Bottom Right (+Y)
						Index = StartIndex - i + j * SizeX;
						if (i <= (StartIndex % SizeX))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}

						//Bottom Left (-Y)
						Index = StartIndex - i - j * SizeX;
						if (i <= (StartIndex % SizeX))
						{
							if (HighlightTile(Index))
								HitTiles.Add(Index);
						}
					}
				}
			}
			else
			{
				//+Y if Start-Caster > 0
				if (StartIndex - CasterIndex > 0)
				{
					//From Min to Max
					for (int i = Min; i <= Max; i++)
					{
						int Index;

						//+Y
						Index = StartIndex + i * SizeX;
						if (HighlightTile(Index))
							HitTiles.Add(Index);

						//From j to i
						for (int j = 1; j <= i; j++)
						{

							//Right Column
							//Top Right
							Index = StartIndex + j + i * SizeX;
							//Edge Check
							if (j < (SizeX - (StartIndex % SizeX)))
							{
								if (HighlightTile(Index))
									HitTiles.Add(Index);
							}

							//Bottom Right
							Index = StartIndex - j + i * SizeX;
							//Edge Check
							if (j <= (StartIndex % SizeX))
							{
								if (HighlightTile(Index))
									HitTiles.Add(Index);
							}
						}
					}
				} //Left
				else
				{
					//From Min to Max
					for (int i = Min; i <= Max; i++)
					{
						int Index;

						//-Y
						Index = StartIndex - i * SizeX;
						if (HighlightTile(Index))
							HitTiles.Add(Index);

						//From j to i
						for (int j = 1; j <= i; j++)
						{
							//Left Column
							//Top Left
							Index = StartIndex + j - i * SizeX;
							//Edge Check
							if (j < (SizeX - (StartIndex % SizeX)))
							{
								if (HighlightTile(Index))
									HitTiles.Add(Index);
							}

							//Bottom Left
							Index = StartIndex - j - i * SizeX;
							//Edge Check
							if (j <= (StartIndex % SizeX))
							{
								if (HighlightTile(Index))
									HitTiles.Add(Index);
							}
						}
					}
				}
			}
		}
	}
	break;
	}

	//Return the Tiles that were Hit
	return HitTiles;
}

//Clear the Highlight instances
void AGridManager_Editor::ClearHighlights()
{
	//Clear existing Tile Highlights
	TileHighlights->ClearInstances();

	//All tiles are no longer targetable
	for (int i = 0; i < Grid.Num(); i++)
	{
		Grid[i].bIsTargetable = false;
	}
}

//Check if the Tile at Location is Targetable
bool AGridManager_Editor::IsTileTargetable(FVector Location)
{
	//Get the Relative Location
	FVector RelativeLoc = Location - this->GetActorLocation();

	//Find the Index
	int XVal = (int)(FMath::TruncToFloat(RelativeLoc.X) / TileSize);

	if (((int)RelativeLoc.X % (int)TileSize) > (TileSize * 0.5f))
	{
		XVal++;
	}

	int YVal = (int)(FMath::TruncToFloat(RelativeLoc.Y) / TileSize);

	if (((int)RelativeLoc.Y % (int)TileSize) > (TileSize * 0.5f))
	{
		YVal++;
	}

	//Return the Index's Targetability if valid
	if ((XVal + (SizeX * YVal)) < Grid.Num())
		return Grid[(XVal + (SizeX * YVal))].bIsTargetable;
	else
		return false;
}

//Try to add a Highlight at the Index, return true if successful
bool AGridManager_Editor::HighlightTile(int Index)
{
	//Check that the Index is in range
	if (Index > -1 && Index < Grid.Num())
	{
		//Get the Index Transform
		FTransform Transform;
		GridMesh->GetInstanceTransform(Index, Transform);
		//Make the Index Targetable
		Grid[Index].bIsTargetable = true;
		//Add an Instance to TileHighlights
		TileHighlights->AddInstance(Transform);
		//Return success
		return true;
	}

	//Return false
	return false;
}