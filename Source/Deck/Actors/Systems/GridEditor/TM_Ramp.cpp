// Fill out your copyright notice in the Description page of Project Settings.

#include "TM_Ramp.h"
#include "Actors/Systems/GridEditor/GridManager_Editor.h"
#include "Actors/Systems/GridEditor/TM_Height.h"
#include "Engine.h"

ATM_Ramp::ATM_Ramp()
{
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComponent);

	Box->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);
}

void ATM_Ramp::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	Score = (int)MidHeight;

	FVector Start = Transform.GetLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000.0f);

	FHitResult Hit;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
	{
		if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.GetActor()))
		{
			MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
			MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;

			FVector BoxLocation = Grid->GetClosestTileLocation(Hit.Location);
			BoxLocation.Z = Grid->GetActorLocation().Z + MaxHeight * 0.5f;
			Box->SetWorldLocation(BoxLocation);

			Box->SetBoxExtent(FVector(Grid->TileSize * 0.5f, Grid->TileSize * 0.5f, MaxHeight * 0.5f));
			
			if (CurrentIndex != Grid->GetIndexFromLocation(Hit.Location))
			{
				TArray<FHitResult> Hits;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				FVector PreviousStart = Grid->GetLocationFromIndex(CurrentIndex) + FVector(0.0f, 0.0f, 90000.0f);
				FVector PreviousEnd = PreviousStart - FVector(0.0f, 0.0f, 180000.0f);

				if (GetWorld()->LineTraceMultiByChannel(Hits, PreviousStart, PreviousEnd, ECC_GameTraceChannel2, Params))
				{
					int HighestScore = -1;
					int HighestRef = -1;

					for (int i = 0; i < Hits.Num(); i++)
					{
						if (ATileModifier* TileMod = Cast<ATileModifier>(Hits[i].Actor))
						{
							if (TileMod->Score > HighestScore)
							{
								HighestScore = TileMod->Score;
								HighestRef = i;
							}

							if (TileMod->Score > Score)
							{
								HighestRef = -1;
								break;
							}
						}
					}

					if (HighestRef != -1)
					{
						//Cast to Height or Ramp
						if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
						{
							//Check for Ramp Angle
							Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
						}
						else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
						{
							//Check for Ramp Angle
							Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
						}
					}
				}
				else
				{
					Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), 0.0f);
				}

				CurrentIndex = Grid->GetIndexFromLocation(Hit.Location);

				if (GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECC_GameTraceChannel2, Params))
				{
					int HighestScore = -1;
					int HighestRef = -1;

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

					if (HighestScore < Score)
					{
						Grid->UpdateIndexTransform(CurrentIndex, GetRotation(), MidHeight);
					}
					else
					{
						//Cast to Height or Ramp
						if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
						{
							//Check for Ramp Angle
							Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
						}
						else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
						{
							//Check for Ramp Angle
							Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
						}
					}
				}
				else
				{
					Grid->UpdateIndexTransform(CurrentIndex, GetRotation(), MidHeight, GetScaleAdjustment(Grid->TileSize));
				}
			}
			else
			{
				TArray<FHitResult> Hits;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				if (GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECC_GameTraceChannel2, Params))
				{
					int HighestScore = -1;
					int HighestRef = -1;

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

					if (HighestScore < Score)
					{
						Grid->UpdateIndexTransform(CurrentIndex, GetRotation(), MidHeight, GetScaleAdjustment(Grid->TileSize));
					}
					else
					{
						//Cast to Height or Ramp
						if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
						{
							//Check for Ramp Angle
							Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
						}
						else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
						{
							//Check for Ramp Angle
							Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
						}
					}
				}
				else
				{
					Grid->UpdateIndexTransform(CurrentIndex, GetRotation(), MidHeight, GetScaleAdjustment(Grid->TileSize));
				}
			}
		}
	}
}

void ATM_Ramp::Destroyed()
{
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FVector Start = this->GetActorLocation() + FVector(0.0f, 0.0f, MaxHeight*2.0f);
	FVector End = Start - FVector(0.0f, 0.0f, MaxHeight * 4.0f);

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECC_GameTraceChannel2, Params))
	{
		int HighestScore = -1;
		int HighestRef = -1;

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
						//Check for Ramp Angle
						Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
					}
					else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
					{
						//Check for Ramp Angle
						Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
					}
				}
			}
		}
	}
	else
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

	Super::Destroyed();
}

