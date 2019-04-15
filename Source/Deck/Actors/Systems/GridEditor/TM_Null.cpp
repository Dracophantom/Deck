// Fill out your copyright notice in the Description page of Project Settings.

#include "TM_Null.h"
#include "Actors/Systems/GridEditor/TM_Height.h"
#include "Actors/Systems/GridEditor/TM_Ramp.h"
#include "Engine.h"

ATM_Null::ATM_Null()
{
	//Create a Box Component
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComponent);

	//Set Collision Response for Grid Modifier
	Box->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);

	//Set Score to -1
	Score = -1;
}

//Handle OnConstruction
void ATM_Null::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	FVector Start = Transform.GetLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000.0f);

	FHitResult Hit;

	//Try to find a Grid
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
	{
		if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.GetActor()))
		{
			//If CurrentIndex != new Index
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
					else
					{
						Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), 0.0f);
					}
				}
				else
				{
					Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), 0.0f);
				}

				CurrentIndex = Grid->GetIndexFromLocation(Hit.Location);
			}

			GridRef = Grid;
			FVector BoxLocation = Grid->GetClosestTileLocation(Hit.Location);
			BoxLocation.Z = Grid->GetActorLocation().Z + Grid->TileSize * 0.5f;
			Box->SetWorldLocation(BoxLocation);

			Box->SetBoxExtent(FVector(Grid->TileSize * 0.5f, Grid->TileSize * 0.5f, Grid->TileSize * 0.5f));

			Grid->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), 0.0f, FVector(0.0f));
		}
	}
	else
	{
		if (GridRef)
		{
			FVector BoxLocation = GridRef->GetClosestTileLocation(Hit.Location);
			BoxLocation.Z = GridRef->GetActorLocation().Z + GridRef->TileSize * 0.5f;
			Box->SetWorldLocation(BoxLocation);
		}
	}
}

void ATM_Null::Destroyed()
{
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FVector Start = this->GetActorLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000.0f);

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
			if (GridRef)
				GridRef->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), 0.0f);
		}
		else if (HighestScore < Score)
		{
			//Cast to Height or Ramp
			if (ATM_Ramp* Ramp = Cast<ATM_Ramp>(Hits[HighestRef].Actor))
			{
				//Check for Ramp Angle
				if (GridRef)
					GridRef->UpdateIndexTransform(CurrentIndex, Ramp->GetRotation(), Ramp->MidHeight, Ramp->GetScaleAdjustment(GridRef->TileSize));
			}
			else if (ATM_Height* HeightMod = Cast<ATM_Height>(Hits[HighestRef].Actor))
			{
				//Check for Ramp Angle
				if (GridRef)
					GridRef->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), HeightMod->Height);
			}
		}
	}
	else
	{
		if (GridRef)
			GridRef->UpdateIndexTransform(CurrentIndex, FRotator(0.0f), 0.0f);
	}

	Super::Destroyed();
}