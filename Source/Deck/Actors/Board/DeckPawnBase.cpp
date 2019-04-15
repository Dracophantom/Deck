// Fill out your copyright notice in the Description page of Project Settings.

#include "DeckPawnBase.h"
#include "Engine.h"
#include "Actors/Systems/Grid/GridManager.h"
#include "GameplayTagsManager.h"
#include "DeckGM.h"

// Sets default values
ADeckPawnBase::ADeckPawnBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create a default Root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComponent);
	Box->SetRelativeLocation(FVector(0.0f));

	//Create and attach a Skeletal Mesh
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	//Create and attach Stats
	Stats = CreateDefaultSubobject<UStatsComponent>(TEXT("Stats"));
	this->AddOwnedComponent(Stats);

}

// Called when the game starts or when spawned
void ADeckPawnBase::BeginPlay()
{
	Super::BeginPlay();

	//Get the Mesh's world location and make it the Pawn's location
	this->SetActorLocation(Mesh->GetComponentLocation());
	//Change the Mesh's relative to be 0
	Mesh->SetRelativeLocation(FVector(0.0f));

	FVector Start = this->GetActorLocation() + FVector(0.0f, 0.0f, 25.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 50.0f);

	FHitResult Hit;

	//Perform a trace straight down
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility))
	{
		//If a Grid is hit
		if (AGridManager* Grid = Cast<AGridManager>(Hit.Actor))
		{
			//Get the Index
			int Index = Grid->GetIndexFromLocation(Hit.Location);

			//If the Index is valid
			if (Index != -1)
			{
				//Occupy the Space
				Grid->Grid[Index].bIsOccupied = true;
			}
		}
	}

	//If the GM is valid as ADeckGM
	if (ADeckGM* GM = Cast<ADeckGM>(GetWorld()->GetAuthGameMode()))
	{
		GM->AddNewCharacter(this);
	}
}

//OnConstruction override to help with the placement of Characters
void ADeckPawnBase::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	
	FVector Start = Transform.GetLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000);

	FHitResult Hit;

	//Perform a trace straight down
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility))
	{
		//If a Grid is hit
		if (AGridManager* Grid = Cast<AGridManager>(Hit.Actor))
		{
			//Position the Mesh on to that index's space
			Mesh->SetWorldLocation(Grid->GetClosestTileLocation(Hit.Location));
		}
	}
}

// Called every frame
void ADeckPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//If MovementTimer is Active
	if (GetWorldTimerManager().IsTimerActive(MovementTimer))
	{
		/*******************
		*TEMPORARY SOLUTION*
		*******************/
		//UNTIL GRIDMANAGER IS STORED IN AN EASY TO REFERENCE LOCATION

		//Locations to LERP between
		FVector FirstLocation;
		FVector SecondLocation;
		//0-1 Delta to use for LERP based on the Movement Timer
		float Delta = GetWorldTimerManager().GetTimerElapsed(MovementTimer) / 5.0f;

		//If the GM is valid as ADeckGM
		if (ADeckGM* GM = Cast<ADeckGM>(GetWorld()->GetAuthGameMode()))
		{
			//Get the First and Second locations from the Grid
			FirstLocation = GM->GridRef->GetLocationFromIndex(MovementPath[0]);
			SecondLocation = GM->GridRef->GetLocationFromIndex(MovementPath[1]);

			//Lerp
			this->SetActorLocation(FMath::Lerp(FirstLocation, SecondLocation, Delta));
		}
	}
}

// Called to bind functionality to input
void ADeckPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//Handle the Pawn's start of turn
void ADeckPawnBase::StartTurn()
{
	//Reset Movement
	Stats->AvailableMovement = Stats->MaxMovement;

	///Separate from StartTurn so that it can run through any animations
	//Get the Gameplay Tag for Start of Turn
	FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Status Effect.Trigger Time.Start of Turn"));
	//Loop through all Status Effects
	for (int i = 0; i < StatusEffects.Num(); i++)
	{
		//If a Status Effect has the right tag, Update it
		if (StatusEffects[i]->GameplayTags.HasTag(Tag))
		{
			StatusEffects[i]->UpdateEffect();
		}
	}
}

//Handle the Pawn's end of turn
void ADeckPawnBase::EndTurn()
{
	//If the GM is valid as ADeckGM
	if (ADeckGM* GM = Cast<ADeckGM>(GetWorld()->GetAuthGameMode()))
	{
		//Separate from EndTurn so that it can run through any animations
	//Get the Gameplay Tag for End of Turn
		FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Status Effect.Trigger Time.End of Turn"));
		//Loop through all Status Effects
		for (int i = 0; i < StatusEffects.Num(); i++)
		{
			//If a Status Effect has the right tag, Update it
			if (StatusEffects[i]->GameplayTags.HasTag(Tag))
			{
				StatusEffects[i]->UpdateEffect();
			}
		}

		//Move to the Next Character
		GM->NextCharacter();
	}
}

//Start moving the Pawn
bool ADeckPawnBase::StartMovement(TArray<int> NewPath)
{
	//If the Path is Valid
	if (NewPath.Num() > 1 && NewPath.Num() <= Stats->AvailableMovement + 1)
	{
		//Save the Path
		MovementPath = NewPath;
		//Start the Movement Timer
		GetWorldTimerManager().SetTimer(MovementTimer, this, &ADeckPawnBase::UpdateMovement, 5.0f, true);
		//Deduct from Available Movement
		Stats->AvailableMovement -= (NewPath.Num() - 1);

		FVector Start = this->GetActorLocation() + FVector(0.0f, 0.0f, 25.0f);
		FVector End = Start - FVector(0.0f, 0.0f, 50.0f);

		FHitResult Hit;

		//Ray trace straight down to find a Grid
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility))
		{
			//If a Grid is found
			if (AGridManager* Grid = Cast<AGridManager>(Hit.Actor))
			{
				//Get the Index
				int Index = Grid->GetIndexFromLocation(Hit.Location);

				//If the Index is valid, it is no longer occupied
				if (Index != -1)
				{
					Grid->Grid[Index].bIsOccupied = false;
				}
			}
		}

		//Return true
		return true;
	}
	
	//Return false
	return false;
}

//Update the Pawn's movement
void ADeckPawnBase::UpdateMovement()
{
	//If the Movement Path is greater than 2
	if (MovementPath.Num() > 2)
	{
		//Remove the first Index
		MovementPath.RemoveAt(0);
	}
	else
	{
		//Otherwise clear and Finish the timer
		GetWorldTimerManager().ClearTimer(MovementTimer);
		FinishMovement();
	}
}

//Finish the Pawn's movement
void ADeckPawnBase::FinishMovement()
{
	FVector Start = this->GetActorLocation() + FVector(0.0f, 0.0f, 25.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 50.0f);

	FHitResult Hit;

	//Ray trace straight down
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility))
	{
		//If a Grid is found
		if (AGridManager* Grid = Cast<AGridManager>(Hit.Actor))
		{
			//Get the Index
			int Index = Grid->GetIndexFromLocation(Hit.Location);

			//If the Index is valid, that tile is now occupied
			if (Index != -1)
			{
				Grid->Grid[Index].bIsOccupied = true;
			}
		}
	}
}
