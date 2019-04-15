// Fill out your copyright notice in the Description page of Project Settings.

#include "StatsComponent.h"

// Sets default values for this component's properties
UStatsComponent::UStatsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

//Handle Damage
void UStatsComponent::Damage(int Damage)
{
	//Check that the CurrentHealth is greater than 0
	if (CurrentHealth > 0)
	{
		//Deduct Damage from CurrentHealth
		CurrentHealth -= Damage;

		//If CurrentHealth <= Broadcast Death, otherwise Broadcast Hit
		if (CurrentHealth <= 0)
		{
			OnDeath.Broadcast();
		}
		else
		{
			OnHit.Broadcast(Damage);
		}
	}
}

//Handle Healing
void UStatsComponent::Heal(int Heal)
{
	//Check that Healing can occur
	if (CurrentHealth > 0 && CurrentHealth < MaxHealth)
	{
		//Add health capped at MaxHealth
		CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Heal);
	}
}