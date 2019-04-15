// Fill out your copyright notice in the Description page of Project Settings.

#include "TM_Wall.h"
#include "Actors/Systems/GridEditor/GridManager_Editor.h"
#include "Engine.h"

ATM_Wall::ATM_Wall()
{
	Mesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT("StaticMesh'/Game/Art/Systems/GridWall.GridWall'"));
	UStaticMesh* Asset = MeshAsset.Object;
	Mesh->SetStaticMesh(Asset);

	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);

	Score = 0;
}

void ATM_Wall::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	FVector Start = Transform.GetLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000.0f);

	FHitResult Hit;

	Score = Height;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
	{
		if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.GetActor()))
		{
			Mesh->SetWorldLocation(Grid->GetClosestTileLocation(Hit.Location));

			CurrentIndex = Grid->GetIndexFromLocation(Hit.Location);

			InstantiateWalls(Grid ->TileSize);
		}
	}
}

void ATM_Wall::InstantiateWalls(float TileSize)
{
	Mesh->ClearInstances();

	for (int32 i = (int32)EDirectionFlags::D_PosX; i <= (int32)EDirectionFlags::D_NegY; ++i)
	{
		if (DirectionFlags & (1 << i))
		{
			FVector Offset = FVector(0.0f);
			FRotator Rotation = FRotator(0.0f);

			switch (i)
			{
			case 0:
				Offset = FVector(TileSize * 0.5f, 0.0f, 0.0f);

				break;
			case 1:
				Offset = FVector(0.0f, TileSize * 0.5f, 0.0f);
				Rotation = FRotator(0.0f, 90.0f, 0.0f);

				break;
			case 2:
				Offset = FVector(-TileSize * 0.5f, 0.0f, 0.0f);

				break;
			case 3:
				Offset = FVector(0.0f, -TileSize * 0.5f, 0.0f);
				Rotation = FRotator(0.0f, 90.0f, 0.0f);

				break;
			}

			Mesh->AddInstance(FTransform(Rotation.Quaternion(), Offset, FVector(Thickness, TileSize, Height)));
		}
	}
}

void ATM_Wall::ToggleWall(EDirectionFlags Direction)
{
	DirectionFlags ^= (1 << (int32)Direction);

	FVector Start = this->GetActorLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000.0f);

	FHitResult Hit;

	Score = Height;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
	{
		if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.GetActor()))
		{
			Mesh->SetWorldLocation(Grid->GetClosestTileLocation(Hit.Location));

			CurrentIndex = Grid->GetIndexFromLocation(Hit.Location);

			InstantiateWalls(Grid->TileSize);
		}
	}
}

void ATM_Wall::UpdateModifier()
{
	FVector Start = this->GetActorLocation() + FVector(0.0f, 0.0f, 90000.0f);
	FVector End = Start - FVector(0.0f, 0.0f, 180000.0f);

	FHitResult Hit;

	Score = Height;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1))
	{
		if (AGridManager_Editor* Grid = Cast<AGridManager_Editor>(Hit.GetActor()))
		{
			Mesh->SetWorldLocation(Grid->GetClosestTileLocation(Hit.Location));

			CurrentIndex = Grid->GetIndexFromLocation(Hit.Location);

			InstantiateWalls(Grid->TileSize);
		}
	}
}

void ATM_Wall::ClampToNeighbour(int Index)
{

}