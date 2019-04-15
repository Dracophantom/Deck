// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Animation/AnimMontage.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "Actors/Systems/Gameplay/GE_CardBase.h"
#include "DeckCard.generated.h"

//Type of Slot the Card uses
UENUM(BlueprintType)
enum class ESlotType : uint8
{
	ST_Action	UMETA(DisplayName = "Action"),
	ST_Bonus	UMETA(DisplayName = "Bonus"),
	ST_Free		UMETA(DisplayName = "Free")
};

//Type of Card
UENUM(BlueprintType)
enum class ECardType : uint8
{
	CT_Physical	UMETA(DisplayName = "Physical"),
	CT_Magical	UMETA(DisplayName = "Magical"),
	CT_Healing	UMETA(DisplayName = "Healing"),
	CT_Buff		UMETA(DisplayName = "Buff"),
	CT_Debuff	UMETA(DisplayName = "Debuff")
};

//Pre-Defined Attack Patterns
UENUM(BlueprintType)
enum class EPatternType : uint8
{
	PT_Line			UMETA(DisplayName = "Line"),
	PT_Diagonal		UMETA(DisplayName = "Diagonal"),
	PT_Diamond		UMETA(DisplayName = "Diamond"),
	PT_Square		UMETA(DisplayName = "Square"),
	PT_Spread		UMETA(DisplayName = "Spread")
};
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class DECK_API UDeckCard : public UObject, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	
public:

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = GameplayTags; return; }

	//Set the type of Input the Card will consume
	UFUNCTION(BlueprintCallable)
		void SetConsumedInput(ESlotType Input);

	//An animation tied to the card
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
		UAnimMontage* Animation;

	//Name of the Card
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
		FName Name;

	//Default Slot to use
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
		ESlotType DefaultSlot;

	//Consumed Input
	UPROPERTY(BlueprintReadOnly, Category = "Action")
		ESlotType ConsumedInput;

	//Type of Card
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
		ECardType CardType;

	//Gameplay Tags
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
		FGameplayTagContainer GameplayTags;
	/*
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Pattern")
		PatternType AttackPattern;
		*/

	///Temporary until custom UI editing is improved
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Pattern")
		bool bUseCustom;

	/////////
	//CUSTOM/
	/////////

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Pattern", meta = (EditCondition = "bUseCustom"))
		FName CUSTOM_PLACEHOLDER = TEXT("Holder for Custom Details");

	/////////////
	//Calculated/
	/////////////

	//Type of Pattern for initial check
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Pattern|Placement", meta = (EditCondition = "!bUseCustom"))
		EPatternType PlacementPattern;

	//Min distance for the attack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Pattern|Placement", meta = (EditCondition = "!bUseCustom"))
		int PlacementMin;

	//Max distance for the attack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Pattern|Placement", meta = (EditCondition = "!bUseCustom"))
		int PlacementMax;

	//Type of Pattern used to detect what's hit
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Pattern|Hit", meta = (EditCondition = "!bUseCustom"))
		EPatternType HitPattern;

	//Whether it originates from the Caster or not
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Pattern|Hit", meta = (EditCondition = "!bUseCustom"))
		bool bFromCaster;

	//Min distance for the Hit
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Pattern|Hit", meta = (EditCondition = "!bUseCustom"))
		int HitMin;

	//Max distance for the Hit
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Pattern|Hit", meta = (EditCondition = "!bUseCustom"))
		int HitMax;

	//Effects tied to the Card
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
		TArray<TSubclassOf<AGE_CardBase>> Effects;

	//Attributes the Card has which can influence the Card Effects
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
		TArray<FPropertyData> CardProperties;
};
