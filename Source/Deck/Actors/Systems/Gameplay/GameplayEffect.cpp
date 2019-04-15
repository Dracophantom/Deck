// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayEffect.h"
#include "DeckGM.h"
#include "Engine.h"

// Sets default values
AGameplayEffect::AGameplayEffect()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AGameplayEffect::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGameplayEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//Default implementation for SetProperties
void AGameplayEffect::SetProperties_Implementation(const TArray<FPropertyData>& Properties)
{
	//Loop through all passed in Properties
	for (int i = 0; i < Properties.Num(); i++)
	{
		//If the Type is a Float
		if (Properties[i].PropertyType == TEXT("float") || Properties[i].PropertyType == TEXT("Float"))
		{
			//Find the Float Property listed
			UFloatProperty* FloatProp = FindField<UFloatProperty>(this->GetClass(), Properties[i].PropertyName);

			//If successful, set the value
			if (FloatProp)
			{
				FloatProp->SetPropertyValue_InContainer(this, FCString::Atof(*Properties[i].PropertyValue.ToString()));
			}
		}
		//If the Type is a Int
		else if (Properties[i].PropertyType == TEXT("int") || Properties[i].PropertyType == TEXT("Int") || Properties[i].PropertyType == TEXT("integer") || Properties[i].PropertyType == TEXT("Integer"))
		{
			//Find the Int Property listed
			UIntProperty* IntProp = FindField<UIntProperty>(this->GetClass(), Properties[i].PropertyName);

			//If successful, set the value
			if (IntProp)
			{
				IntProp->SetPropertyValue_InContainer(this, FCString::Atoi(*Properties[i].PropertyValue.ToString()));
			}
		}
		//If the Type is a Bool
		else if (Properties[i].PropertyType == TEXT("bool") || Properties[i].PropertyType == TEXT("Bool") || Properties[i].PropertyType == TEXT("boolean") || Properties[i].PropertyType == TEXT("Boolean"))
		{
			//Find the Bool Property listed
			UBoolProperty* BoolProp = FindField<UBoolProperty>(this->GetClass(), Properties[i].PropertyName);

			//If successful, set the value
			if (BoolProp)
			{
				if (Properties[i].PropertyValue == TEXT("true") || Properties[i].PropertyValue == TEXT("True"))
				{
					BoolProp->SetPropertyValue_InContainer(this, true);
				}
				else if (Properties[i].PropertyValue == TEXT("false") || Properties[i].PropertyValue == TEXT("False"))
				{
					BoolProp->SetPropertyValue_InContainer(this, false);
				}
			}
		}
	}
}

void AGameplayEffect::StartEffect_Implementation()
{
}

void AGameplayEffect::FinishEffect_Implementation()
{
	//Destroy the effect
	this->Destroy();
}