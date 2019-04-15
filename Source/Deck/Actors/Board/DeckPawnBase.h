// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Actors/Components/StatsComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Actors/Systems/Gameplay/GE_StatusBase.h"
#include "DeckPawnBase.generated.h"

UCLASS()
class DECK_API ADeckPawnBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADeckPawnBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	//OnConstruction override to help with the placement of Characters
	virtual void OnConstruction(const FTransform &Transform) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Handle the Pawn's start of turn
	virtual void StartTurn();
	//Handle the Pawn's end of turn
	virtual void EndTurn();

	//Handle when the Pawn finishes an Action
	virtual void FinishAction(){}

	//Start moving the Pawn
	bool StartMovement(TArray<int> NewPath);

	//Update the Pawn's movement
	void UpdateMovement();

	//Finish the Pawn's movement
	virtual void FinishMovement();

	//Mesh used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box")
		UBoxComponent* Box;

	//Stats Component
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
		UStatsComponent* Stats;

	//The Path being used for Movement
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TArray<int> MovementPath;

	//Timer for controlling Movement
	FTimerHandle MovementTimer;

	//Array of Status Effects on the Pawn
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TArray<AGE_StatusBase*> StatusEffects;
};
