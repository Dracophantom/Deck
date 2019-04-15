// Fill out your copyright notice in the Description page of Project Settings.

#include "GridTestingActor.h"
#include "Actors/Systems/GridEditor/GridManager_Editor.h"
#include "Engine.h"

// Sets default values
AGridTestingActor::AGridTestingActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT("StaticMesh'/Game/Art/Systems/GridWall.GridWall'"));
	UStaticMesh* Asset = MeshAsset.Object;
	Mesh->SetStaticMesh(Asset);

	Mesh->SetRelativeScale3D(FVector(50.0f));
}

// Called when the game starts or when spawned
void AGridTestingActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGridTestingActor::OnConstruction(const FTransform &Transform)
{
	FVector Start = Transform.GetLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000.0f);

	FHitResult Hit;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
	{
		if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.Actor))
		{
			int Index = Grid->GetIndexFromLocation(Hit.Location);

			if (Index != -1)
			{
				//Set Location of Mesh
				Mesh->SetWorldLocation(Grid->GetLocationFromIndex(Index));

				Grid->ClearHighlights();
			}
		}
	}
}

// Called every frame
void AGridTestingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGridTestingActor::CheckMovement()
{
	FVector Start = this->GetActorLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000.0f);

	FHitResult Hit;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
	{
		if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.Actor))
		{
			int Index = Grid->GetIndexFromLocation(Hit.Location);

			if (Index != -1)
			{
				Grid->HighlightMovement(Hit.Location, MovementRange, VerticalMovement);
			}
		}
	}
}

void AGridTestingActor::CheckAttack()
{
	FVector Start = this->GetActorLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000.0f);

	FHitResult Hit;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
	{
		if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.Actor))
		{
			int Index = Grid->GetIndexFromLocation(Hit.Location);

			if (Index != -1)
			{
				Grid->HighlightAttackPlacement(Hit.Location, PlacementPattern, MinDistance, MaxDistance);
			}
		}
	}
}


