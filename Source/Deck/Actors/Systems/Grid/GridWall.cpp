// Fill out your copyright notice in the Description page of Project Settings.

#include "GridWall.h"
#include "Engine.h"

// Sets default values
AGridWall::AGridWall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT("StaticMesh'/Game/Art/Systems/GridWall.GridWall'"));
	UStaticMesh* Asset = MeshAsset.Object;
	Mesh->SetStaticMesh(Asset);
}

// Called when the game starts or when spawned
void AGridWall::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGridWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGridWall::CreateWalls_Implementation(float TileSize, float Height, float Thickness, uint8 Directions)
{
	for (int32 i = 0; i <= 3; ++i)
	{
		if (Directions & (1 << i))
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

