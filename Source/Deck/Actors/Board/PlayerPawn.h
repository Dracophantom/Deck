// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Board/DeckPawnBase.h"
#include "Actors/Player/DeckCard.h"
#include "GameplayTagsManager.h"
#include "PlayerPawn.generated.h"

//Struct for handling cards in the deck
USTRUCT(BlueprintType)
struct FDeckData
{
	GENERATED_BODY()

	//Type of Card to create
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck Data")
	TSubclassOf<UDeckCard> Card;

	//Count in deck
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck Data")
		int InDeck;
};
/**
 * 
 */
UCLASS()
class DECK_API APlayerPawn : public ADeckPawnBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	//Handle the Pawn's start of turn
	virtual void StartTurn() override;
	//Handle the Pawn's start of turn
	virtual void EndTurn() override;

	//Handle when the Pawn's Movement is Finished
	virtual void FinishMovement() override;

	//Add ControlUI in Blueprint
	UFUNCTION(BlueprintImplementableEvent)
		void AddControlUI();

	//RemoveUI in Blueprint
	UFUNCTION(BlueprintImplementableEvent)
		void RemoveUI();

	//Draw new Cards
	UFUNCTION(BlueprintCallable)
		void DrawCards();

	//Handle when the Pawn finishes an Action
	virtual void FinishAction() override;

	//Add an Available Action
	UFUNCTION(BlueprintCallable)
	void AddAction();

	//Remove an Available Action
	UFUNCTION(BlueprintCallable)
	void RemoveAction();

	//Add an Available Bonus Action
	UFUNCTION(BlueprintCallable)
	void AddBonusAction();

	//Remove an Available Bonus Action
	UFUNCTION(BlueprintCallable)
	void RemoveBonusAction();

	//Discard specified Cards
	UFUNCTION(BlueprintCallable)
		void DiscardCards(TArray<int> CardRefs);

	//Add a a tag to selected Cards
	UFUNCTION(BlueprintCallable)
		void AddCardTags(TArray<int> CardRefs, FGameplayTag TargetTag);

	//Discard a Card after use
	void DiscardUsedCard(int CardRef);

	//Array of Cards 'in the deck' - Purely data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
		TArray<FDeckData> Deck;

	//Array of Cards in hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<UDeckCard*> Hand;

	//# of Actions available
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
		int AvailableActions = 1;

	//# of Actions available
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
		int AvailableBonusActions = 1;
};
