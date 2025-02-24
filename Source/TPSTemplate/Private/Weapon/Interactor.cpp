// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Interactor.h"
#include "Weapon/InteractionComponent.h"
#include "Weapon/Interaction.h"
#include "../TPSTemplateCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UInteractor::UInteractor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInteractor::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UInteractor::StopLastIntteraction()
{
	InteractionActor = nullptr;
	InteractionBP->Marked = false;
	InteractionBP = nullptr;
	HasInteraction = false;
}

void UInteractor::StartInteraction(AController* Interactor)
{
	if (AInteraction* Interaction = Cast<AInteraction>(InteractionActor))
	{
		Interaction->Interact(Interactor);
		Reset();
	}
}

void UInteractor::Reset()
{
	InteractionActor = nullptr;
	InteractionBP = nullptr;
	HasInteraction = false;
}

EInteractiveType UInteractor::GetInteractionType()
{
	if (!InteractionActor)
		return EInteractiveType();
	return InteractionActor->GetComponentByClass<UInteractionComponent>()->InteractionType;
}

// Called every frame
void UInteractor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (!InteractorActive)
		return;

	switch (InteractionMethod)
	{
	case EInteractionMethod::Camera:
		FVector StartLocation = CharacterRef->GetFollowCamera()->GetComponentLocation();
		FVector EndLocation = StartLocation + (CharacterRef->GetFollowCamera()->GetForwardVector() * DetectionDistance);
		// Sphere Trace 설정
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(CharacterRef); 
		
		TArray<FHitResult> HitResults;
		
		// Sphere Trace 실행
		//DrawDebugSphere(
		//	GetWorld(),                          // WorldContextObject
		//	StartLocation,                       // Start
		//	10.0f,                              // Radius
		//	UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),  // TraceChannel
		//	FColor::Red,                              // bTraceComplex
		//	false,              // DrawDebugType
		//	5.0f,                               // bIgnoreSelf
		//	0,                // TraceHitColor
		//	1.0f                                // DrawTime
		//);

		// TODO: Change mechanism for recognition the item
		FHitResult HitResult;
		bool bHit = UKismetSystemLibrary::SphereTraceSingle(
			GetWorld(),                          // WorldContextObject
			StartLocation,                       // Start
			EndLocation,                         // End
			10.0f,                              // Radius
			UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),  // TraceChannel
			false,                              // bTraceComplex
			TArray<AActor*>{ CharacterRef },    // ActorsToIgnore
			EDrawDebugTrace::None,              // DrawDebugType
			HitResult,                          // OutHit
			true,                               // bIgnoreSelf
			FLinearColor::Red,                  // TraceColor
			FLinearColor::Green,                // TraceHitColor
			5.0f                                // DrawTime
		);
		if (bHit) 
		{
			if (AActor* HitActor = HitResult.GetActor())
			{
				if (UInteractionComponent* InteractionComp = HitActor->FindComponentByClass<UInteractionComponent>())
				{
					if (InteractionComp->IsActive())
					{
						if (InteractionActor == nullptr || HitActor != InteractionActor)
						{
							if (HasInteraction)
							{
								StopLastIntteraction();
							}
							InteractionActor = HitActor;
							InteractionBP = Cast<AInteraction>(InteractionActor);
							if (InteractionBP)
							{
								InteractionBP->Marked = true;
								HasInteraction = true;
							}
						}
					}
					else
					{
						if (HasInteraction)
						{
							StopLastIntteraction();
						}
					}
				}
				else
				{
					if (HasInteraction)
					{
						StopLastIntteraction();
					}
				}
			}
		}
		else
		{
			if (HasInteraction)
			{
				StopLastIntteraction();
			}
		}
		break;
	}
}

