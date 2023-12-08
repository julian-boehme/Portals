// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/InputComponent.h"


//needs to be the last include
#include "FPGrabber.generated.h"

//FOnRelease is used to tell the blueprint that the object that was held is now free 
//and should be moved to the dest portal, if it went through the src portal.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRelease);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTALS_API UFPGrabber : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFPGrabber();

private:
	//how far can you reach
	float Reach = 230.f;
	UPhysicsHandleComponent* PhysicsHandle{ nullptr };
	UInputComponent* InputComponent{ nullptr };
	FHitResult TempHitResult;
	
	bool DebugLineEnabled = false;

	void HandleGrab();
	void Grab();
	void Release();
	void FindPhysicsHandleComponent();
	void SetupInputComponent();
	//Return hit for first physics body in reach
	const FHitResult GetFirstPhysicsBodyInReach();

	//Returns current start of reach line
	FVector GetReachLineStart();

	//Returns current end of reach line
	FVector GetReachLineEnd();

	FRotator MemoObjectRotation;
	UPrimitiveComponent* MemoGrabbedComponent;
	FName MemoBoneName;
	FVector MemoGrabbedLocation;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadOnly)
	AActor* GrabbedObject{ nullptr };

	UPROPERTY(BlueprintAssignable)
	FOnRelease OnRelease;

	UPROPERTY(BlueprintReadWrite)
	bool DisableGrabbing = false;

	UFUNCTION(BlueprintCallable, Category = "My Functions")
	void ReleaseTemporarily();

	UFUNCTION(BlueprintCallable, Category = "My Functions")
	void GrabAfterTemporaryRelease();

};
