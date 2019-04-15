// Fill out your copyright notice in the Description page of Project Settings.

#include "TM_Height.h"
#include "Actors/Systems/GridEditor/GridManager_Editor.h"
#include "Actors/Systems/GridEditor/TM_Ramp.h"
#include "Engine.h"

ATM_Height::ATM_Height()
{
	//Create a Box Component
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComponent);

	//Set Collision Response for Grid Modifier
	Box->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);
}

//Handle OnConstruction
void ATM_Height::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	//Set Score to the Current Height
	Score = (int)Height;

	FVector Start = Transform.GetLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000.0f);

	FHitResult Hit;

	//Perform a Ray Trace to find Grid underneath
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
	{
		//If a Grid is found
		if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.GetActor()))
		{
			//Set the Box Location to use the Grid Location
			FVector BoxLocation = Grid->GetClosestTileLocation(Hit.Location);
			BoxLocation.Z = Grid->GetActorLocation().Z + Height * 0.5f;
			Box->SetWorldLocation(BoxLocation);

			Box->SetBoxExtent(FVector(Grid->TileSize * 0.5f, Grid->TileSize * 0.5f, Height * 0.5f));

			//If the CurrentIndex does not match the new one
			if (CurrentIndex != Grid->GetIndexFromLocation(Hit.Location))
			{
				TArray<FHitResult> Hits;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				FVector PreviousStart = Grid->GetLocationFromIndex(CurrentIndex) + FVector(0.0f, 0.0f, 90000.0f);
				FVector PreviousEnd = PreviousStart - FVector(0.0f, 0.0f, 180000.0f);

				//Perform a ray trace to find another Modifier on the previous index
				if (GetWorld()->LineTraceMultiByChannel(Hits, PreviousStart, PreviousEnd, ECC_GameTraceChannel2, Params))
				{
					int HighestScore = -1;
					int HighestRef = -1;

					//Loop through all the Modifiers
					for (int i = 0; i < Hits.Num(); i++)
					{
						if (ATileModifier* TileMod = Cast<ATileModifier>(Hits[i].Actor))
						{
							//Find the Modifier with the highest score
							if (TileMod->Score > HighestScore)
							{
								HighestScore = TileMod->Score;
								HighestRef = i;
							}

							//If one is found that outscores this Modifier, set to -1 and break (this one was already ignored)
							if (TileMod->Score > Score)
							{
								HighestRef = -1;
								break;
							}
						}
					}

					//If a second highest score was found
					if (HighestRef != -1)
					{
						//Cast to Height or Ramp
						if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
						{
							//Update the Index Transform
							Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
						}
						else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
						{
							//Update the Index Transform
							Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
						}
					}
				}
				else
				{
					//If no modifiers were found, return to default
					Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), 0.0f);
				}

				//Update the Current Index
				CurrentIndex = Grid->GetIndexFromLocation(Hit.Location);

				//Check for other modifiers on the Current Index
				if (GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECC_GameTraceChannel2, Params))
				{
					int HighestScore = -1;
					int HighestRef = -1;

					//Find the Highest Score
					for (int i = 0; i < Hits.Num(); i++)
					{
						if (ATileModifier* TileMod = Cast<ATileModifier>(Hits[i].Actor))
						{
							if (TileMod->Score > HighestScore)
							{
								HighestScore = TileMod->Score;
								HighestRef = i;
							}
						}
					}

					//If no Score is higher
					if (HighestScore < Score)
					{
						//Update Transform using this Modifier
						Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), Height);
					}
					else
					{
						//Cast to Height or Ramp
						if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
						{
							//Update Transform
							Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
						}
						else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
						{
							//Update Transform
							Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
						}
					}
				}
				else
				{
					//If no modifiers are found, update using this Modifier
					Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), Height);
				}
			}
			else
			{
				//If Current Index is this one
				TArray<FHitResult> Hits;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				//Check for other modifiers on the Current Index
				if (GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECC_GameTraceChannel2, Params))
				{
					int HighestScore = -1;
					int HighestRef = -1;

					//Find the Highest Score
					for (int i = 0; i < Hits.Num(); i++)
					{
						if (ATileModifier* TileMod = Cast<ATileModifier>(Hits[i].Actor))
						{
							if (TileMod->Score > HighestScore)
							{
								HighestScore = TileMod->Score;
								HighestRef = i;
							}
						}
					}

					//If no Score is higher
					if (HighestScore < Score)
					{
						//Update Transform using this Modifier
						Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), Height);
					}
					else
					{
						//Cast to Height or Ramp
						if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
						{
							//Update Transform
							Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
						}
						else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
						{
							//Update Transform
							Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
						}
					}
				}
				else
				{
					//If no modifiers are found, update using this Modifier
					Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), Height);
				}
			}
		}
	}
}