FRotator ATM_Ramp::GetRotation()
{
	FRotator ReturnRot = FRotator(0.0f);

	switch (RampDirection)
	{
	case ETilt::T_XOnly:
		ReturnRot = FRotator(Angle, 0.0f, 0.0f);
		break;
	case ETilt::T_YOnly:
		ReturnRot = FRotator(0.0f, 0.0f, Angle);
		break;
	}

	return ReturnRot;
}

FVector ATM_Ramp::GetScaleAdjustment(float TileSize)
{
	float HypeScale = (TileSize / FMath::Cos(FMath::DegreesToRadians(Angle))) / TileSize;

	FVector ReturnVector = FVector(1.0f);

	switch (RampDirection)
	{
	case ETilt::T_XOnly:
		ReturnVector = FVector(HypeScale, 1.0f, 1.0f);
		break;
	case ETilt::T_YOnly:
		ReturnVector = FVector(1.0f, HypeScale, 1.0f);
		break;
	}

	return ReturnVector;
}

void ATM_Ramp::UpdateModifier()
{
	Score = (int)MidHeight;

	FVector Start = this->GetActorLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000.0f);

	FHitResult Hit;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
	{
		if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.GetActor()))
		{
			MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
			MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;

			FVector BoxLocation = Grid->GetClosestTileLocation(Hit.Location);
			BoxLocation.Z = Grid->GetActorLocation().Z + MaxHeight * 0.5f;
			Box->SetWorldLocation(BoxLocation);

			Box->SetBoxExtent(FVector(Grid->TileSize * 0.5f, Grid->TileSize * 0.5f, MaxHeight * 0.5f));

			if (CurrentIndex != Grid->GetIndexFromLocation(Hit.Location))
			{
				TArray<FHitResult> Hits;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				FVector PreviousStart = Grid->GetLocationFromIndex(CurrentIndex) + FVector(0.0f, 0.0f, 90000.0f);
				FVector PreviousEnd = PreviousStart - FVector(0.0f, 0.0f, 180000.0f);

				if (GetWorld()->LineTraceMultiByChannel(Hits, PreviousStart, PreviousEnd, ECC_GameTraceChannel2, Params))
				{
					int HighestScore = -1;
					int HighestRef = -1;

					for (int i = 0; i < Hits.Num(); i++)
					{
						if (ATileModifier* TileMod = Cast<ATileModifier>(Hits[i].Actor))
						{
							if (TileMod->Score > HighestScore)
							{
								HighestScore = TileMod->Score;
								HighestRef = i;
							}

							if (TileMod->Score > Score)
							{
								HighestRef = -1;
								break;
							}
						}
					}

					if (HighestRef != -1)
					{
						//Cast to Height or Ramp
						if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
						{
							//Check for Ramp Angle
							Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
						}
						else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
						{
							//Check for Ramp Angle
							Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
						}
					}
				}
				else
				{
					Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), 0.0f);
				}

				CurrentIndex = Grid->GetIndexFromLocation(Hit.Location);

				if (GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECC_GameTraceChannel2, Params))
				{
					int HighestScore = -1;
					int HighestRef = -1;

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

					if (HighestScore < Score)
					{
						Grid->UpdateIndexTransform(CurrentIndex, GetRotation(), MidHeight);
					}
					else
					{
						//Cast to Height or Ramp
						if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
						{
							//Check for Ramp Angle
							Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
						}
						else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
						{
							//Check for Ramp Angle
							Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
						}
					}
				}
				else
				{
					Grid->UpdateIndexTransform(CurrentIndex, GetRotation(), MidHeight, GetScaleAdjustment(Grid->TileSize));
				}
			}
			else
			{
				TArray<FHitResult> Hits;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				if (GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECC_GameTraceChannel2, Params))
				{
					int HighestScore = -1;
					int HighestRef = -1;

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

					if (HighestScore < Score)
					{
						Grid->UpdateIndexTransform(CurrentIndex, GetRotation(), MidHeight, GetScaleAdjustment(Grid->TileSize));
					}
					else
					{
						//Cast to Height or Ramp
						if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
						{
							//Check for Ramp Angle
							Grid->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(Grid->TileSize));
						}
						else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
						{
							//Check for Ramp Angle
							Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
						}
					}
				}
				else
				{
					Grid->UpdateIndexTransform(CurrentIndex, GetRotation(), MidHeight, GetScaleAdjustment(Grid->TileSize));
				}
			}
		}
	}
}

