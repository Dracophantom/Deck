// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "CameraPawn.generated.h"

UCLASS()
class DECK_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACameraPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Dolly(float Val);

	void Orbit(float Val);

	void Tilt(float Val);

	void EnableCameraMovement();

	void DisableCameraMovement();

	void SetNewFocus(AActor* NewFocus);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		USceneComponent* CameraPivot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Controls")
		float MinDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Controls")
		float Distance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Controls")
		float MaxDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Controls")
		float MinPitch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Controls")
		float MaxPitch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Controls")
		float MoveRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Controls")
		float TurnRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Controls")
		float PitchRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Controls")
		bool bCameraMovementEnabled = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Controls")
		AActor* FocusTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Controls", meta = (ClampMin = 0.0f))
		float MovementSpeed = 1.0f;
};
