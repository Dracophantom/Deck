// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Actors/Board/PlayerPawn.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "DeckPC.generated.h"

//Current State the PC could be in
UENUM(BlueprintType)
enum class ECurrentState : uint8
{
	CS_Inactive		UMETA(DisplayName = "Inactive"),
	CS_Inspection	UMETA(DisplayName = "Inspection"),
	CS_Movement		UMETA(DisplayName = "Movement"),
	CS_Action		UMETA(DisplayName = "Action Placement"),
	CS_Action_C		UMETA(DisplayName = "Action Confirmation"),
	CS_Discard		UMETA(DisplayName = "Discard"),
	CS_SelectCards	UMETA(DisplayName = "Select Cards"),
	CS_Pending		UMETA(DisplayName = "Pending Action")
};
/**
 * 
 */
UCLASS()
class DECK_API ADeckPC : public APlayerController, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = CardTags; return; }

	//Set up Input Component
	virtual void SetupInputComponent() override;

	//Select a Tile
	void SelectTile();

	void StartBackStep();

	void ClearBackstep();

	//Go back a step in the Menu
	void Backstep();

	//Start the Player's turn
	UFUNCTION(BlueprintCallable)
		void StartTurn();

	//End the Player's turn
	UFUNCTION(BlueprintCallable)
		void EndTurn();

	//Start Movement step
	UFUNCTION(BlueprintCallable)
		void StartMove();

	//Finish the Movement step
	void FinishMove();

	//Start an Action
	UFUNCTION(BlueprintCallable)
		void StartAction(int ActionIndex);

	//Confirm an Action
	UFUNCTION(BlueprintCallable)
		void ConfirmAction();

	//Current Action is Finished
	UFUNCTION(BlueprintCallable)
		void ActionFinished();

	//Start selecting cards to Discard
	UFUNCTION(BlueprintCallable)
		void DiscardSelection(FGameplayTagContainer TargetTags, int Min, int Max);

	//Start selecting cards
	UFUNCTION(BlueprintCallable)
		void CardSelection(FGameplayTagContainer TargetTags, int Min, int Max);

	//Add a Selected card to hand interactions
	UFUNCTION(BlueprintCallable)
		void AddSelectedCard(int CardRef);

	//Remove a Selected card from hand interactions
	UFUNCTION(BlueprintCallable)
		void RemoveSelectedCard(int CardRef);

	//Finish current hand interaction
	void FinishCardInteraction();

	//Currently focused character that can be controlled
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
		APlayerPawn* CurrentCharacter;

	//Currently selected card
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
		int CurrentCardRef = -1;

	//Tiles that will be hit by a current action
	TArray<int> TilesToHit;

	//Current state
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	ECurrentState CurrentState = ECurrentState::CS_Inactive;

	//Selected cards
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Card Selection")
		TArray<int> SelectCardRefs;

	//Min cards to select
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Card Selection")
		int MinCardsRequired = 0;

	//Max cards to select
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Card Selection")
		int MaxCardsRequired = 0;

	//Cards that can be selected
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
		FGameplayTagContainer CardTags;

	UPROPERTY(EditAnywhere, Category = "Development")
		float InputDelay = 0.75f;

	FTimerHandle BackstepTimer;
};
