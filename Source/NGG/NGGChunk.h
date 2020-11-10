// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NGGAlgorithmBase.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NGGChunk.generated.h"

UENUM(BlueprintType)
enum class EUsedAlgorithm :uint8
{
	MARCHING_CUBE			UMETA(DisplayName = "Marching Cube"),
	DUAL_CONTOURING		UMETA(DisplayName = "Dual Contouring"),
};

UCLASS()
class NGG_API ANGGChunk : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANGGChunk();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
		EUsedAlgorithm Algorithm = EUsedAlgorithm::MARCHING_CUBE;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY()
	UNGGAlgorithmBase* AlgorithmImpl = nullptr;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
