// Fill out your copyright notice in the Description page of Project Settings.


#include "FPGrabber.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"

//does nothing, just to mark out_variables
#define OUT

// Sets default values for this component's properties
UFPGrabber::UFPGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UFPGrabber::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Display, TEXT("Grabber reporting for duty!"))

	SetupInputComponent();
	FindPhysicsHandleComponent();
	
}


// Called every frame
void UFPGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//if physics handle exists, update position of the object we are holding
	if (PhysicsHandle != nullptr)
	{
		if (PhysicsHandle->GrabbedComponent != nullptr)
		{
			//UE_LOG(LogTemp, Display, TEXT("Updating location"))
			PhysicsHandle->SetTargetLocation(GetReachLineEnd());
		}
	}

	if(DebugLineEnabled)
		DrawDebugLine(GetWorld(), GetReachLineStart(), GetReachLineEnd(), FColor(255, 0, 0), false, -1.0f, '\000', 1.0f);
}


void UFPGrabber::SetupInputComponent()
{
	//Look for ActorInputComponent
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	if (InputComponent)
	{
		InputComponent->BindAction(FName("Grab"), EInputEvent::IE_Pressed, this, &UFPGrabber::HandleGrab);
		
	
		/*InputComponent->BindAction(FName("Grab"), EInputEvent::IE_Pressed, this, &UFPGrabber::Grab);
		InputComponent->BindAction(FName("Grab"), EInputEvent::IE_Released, this, &UFPGrabber::Release);*/
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Error! Could not find input component at object %s"), *(GetOwner()->GetName()))
	}
	//
}

void UFPGrabber::FindPhysicsHandleComponent()
{
	//Look for attached physics handle
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Error! Could not find physics handle component at object %s!"), *(GetOwner()->GetName()))
	}
	else
	{
		PhysicsHandle->SetInterpolationSpeed(300.0f);
	}
	//
}

void UFPGrabber::HandleGrab()
{
	if (PhysicsHandle->GrabbedComponent != nullptr)
		Release();
	else
		Grab();
	
}

void UFPGrabber::Grab()
{
	//UE_LOG(LogTemp, Display, TEXT("Grabbed something."))

	//Line trace and see if we reach any actors with physics body collision channel set
	FHitResult HitResult = GetFirstPhysicsBodyInReach();
	TempHitResult = HitResult;
	UPrimitiveComponent* ComponentToGrab = HitResult.GetComponent();
	GrabbedObject = HitResult.GetActor();

	AActor* HitActor = HitResult.GetActor();
	//If we hit something, then attach to physics handle
	if (HitActor)
	{

		
		if (!PhysicsHandle) { return; }
		FRotator ObjectRotation = ComponentToGrab->GetOwner()->GetActorRotation();
		FVector GrabbedLocation = ComponentToGrab->GetOwner()->GetActorLocation();
		PhysicsHandle->GrabComponentAtLocationWithRotation(ComponentToGrab, NAME_None, GrabbedLocation, ObjectRotation);

		MemoGrabbedComponent = ComponentToGrab;
		MemoBoneName = HitResult.BoneName;
		MemoGrabbedLocation = GrabbedLocation;
		MemoObjectRotation = ObjectRotation;

		FString GrabbedComponentName = ComponentToGrab->GetOwner()->GetName();
		//UE_LOG(LogTemp, Display, TEXT("Grabbed object: %s"), *GrabbedComponentName)
	}


	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Could not find grabbable actor."))
	//}
}

void UFPGrabber::Release()
{
	//UE_LOG(LogTemp, Display, TEXT("Released something."))
	if (!PhysicsHandle) { return; }
	PhysicsHandle->ReleaseComponent();
	OnRelease.Broadcast();
	//set object to null AFTER OnRelease.Broadcast(), otherwise the Blueprint can't move the object to the dest portal.
	GrabbedObject = nullptr;
}

void UFPGrabber::ReleaseTemporarily() 
{
	if (!PhysicsHandle) { return; }
	PhysicsHandle->ReleaseComponent();
}

void UFPGrabber::GrabAfterTemporaryRelease() 
{
	UE_LOG(LogTemp, Display, TEXT("Grabbing %s"), *(MemoGrabbedComponent->GetName()));
	PhysicsHandle->GrabComponentAtLocationWithRotation(MemoGrabbedComponent, MemoBoneName, MemoGrabbedLocation, MemoObjectRotation);
}

const FHitResult UFPGrabber::GetFirstPhysicsBodyInReach()
{
	//detect collisions of specific object types we can grab
	FHitResult Hit;
	FCollisionQueryParams TraceParams{ TEXT(""), false, GetOwner() };
	GetWorld()->LineTraceSingleByObjectType(
		OUT Hit,
		GetReachLineStart(),
		GetReachLineEnd(),
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParams
	);
	return Hit;
}

FVector UFPGrabber::GetReachLineStart()
{
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);
	//UE_LOG(LogTemp, Warning, TEXT("Line Start: %s, %s"), *PlayerViewPointLocation.ToString(), *PlayerViewPointRotation.ToString())
	return PlayerViewPointLocation;
}

FVector UFPGrabber::GetReachLineEnd()
{
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);
	FVector LineTraceEnd = PlayerViewPointLocation + (PlayerViewPointRotation.Vector() * Reach);
	return LineTraceEnd;
}