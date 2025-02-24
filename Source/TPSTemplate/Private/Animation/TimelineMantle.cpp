// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/TimelineMantle.h"
#include "Animation/MantleSystem.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
// Sets default values
ATimelineMantle::ATimelineMantle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultScene"));
	TimelineRef = CreateDefaultSubobject<UTimelineComponent>(TEXT("MantleTimeline"));
	RootComponent = DefaultSceneRoot;

	// 루트 컴포넌트가 없다면 새로 생성
	if (!RootComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ATimelineMantle::RootComponent Is Null"));
		USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
		RootComponent = SceneRoot;
		SceneRoot->SetMobility(EComponentMobility::Stationary);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("ATimelineMantle::RootComponent Is Valid"));

	DefaultSceneRoot->SetMobility(EComponentMobility::Stationary);

}

// Called when the game starts or when spawned
void ATimelineMantle::BeginPlay()
{
	Super::BeginPlay();
	
	MantleSystem = GetOwner()->GetComponentByClass<UMantleSystem>();
	if (!MantleSystem)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("ATimelineMantle::BeginPlay()::MantleSystem Is NULL"));
		return;
	}

	// TODO: I was unsuccessful in creating and calling the timeline directly, 
	// so I replaced it with initializing the reference. I'll try again in the future
	//MantleCurve = LoadObject<UCurveFloat>(
	//	nullptr,
	//	TEXT("/Game/ThirdPerson/MantleSystem/MantleCurve")
	//);

	//if (MantleCurve)
	//{
	//	FOnTimelineFloat UpdateFunction;
	//	UpdateFunction.BindDynamic(this, &ATimelineMantle::MantleTimelineUpdate);

	//	FOnTimelineEvent FinishedFunction;
	//	FinishedFunction.BindDynamic(this, &ATimelineMantle::MantleTimelineFinished);

	//	TimelineRef->AddInterpFloat(MantleCurve, UpdateFunction);
	//	TimelineRef->SetTimelineFinishedFunc(FinishedFunction);
	//	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("ATimelineMantle::BeginPlay()::MantleCurve Is Valid"));
	//}
	//else
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("ATimelineMantle::BeginPlay()::MantleCurve Is NULL"));

	//}
}

// Called every frame
void ATimelineMantle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATimelineMantle::MantleTimelineUpdate(float BlendIn)
{
	if (MantleSystem)
	{
		MantleSystem->MantleUpdate(BlendIn);
	}
}

void ATimelineMantle::MantleTimelineFinished()
{
	if (MantleSystem)
	{
		MantleSystem->MantleEnd();
	}
}

