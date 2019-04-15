// Fill out your copyright notice in the Description page of Project Settings.

#include "GridManager.h"
#include "DeckGM.h"

// Sets default values
AGridManager::AGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create the Grid Mesh as the RootComponent
	GridMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Grid Mesh"));
	RootComponent = GridMesh;

	//Load a default mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh>GridMeshAsset(TEXT("StaticMesh'/Game/Art/Systems/GridBase.GridBase'"));
	UStaticMesh* GridAsset = GridMeshAsset.Object;
	GridMesh->SetStaticMesh(GridAsset);

	//Set Collision response to ECC_GameTraceChannel1 (Grid)
	GridMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Block);

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

// Called when the game starts or when spawned
void AGridManager::BeginPlay()
{
	Super::BeginPlay();

	if (ADeckGM* GM = Cast<ADeckGM>(GetWorld()->GetAuthGameMode()))
	{
		GM->GridRef = this;
	}
}

// Called every frame
void AGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//Import a board from a file
void AGridManager::ImportBoard()
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
			FGraphNode NewNode;
			//Split the current line into the appropriate data
			FRotator NodeRotation = FRotator(0.0f);
			TArray<FString> SampleLines;
			Lines[i].ParseIntoArray(SampleLines, TEXT("|"));

			//Check that i+1 doesn't go out of bounds
			if (i + 1 < Lines.Num())
			{
				//The correct data should break into 7 parts
				if (SampleLines.Num() == 7)
				{
					//Create defaults for Index, IndexPosition
					int Index = -1;
					FVector IndexPosition = FVector(0.0f);

					//Set Index
					Index = FCString::Atoi(*SampleLines[0]);
					//Set Position
					IndexPosition.X = FCString::Atof(*SampleLines[1]);
					IndexPosition.Y = FCString::Atof(*SampleLines[2]);
					IndexPosition.Z = FCString::Atof(*SampleLines[3]);
					//Set Rotation
					NodeRotation.Pitch = FCString::Atof(*SampleLines[4]);
					NodeRotation.Yaw = FCString::Atof(*SampleLines[5]);
					NodeRotation.Roll = FCString::Atof(*SampleLines[6]);

					//Set the Node's Position
					NewNode.Position = IndexPosition;
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
							ConnectedScore = FCString::Atoi(*SampleLines[j + 1]);

							//Set and add the new Connection
							NewConnection.Index = ConnectedIndex;
							NewConnection.Score = ConnectedScore;
							NewNode.Connections.Add(NewConnection);
						}
					}
				}

				//Set the Node's Scale to 0
				FVector NodeScale = FVector(0.0f);

				{
					//Loop through the Node's connections while the Scale is still 0
					for (int j = 0; j < NewNode.Connections.Num() && NodeScale == FVector(0.0f); j++)
					{
						if (NewNode.Connections[j].Score != -1)
							NodeScale = FVector(TileSize);
					}

					//If the Scale is TileSize and the Tile has a rotation to it
					if (NodeScale == FVector(TileSize) && NodeRotation != FRotator(0.0f))
					{
						//Check if the Rotation is Pitch or Roll
						if (NodeRotation.Pitch != 0.0f)
						{
							//Calculate a modified Scale
							float ScaleModifier = (TileSize / FMath::Cos(FMath::DegreesToRadians(NodeRotation.Pitch))) / TileSize;
							NodeScale = FVector(TileSize * ScaleModifier, TileSize, TileSize);
						}
						else if (NodeRotation.Roll != 0.0f)
						{
							//Calculate a modified Scale
							float ScaleModifier = (TileSize / FMath::Cos(FMath::DegreesToRadians(NodeRotation.Roll))) / TileSize;
							NodeScale = FVector(TileSize, TileSize * ScaleModifier, TileSize);
						}
					}
				}

				//Add the Node
				Grid.Add(NewNode);
				//Create an Instance on the GridMesh using the Tile's data
				GridMesh->AddInstance(FTransform(NodeRotation, NewNode.Position, NodeScale));
			}
		}

		//Commented out for later use proper version of the TM_Wall class for gameplay
		
		//Start looping after the Tile data
		int LineCount = 3 + (SizeX * SizeY) * 2;
		while (LineCount < Lines.Num())
		{
			//Parse each line using |
			TArray<FString> SampleLines;
			Lines[LineCount].ParseIntoArray(SampleLines, TEXT("|"));

			//Wall data should result in 7 elements
			if (SampleLines.Num() == 7)
			{
				//Get the Index to use
				int Index = FCString::Atoi(*SampleLines[0]);

				//Check that the Index is valid
				if (Index > -1 && Index < Grid.Num())
				{
					//Get the Height and Thickness
					float Height = FCString::Atof(*SampleLines[1]);
					float Thickness = FCString::Atof(*SampleLines[2]);
					uint8 DirectionFlags = 0;

					//Check for the 4 Directional flags
					for (int32 i = 0; i <= 3; ++i)
					{
						if (3 + i < SampleLines.Num())
						{
							if (FCString::Atoi(*SampleLines[3 + i]) == 1)
							{
								DirectionFlags |= (1 << i);
							}
						}
					}

					//If the WallType is valid
					if (WallType)
					{
						//Spawn a Wall of the appropriate Wall Type
						AGridWall* WallMod = GetWorld()->SpawnActor<AGridWall>(WallType->GetDefaultObject()->GetClass(), FVector(Grid[Index].Position.X, Grid[Index].Position.Y, Grid[Index].Position.Z) + this->GetActorLocation(), FRotator(0.0f));
						//Create the Walls
						WallMod->CreateWalls(TileSize, Height, Thickness, DirectionFlags);
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("ERROR WITH WALL AT INDEX: %d"), Index);
				}

			}

			LineCount++;
		}
	}
}