//Override Destroyed to remove influence
void ATM_Height::Destroyed()
{
	//Disable Collision
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FVector Start = this->GetActorLocation() + FVector(0.0f, 0.0f, Height*2.0f);
	FVector End = Start - FVector(0.0f, 0.0f, Height * 4.0f);

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	//Ray trace to find other modifiers
	if (GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECC_GameTraceChannel2, Params))
	{
		int HighestScore = -1;
		int HighestRef = -1;

		//Find the Highest Score
		for (int i = 0; i < Hits.Num(); i++)
		{
			if (ATileModifier* TileMod = Cast<ATileModifier>(Hits[i].Actor))
			{
				if (TileMod->Score > HighestScore)
				{
					HighestScore = TileMod->Score;
					HighestRef = i;
				}
			}
		}

		//If Highest Score is -1, use defaults
		if (HighestScore == -1)
		{
			FHitResult Hit;

			if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
			{
				if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.GetActor()))
				{
					Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), 0.0f);
				}
			}
		}
		//If Highest Score < Score, otherwise do nothing
		else if (HighestScore < Score)
		{
			FHitResult Hit;

			if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
			{
				if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.GetActor()))
				{
					//Cast to Height or Ramp
					if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
					{
						//Update Transform
						Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
					}
					else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
					{
						//Update Transform
						Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
					}
				}
			}
		}
	}
	else
	{
		//If no hits found
		FHitResult Hit;

		//Set to default
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
		{
			if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.GetActor()))
			{
				Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), 0.0f);
			}
		}
	}

	Super::Destroyed();

}

void ATM_Height::UpdateModifier()
{
	//Set Score to the Current Height
	Score = (int)Height;

	FVector Start = this->GetActorLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000.0f);

	FHitResult Hit;

	//Perform a Ray Trace to find Grid underneath
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
	{
		//If a Grid is found
		if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.GetActor()))
		{
			//Set the Box Location to use the Grid Location
			FVector BoxLocation = Grid->GetClosestTileLocation(Hit.Location);
			BoxLocation.Z = Grid->GetActorLocation().Z + Height * 0.5f;
			Box->SetWorldLocation(BoxLocation);

			Box->SetBoxExtent(FVector(Grid->TileSize * 0.5f, Grid->TileSize * 0.5f, Height * 0.5f));

			//If the CurrentIndex does not match the new one
			if (CurrentIndex != Grid->GetIndexFromLocation(Hit.Location))
			{
				TArray<FHitResult> Hits;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				FVector PreviousStart = Grid->GetLocationFromIndex(CurrentIndex) + FVector(0.0f, 0.0f, 90000.0f);
				FVector PreviousEnd = PreviousStart - FVector(0.0f, 0.0f, 180000.0f);

				//Perform a ray trace to find another Modifier on the previous index
				if (GetWorld()->LineTraceMultiByChannel(Hits, PreviousStart, PreviousEnd, ECC_GameTraceChannel2, Params))
				{
					int HighestScore = -1;
					int HighestRef = -1;

					//Loop through all the Modifiers
					for (int i = 0; i < Hits.Num(); i++)
					{
						if (ATileModifier* TileMod = Cast<ATileModifier>(Hits[i].Actor))
						{
							//Find the Modifier with the highest score
							if (TileMod->Score > HighestScore)
							{
								HighestScore = TileMod->Score;
								HighestRef = i;
							}

							//If one is found that outscores this Modifier, set to -1 and break (this one was already ignored)
							if (TileMod->Score > Score)
							{
								HighestRef = -1;
								break;
							}
						}
					}

					//If a second highest score was found
					if (HighestRef != -1)
					{
						//Cast to Height or Ramp
						if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
						{
							//Update the Index Transform
							Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
						}
						else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
						{
							//Update the Index Transform
							Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
						}
					}
				}
				else
				{
					//If no modifiers were found, return to default
					Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), 0.0f);
				}

				//Update the Current Index
				CurrentIndex = Grid->GetIndexFromLocation(Hit.Location);

				//Check for other modifiers on the Current Index
				if (GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECC_GameTraceChannel2, Params))
				{
					int HighestScore = -1;
					int HighestRef = -1;

					//Find the Highest Score
					for (int i = 0; i < Hits.Num(); i++)
					{
						if (ATileModifier* TileMod = Cast<ATileModifier>(Hits[i].Actor))
						{
							if (TileMod->Score > HighestScore)
							{
								HighestScore = TileMod->Score;
								HighestRef = i;
							}
						}
					}

					//If no Score is higher
					if (HighestScore < Score)
					{
						//Update Transform using this Modifier
						Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), Height);
					}
					else
					{
						//Cast to Height or Ramp
						if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
						{
							//Update Transform
							Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
						}
						else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
						{
							//Update Transform
							Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
						}
					}
				}
				else
				{
					//If no modifiers are found, update using this Modifier
					Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), Height);
				}
			}
			else
			{
				//If Current Index is this one
				TArray<FHitResult> Hits;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				//Check for other modifiers on the Current Index
				if (GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECC_GameTraceChannel2, Params))
				{
					int HighestScore = -1;
					int HighestRef = -1;

					//Find the Highest Score
					for (int i = 0; i < Hits.Num(); i++)
					{
						if (ATileModifier* TileMod = Cast<ATileModifier>(Hits[i].Actor))
						{
							if (TileMod->Score > HighestScore)
							{
								HighestScore = TileMod->Score;
								HighestRef = i;
							}
						}
					}

					//If no Score is higher
					if (HighestScore < Score)
					{
						//Update Transform using this Modifier
						Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), Height);
					}
					else
					{
						//Cast to Height or Ramp
						if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
						{
							//Update Transform
							Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
						}
						else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
						{
							//Update Transform
							Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
						}
					}
				}
				else
				{
					//If no modifiers are found, update using this Modifier
					Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), Height);
				}
			}
		}
	}
}

