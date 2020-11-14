// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NGGGeometry.generated.h"

UCLASS()
class NGG_API ANGGGeometry : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANGGGeometry();

	// If given, then the mesh will be analyzed and converted into a chunkable
	// geometry that is split into the separate chunks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NGG_General")
		UStaticMesh* StartMesh = nullptr;

	// Defines the size of individual chunks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NGG_General",
		meta = (ClampMin = "1.0"))
		FIntVector ChunkSize = FIntVector(1000, 1000, 1000);

	// Defines how many chunks are created in the given direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NGG_General",
		meta = (ClampMin = "1.0"))
		FIntVector NumberOfChunks = FIntVector(1, 1, 1);
	
	// If true then new chunks are generated when the player 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NGG_General")
		TArray<NGGChunk*> Chunks;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
