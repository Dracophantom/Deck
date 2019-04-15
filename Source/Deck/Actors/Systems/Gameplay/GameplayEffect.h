// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffect.generated.h"

//Data for setting Properties for Gameplay Effects
USTRUCT(BlueprintType)
struct FPropertyData
{
	GENERATED_BODY()

	//The Property Name
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName PropertyName;

	//The Property Type
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName PropertyType;

	//The Property Value
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName PropertyValue;
};

UCLASS()
class DECK_API AGameplayEffect : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameplayEffect();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Set the Gameplay Effect's properties
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SetProperties(const TArray<FPropertyData>& Properties);
		virtual void SetProperties_Implementation(const TArray<FPropertyData>& Properties);

	//Start the Gameplay Effect
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void StartEffect();
		virtual void StartEffect_Implementation();

	//Finish the Gameplay Effect
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void FinishEffect();
		virtual void FinishEffect_Implementation();
};