void ATM_Height::ClampToNeighbour(int Index)
{
	FVector Start = this->GetActorLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000.0f);

	FHitResult Hit;

	//Perform a Ray Trace to find Grid underneath
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
	{
		//If a Grid is found
		if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.GetActor()))
		{
			if (Index > -1 && Index < Grid->Grid.Num() && CurrentIndex > -1 && CurrentIndex < Grid->Grid.Num()
				&& (FMath::Abs(Index - CurrentIndex) == 1 || FMath::Abs(Index - CurrentIndex) == Grid->SizeX))
			{
				if (Grid->Grid[Index].Rotation.Pitch != 0.0f && FMath::Abs(Index - CurrentIndex) == 1)
				{
					float Pitch = Grid->Grid[Index].Rotation.Pitch;
					float HeightDifference = (FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Pitch))) * Grid->TileSize) * 0.5f;

					if (Pitch > 0)
					{
						//Tilted so high is +X and low is -X
						if (Index - CurrentIndex == 1)
						{
							//CurrentIndex is -X
							Height = Grid->Grid[Index].Position.Z - HeightDifference;
							UpdateModifier();
						}
						else if (Index - CurrentIndex == -1)
						{
							//Current Index is +X
							Height = Grid->Grid[Index].Position.Z + HeightDifference;
							UpdateModifier();
						}
					}
					else if (Pitch < 0)
					{
						//Tilted so high is -X and low is +X
						if (Index - CurrentIndex == 1)
						{
							//CurrentIndex is -X
							Height = Grid->Grid[Index].Position.Z + HeightDifference;
							UpdateModifier();
						}
						else if (Index - CurrentIndex == -1)
						{
							//Current Index is +X
							Height = Grid->Grid[Index].Position.Z - HeightDifference;
							UpdateModifier();
						}
					}
				}
				else if (Grid->Grid[Index].Rotation.Roll != 0.0f && FMath::Abs(Index - CurrentIndex) == Grid->SizeX)
				{
					float Roll = Grid->Grid[Index].Rotation.Roll;
					float HeightDifference = (FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Roll))) * Grid->TileSize) * 0.5f;

					if (Roll > 0)
					{
						//Tilted so high is -Y and low is +Y
						if (Index - CurrentIndex == Grid->SizeX)
						{
							//CurrentIndex is -Y
							Height = Grid->Grid[Index].Position.Z + HeightDifference;
							UpdateModifier();
						}
						else if (Index - CurrentIndex == -Grid->SizeX)
						{
							//Current Index is +Y
							Height = Grid->Grid[Index].Position.Z - HeightDifference;
							UpdateModifier();
						}
					}
					else if (Roll < 0)
					{
						//Tilted so high is +Y and low is -Y
						if (Index - CurrentIndex == Grid->SizeX)
						{
							//CurrentIndex is -Y
							Height = Grid->Grid[Index].Position.Z - HeightDifference;
							UpdateModifier();
						}
						else if (Index - CurrentIndex == -Grid->SizeX)
						{
							//Current Index is +Y
							Height = Grid->Grid[Index].Position.Z + HeightDifference;
							UpdateModifier();
						}
					}
				}
				else
				{
					Height = Grid->Grid[Index].Position.Z;
					UpdateModifier();
				}
			}
		}
	}
}