//Find the Index closest to the specified Location, returns -1 on fail
int AGridManager::GetIndexFromLocation(FVector Location)
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
FVector AGridManager::GetLocationFromIndex(int Index)
{
	//Check that the Index is within bounds, return the World location, otherwise return 0
	if (Index > -1 && Index < Grid.Num())
		return this->GetActorLocation() + Grid[Index].Position;
	else
		return FVector(0.0f);
}

//Get the closest Index Location to the specified Location
FVector AGridManager::GetClosestTileLocation(FVector Location)
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

//Heuristic Estimate for pathfinding
int AGridManager::HeuristicEstimate(int StartIndex, int Goal)
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
TArray<int> AGridManager::ReconstructPath(TArray<SearchNode> Map, int Current)
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
TArray<int> AGridManager::FindPath(FVector Start, FVector End, float MaxHeight)
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
				if (!Grid[Grid[CurrentNode].Connections[i].Index].bIsOccupied && Grid[CurrentNode].Connections[i].Score != -1 && Grid[CurrentNode].Connections[i].Score <= (int)MaxHeight)
				{
					//Make a temp score
					//int tempGScore = Map[CurrentNode].gScore + Grid[CurrentNode].Connections[i].Score;
					int tempGScore = Map[CurrentNode].gScore + 1;

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
void AGridManager::HighlightMovement(FVector Start, int MovementRange, float MaxHeight)
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
				if (!Grid[Grid[CurrentNode].Connections[j].Index].bIsOccupied && Grid[CurrentNode].Connections[j].Score != -1 && Grid[CurrentNode].Connections[j].Score <= (int)MaxHeight)
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
void AGridManager::HighlightAttackPlacement(FVector Start, EPatternType Pattern, int Min, int Max)
{
	//Get the Start Index from the Location
	int StartIndex = GetIndexFromLocation(Start);

	//Switch based on the Pattern used
	switch (Pattern)
	{
		//For a Line
	case EPatternType::PT_Line:
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
				if ((i - j) <= (StartIndex % SizeX))
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
			if (i <= StartIndex % SizeX)
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
TArray<int> AGridManager::DetectAttackTiles(FVector CasterPos, FVector Location, EPatternType Pattern, int Min, int Max, bool FromCaster)
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
				if (TargetIndex > StartIndex - (StartIndex / SizeX - TargetIndex / SizeX) * SizeX)
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
void AGridManager::ClearHighlights()
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
bool AGridManager::IsTileTargetable(FVector Location)
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
bool AGridManager::HighlightTile(int Index)
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