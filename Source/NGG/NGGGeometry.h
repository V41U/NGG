// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NGGChunk.h"

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

	// TODO: SUPPORT THIS?
	//// If given, then the mesh will be analyzed and converted into a chunkable
	//// geometry that is split into the separate chunks
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NGG_General")
	//	UStaticMesh* StartMesh = nullptr;

	// Defines the size of individual chunks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NGG_General",
		meta = (ClampMin = "1.0"))
		FIntVector ChunkSize = FIntVector(1000, 1000, 1000);

	// Defines the cube resolution of all spawned chunks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NGG_General",
		meta = (ClampMin = "1.0"))
		FIntVector ChunkCubeResolution = FIntVector(100, 100, 100);

	// Defines whether the individual triangles are generated with a fixed surface split of 0.5
	// or whether the edges should calculate a difference of a sampling and create a smooth surface
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NGG_General")
		bool bUseSmoothGeometryGeneration = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NGG_General")
		float ChunkFeatureSize = 50.0f;

	// Defines how many chunks are created in the given direction
	// ATTENTION: Spawning chunks like this currently only supports positive directions
	// The SpawnChunk method supports both direction if you want to add chunks at a later stage. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NGG_General",
		meta = (ClampMin = "1.0"))
		FIntVector NumberOfChunks = FIntVector(1, 1, 1);
	
	// You can add existing chunks to this handler to allow for terrain editing over multiple chunks
	// ATTENTION: These chunks are not taken into consideration when calling SpawnChunk so there may be some overlapping between spawned chunks and added chunks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NGG_General")
		TArray<ANGGChunk*> StartChunks;
	UPROPERTY(BlueprintReadOnly, Category = "NGG_General")
		TMap<FVector, UChildActorComponent*> Chunks;

	// How many chunks have been added to the actor before starting to create chunks (= initial size of Chunks)
	UPROPERTY(BlueprintReadOnly, Category = "NGG_General")
		int32 InitialChunksNum = 0;

	// Number of chunks added in the positive directions
	UPROPERTY(BlueprintReadOnly, Category = "NGG_General")
		FIntVector MaxChunks = FIntVector(0,0,0);
	// Number of chunks added in the negative directions
	UPROPERTY(BlueprintReadOnly, Category = "NGG_General")
		FIntVector MinChunks = FIntVector(0, 0, 0);


	// keep in mind that the vector must have exactly one axis != 0
	// Spawns a chunk in the given direction (+1 at x = adds this chunk to the last chunk in the positive x direction)
	UFUNCTION(BlueprintCallable, Category = "NGG_General")
		void SpawnChunk(FIntVector AdditionDirection);
	UFUNCTION(BlueprintCallable, Category = "NGG_General")
		void EditTerrain(FVector LocationInWS, FVector HitNormal, bool bAddTerrain, float BrushSize, float SurfaceAmount);
	UFUNCTION(CallInEditor, Category = "NGG_General")
		void AddChunkAtXAxis();
	UFUNCTION(CallInEditor, Category = "NGG_General")
		void AddChunkAtYAxis();
	UFUNCTION(CallInEditor, Category = "NGG_General")
		void AddChunkAtZAxis();
	UFUNCTION(CallInEditor, Category = "NGG_General")
		void AddChunkAtXYAxis();
	UFUNCTION(CallInEditor, Category = "NGG_General")
		void AddChunkAtXYZAxis();
	UFUNCTION(CallInEditor, Category = "NGG_General")
		void RemoveExistingChunks();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

	UFUNCTION()
		UChildActorComponent* ItlCreateChunk();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
