// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraPawn.h"
#include "Components/InputComponent.h"
#include "Engine.h"

// Sets default values
ACameraPawn::ACameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	CameraPivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	CameraPivot->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraPivot);
	Camera->SetRelativeLocation(FVector(0.0f, 0.0f, 900.0f));
	Camera->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
}

// Called when the game starts or when spawned
void ACameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
	this->EnableInput(GetWorld()->GetFirstPlayerController());
}

// Called every frame
void ACameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (FocusTarget)
	{
		this->SetActorLocation(FMath::Lerp(this->GetActorLocation(), FocusTarget->GetActorLocation(), DeltaTime * MovementSpeed));
	}

	Camera->SetRelativeLocation(-1.0f * Distance * Camera->GetForwardVector());
}

// Called to bind functionality to input
void ACameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Zoom", this, &ACameraPawn::Dolly);
	PlayerInputComponent->BindAxis("Orbit", this, &ACameraPawn::Orbit);
	PlayerInputComponent->BindAxis("Tilt", this, &ACameraPawn::Tilt);
	
	PlayerInputComponent->BindAction("Enable Camera", IE_Pressed, this, &ACameraPawn::EnableCameraMovement);
	PlayerInputComponent->BindAction("Enable Camera", IE_Released, this, &ACameraPawn::DisableCameraMovement);
}

void ACameraPawn::Dolly(float Val)
{
	if (Val)
	{
		Distance += MoveRate * Val;
		Distance = FMath::Clamp(Distance, MinDistance, MaxDistance);
	}
}

void ACameraPawn::Orbit(float Val)
{
	if (bCameraMovementEnabled && Val)
	{
		FRotator Rotation = Camera->RelativeRotation;
		Rotation.Yaw += TurnRate * Val;
		Camera->SetRelativeRotation(Rotation);
	}
}

void ACameraPawn::Tilt(float Val)
{
	if (bCameraMovementEnabled && Val)
	{
		FRotator Rotation = Camera->RelativeRotation;
		Rotation.Pitch = FMath::Clamp(Rotation.Pitch + PitchRate * Val, MinPitch, MaxPitch);
		Camera->SetRelativeRotation(Rotation);
	}
}

void ACameraPawn::EnableCameraMovement()
{
	bCameraMovementEnabled = true;
}

void ACameraPawn::DisableCameraMovement()
{
	bCameraMovementEnabled = false;
}

void ACameraPawn::SetNewFocus(AActor* NewFocus)
{
	if (NewFocus)
	{
		FocusTarget = NewFocus;
	}
}