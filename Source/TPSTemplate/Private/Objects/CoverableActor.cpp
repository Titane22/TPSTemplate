// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/CoverableActor.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"

// Sets default values
ACoverableActor::ACoverableActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultScene = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene"));
	RootComponent = DefaultScene;
	
	CoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoverMesh"));
	CoverMesh->SetupAttachment(RootComponent);

	// Initialize Arrow Component
	ArrowComponentNorth = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow Component North"));
	ArrowComponentNorth->SetupAttachment(CoverMesh);
	ArrowComponentNorth->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f)); // 북쪽

	ArrowComponentSouth = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow Component South"));
	ArrowComponentSouth->SetupAttachment(CoverMesh);
	ArrowComponentSouth->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f)); // 남쪽

	ArrowComponentEast = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow Component East"));
	ArrowComponentEast->SetupAttachment(CoverMesh);
	ArrowComponentEast->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f)); // 동쪽

	ArrowComponentWest = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow Component West"));
	ArrowComponentWest->SetupAttachment(CoverMesh);
	ArrowComponentWest->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // 서쪽

	CoverPromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Cover Prompt Widget"));
	CoverPromptWidget->SetupAttachment(CoverMesh);

	CoverPromptWidget->SetWidgetSpace(EWidgetSpace::World);
	CoverPromptWidget->SetDrawSize(FVector2D(150.0f, 150.f));
	CoverPromptWidget->SetVisibility(false);

	// Initialize Box Collision
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	BoxCollision->SetupAttachment(RootComponent);
	BoxCollision->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f)); // Example size, adjust as needed
}

// Called when the game starts or when spawned
void ACoverableActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (CoverPromptWidget)
	{
		CoverPromptWidget->SetVisibility(false);
	}	

	if (CoverPromptWidgetClass)
	{
		CoverPromptWidget->SetWidgetClass(CoverPromptWidgetClass);
	}
}

// Called every frame
void ACoverableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ACoverableActor::IsOccupied()
{
	return bIsOccupied;
}

void ACoverableActor::ShowCoverPrompt(bool bShow)
{
	if (CoverPromptWidget)
	{
		CoverPromptWidget->SetVisibility(bShow);
	}
}

void ACoverableActor::UpdateCoverPromptTransform(FVector Location, FRotator Rotaion)
{
	if (CoverPromptWidget)
	{
		CoverPromptWidget->SetWorldLocation(Location);
		CoverPromptWidget->SetWorldRotation(Rotaion);
	}
}

