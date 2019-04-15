// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine.h"
#include "StatsComponent.generated.h"

//Delegates for OnDeath and OnHit
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHit, int, Damage);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DECK_API UStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatsComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Handle Damage
	UFUNCTION(BlueprintCallable)
	void Damage(int Damage);

	//Handle Healing
	UFUNCTION(BlueprintCallable)
	void Heal(int Heal);

	//OnDeath Delegate
	UPROPERTY(BlueprintAssignable)
		FOnDeath OnDeath;

	//OnHit Delegate
	UPROPERTY(BlueprintAssignable)
		FOnHit OnHit;

	//Current Health
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Combat")
		int CurrentHealth = 0;

	//Max Health
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Combat", meta = (ClampMin = 1))
		int MaxHealth = 1;

	//Attack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Combat", meta = (ClampMin = 1))
		int Attack = 1;

	//Magic Attack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Combat", meta = (ClampMin = 1))
		int MagicPower = 1;

	//Defense
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Combat", meta = (ClampMin = 1))
		int Defense = 1;

	//Magic Defense
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Combat", meta = (ClampMin = 1))
		int MagicResistance = 1;

	//Speed used for combat order
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Combat", meta = (ClampMin = 1))
		int Speed = 1;

	//Available Movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Movement")
		int AvailableMovement = 1;

	//Max Movement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Movement", meta = (ClampMin = 1))
		int MaxMovement = 1;

	//Vertical Movement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Movement", meta = (ClampMin = 1.0f))
		float VerticalMovement = 1.0f;
};
