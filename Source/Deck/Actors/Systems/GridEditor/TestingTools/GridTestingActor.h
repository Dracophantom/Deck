// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actors/Player/DeckCard.h"
#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "GridTestingActor.generated.h"

UCLASS()
class DECK_API AGridTestingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridTestingActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void OnConstruction(const FTransform &Transform) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		UStaticMeshComponent* Mesh;

	UFUNCTION(CallInEditor, Category = "Movement")
		void CheckMovement();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = 1))
		int MovementRange = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = 1.0f))
		float VerticalMovement = 1.0f;

	UFUNCTION(CallInEditor, Category = "Attack")
		void CheckAttack();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
		EPatternType PlacementPattern;

	//Min distance for the attack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = 0))
		int MinDistance;

	//Max distance for the attack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = 0))
		int MaxDistance;
};