void ATM_Ramp::ClampToNeighbour(int Index)
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
					float HeightDifference = (FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Pitch))) * Grid->TileSize)*0.5f;

					if (Pitch > 0.0f)
					{
						//Tilted so high is +X and low is -X
						if (Index - CurrentIndex == 1)
						{
							//CurrentIndex is -X
							if (RampDirection == ETilt::T_XOnly)
							{
								if (Angle > 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z - HeightDifference - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MaxHeight = Grid->Grid[Index].Position.Z - HeightDifference;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else if (Angle < 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z - HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else
								{
									MinHeight = Grid->Grid[Index].Position.Z - HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
							}
							else
							{
								MidHeight = Grid->Grid[Index].Position.Z - HeightDifference;
								MaxHeight = MidHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								MinHeight = MidHeight - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								UpdateModifier();
							}
						}
						else if (Index - CurrentIndex == -1)
						{
							//Current Index is +X
							if (RampDirection == ETilt::T_XOnly)
							{
								if (Angle > 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z + HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else if (Angle < 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z + HeightDifference - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MaxHeight = Grid->Grid[Index].Position.Z + HeightDifference;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else
								{
									MinHeight = Grid->Grid[Index].Position.Z + HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
							}
							else
							{
								MidHeight = Grid->Grid[Index].Position.Z + HeightDifference;
								MaxHeight = MidHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								MinHeight = MidHeight - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								UpdateModifier();
							}
						}
					}
					else if (Pitch < 0.0f)
					{
						//Tilted so high is -X and low is +X
						if (Index - CurrentIndex == 1)
						{
							//CurrentIndex is -X
							if (RampDirection == ETilt::T_XOnly)
							{
								if (Angle > 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z + HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else if (Angle < 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z + HeightDifference - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MaxHeight = Grid->Grid[Index].Position.Z + HeightDifference;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else
								{
									MinHeight = Grid->Grid[Index].Position.Z + HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
							}
							else
							{
								MidHeight = Grid->Grid[Index].Position.Z + HeightDifference;
								MaxHeight = MidHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								MinHeight = MidHeight - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								UpdateModifier();
							}
						}
						else if (Index - CurrentIndex == -1)
						{
							//Current Index is +X
							if (RampDirection == ETilt::T_XOnly)
							{
								if (Angle > 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z - HeightDifference - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MaxHeight = Grid->Grid[Index].Position.Z - HeightDifference;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else if (Angle < 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z - HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else
								{
									MinHeight = Grid->Grid[Index].Position.Z - HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
							}
							else
							{
								MidHeight = Grid->Grid[Index].Position.Z - HeightDifference;
								MaxHeight = MidHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								MinHeight = MidHeight - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								UpdateModifier();
							}
						}
					}
				}
				else if (Grid->Grid[Index].Rotation.Roll != 0.0f && FMath::Abs(Index - CurrentIndex) == Grid->SizeX)
				{
					float Roll = Grid->Grid[Index].Rotation.Roll;
					float HeightDifference = (FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Roll))) * Grid->TileSize)*0.5f;

					if (Roll > 0.0f)
					{
						//Tilted so high is -Y and low is +Y
						if (Index - CurrentIndex == Grid->SizeX)
						{
							//CurrentIndex is -Y
							if (RampDirection == ETilt::T_YOnly)
							{
								if (Angle > 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z + HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else if (Angle < 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z + HeightDifference - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MaxHeight = Grid->Grid[Index].Position.Z + HeightDifference;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else
								{
									MinHeight = Grid->Grid[Index].Position.Z + HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
							}
							else
							{
								MidHeight = Grid->Grid[Index].Position.Z + HeightDifference;
								MaxHeight = MidHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								MinHeight = MidHeight - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								UpdateModifier();
							}
						}
						else if (Index - CurrentIndex == -Grid->SizeX)
						{
							//Current Index is +Y
							if (RampDirection == ETilt::T_YOnly)
							{
								if (Angle > 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z - HeightDifference - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MaxHeight = Grid->Grid[Index].Position.Z - HeightDifference;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else if (Angle < 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z - HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else
								{
									MinHeight = Grid->Grid[Index].Position.Z - HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
							}
							else
							{
								MidHeight = Grid->Grid[Index].Position.Z - HeightDifference;
								MaxHeight = MidHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								MinHeight = MidHeight - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								UpdateModifier();
							}
						}
					}
					else if (Roll < 0.0f)
					{
						//Tilted so high is +Y and low is -Y
						if (Index - CurrentIndex == Grid->SizeX)
						{
							//CurrentIndex is -Y
							if (RampDirection == ETilt::T_YOnly)
							{
								if (Angle > 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z - HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else if (Angle < 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z - HeightDifference - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MaxHeight = Grid->Grid[Index].Position.Z - HeightDifference;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else
								{
									MinHeight = Grid->Grid[Index].Position.Z - HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
							}
							else
							{
								MidHeight = Grid->Grid[Index].Position.Z - HeightDifference;
								MaxHeight = MidHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								MinHeight = MidHeight - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								UpdateModifier();
							}
						}
						else if (Index - CurrentIndex == -Grid->SizeX)
						{
							//Current Index is +Y
							if (RampDirection == ETilt::T_YOnly)
							{
								if (Angle > 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z + HeightDifference - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MaxHeight = Grid->Grid[Index].Position.Z + HeightDifference;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else if (Angle < 0.0f)
								{
									MinHeight = Grid->Grid[Index].Position.Z + HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
								else
								{
									MinHeight = Grid->Grid[Index].Position.Z + HeightDifference;
									MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
									MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
									UpdateModifier();
								}
							}
							else
							{
								MidHeight = Grid->Grid[Index].Position.Z + HeightDifference;
								MaxHeight = MidHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								MinHeight = MidHeight - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
								UpdateModifier();
							}
						}
					}
				}
				else
				{
					if (Index - CurrentIndex == 1)
					{
						//+X
						if (RampDirection == ETilt::T_XOnly)
						{
							if (Angle > 0.0f)
							{
								MinHeight = Grid->Grid[Index].Position.Z - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
								MaxHeight = Grid->Grid[Index].Position.Z;
								MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
								UpdateModifier();
							}
							else if (Angle < 0.0f)
							{
								MinHeight = Grid->Grid[Index].Position.Z;
								MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
								MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
								UpdateModifier();
							}
							else
							{
								MinHeight = Grid->Grid[Index].Position.Z;
								MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
								MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
								UpdateModifier();
							}
						}
						else
						{
							MidHeight = Grid->Grid[Index].Position.Z;
							MaxHeight = MidHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
							MinHeight = MidHeight - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
							UpdateModifier();
						}
					}
					else if (Index - CurrentIndex == -1)
					{
						if (RampDirection == ETilt::T_XOnly)
						{
							if (Angle > 0.0f)
							{
								MinHeight = Grid->Grid[Index].Position.Z;
								MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
								MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
								UpdateModifier();
							}
							else if (Angle < 0.0f)
							{
								MinHeight = Grid->Grid[Index].Position.Z - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
								MaxHeight = Grid->Grid[Index].Position.Z;
								MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
								UpdateModifier();
							}
							else
							{
								MinHeight = Grid->Grid[Index].Position.Z;
								MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
								MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
								UpdateModifier();
							}
						}
						else
						{
							MidHeight = Grid->Grid[Index].Position.Z;
							MaxHeight = MidHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
							MinHeight = MidHeight - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
							UpdateModifier();
						}
					}
					else if (Index - CurrentIndex == Grid->SizeX)
					{
						//+Y
						if (RampDirection == ETilt::T_YOnly)
						{
							if (Angle > 0.0f)
							{
								MinHeight = Grid->Grid[Index].Position.Z;
								MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
								MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
								UpdateModifier();
							}
							else if (Angle < 0.0f)
							{
								MinHeight = Grid->Grid[Index].Position.Z - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
								MaxHeight = Grid->Grid[Index].Position.Z;
								MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
								UpdateModifier();
							}
							else
							{
								MinHeight = Grid->Grid[Index].Position.Z;
								MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
								MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
								UpdateModifier();
							}
						}
						else
						{
							MidHeight = Grid->Grid[Index].Position.Z;
							MaxHeight = MidHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
							MinHeight = MidHeight - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
							UpdateModifier();
						}
					}
					else if (Index - CurrentIndex == -Grid->SizeX)
					{
						//-Y
						if (RampDirection == ETilt::T_YOnly)
						{
							if (Angle > 0.0f)
							{
								MinHeight = Grid->Grid[Index].Position.Z - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
								MaxHeight = Grid->Grid[Index].Position.Z;
								MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
								UpdateModifier();
							}
							else if (Angle < 0.0f)
							{
								MinHeight = Grid->Grid[Index].Position.Z;
								MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
								MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
								UpdateModifier();
							}
							else
							{
								MinHeight = Grid->Grid[Index].Position.Z;
								MaxHeight = MinHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize;
								MidHeight = MinHeight + (MaxHeight - MinHeight) * 0.5f;
								UpdateModifier();
							}
						}
						else
						{
							MidHeight = Grid->Grid[Index].Position.Z;
							MaxHeight = MidHeight + FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
							MinHeight = MidHeight - FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Angle))) * Grid->TileSize * 0.5f;
							UpdateModifier();
						}
					}
				}
			}
		}
	}
}