// Fill out your copyright notice in the Description page of Project Settings.


#include "NGGGeometry.h"


#include "Kismet/GameplayStatics.h"

// Sets default values
ANGGGeometry::ANGGGeometry()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ANGGGeometry::SpawnChunk(FIntVector AdditionDirection)
{
	ANGGChunk * CreatedChunk = nullptr;

	bool bLegalAction = true;
	

	if (bLegalAction)
	{
		CreatedChunk = ItlCreateChunk();
	}

	if (CreatedChunk)
	{
		FVector ChunkLocation(0, 0, 0);
		while (Chunks.Contains(ChunkLocation))
			ChunkLocation += FVector(AdditionDirection);
		CreatedChunk->SetActorLocation(GetActorLocation() + ChunkLocation * FVector(ChunkSize));
		CreatedChunk->GenerateRandomizedMesh();
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);

		CreatedChunk->AttachToActor(this, AttachmentRules);
		Chunks.Add(ChunkLocation, CreatedChunk);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-42069, 10, FColor::Red, FString::Printf(TEXT("Failed to create chunk for direction: %s"), *AdditionDirection.ToString()));
	}
}

void ANGGGeometry::EditTerrain(FVector LocationInWS, FVector HitNormal, bool bAddTerrain, float BrushSize, float SurfaceAmount)
{
	TArray<ANGGChunk*> ChunksToEdit;

	FVector XVector(1, 0, 0);
	FVector YVector(0, 1, 0);
	FVector ZVector(0, 0, 1);

	//TODO: TEST THIS LOGIC!
	// To clear up: How to create hierarchy where the spawned actors are not separated from the geometry actor!

	for (auto & ChunkTuple : Chunks)
	{
		//X-Axis
		bool bAddThisTuple = LocationInWS.X > ChunkTuple.Key.X && LocationInWS.X < (ChunkTuple.Key + FVector(ChunkTuple.Value->Extent)).X;
		bAddThisTuple = bAddThisTuple || (LocationInWS + XVector * BrushSize).X > ChunkTuple.Key.X && (LocationInWS + XVector * BrushSize).X < (ChunkTuple.Key + FVector(ChunkTuple.Value->Extent)).X;
		bAddThisTuple = bAddThisTuple || (LocationInWS - XVector * BrushSize).X > ChunkTuple.Key.X && (LocationInWS - XVector * BrushSize).X < (ChunkTuple.Key + FVector(ChunkTuple.Value->Extent)).X;
		//Y-Axis
		bAddThisTuple = bAddThisTuple || LocationInWS.Y > ChunkTuple.Key.Y && LocationInWS.Y < (ChunkTuple.Key + FVector(ChunkTuple.Value->Extent)).Y;
		bAddThisTuple = bAddThisTuple || (LocationInWS + YVector * BrushSize).Y > ChunkTuple.Key.Y && (LocationInWS + YVector * BrushSize).Y < (ChunkTuple.Key + FVector(ChunkTuple.Value->Extent)).Y;
		bAddThisTuple = bAddThisTuple || (LocationInWS - YVector * BrushSize).Y > ChunkTuple.Key.Y && (LocationInWS - YVector * BrushSize).Y < (ChunkTuple.Key + FVector(ChunkTuple.Value->Extent)).Y;
		//Z-Axis
		bAddThisTuple = bAddThisTuple || LocationInWS.Z > ChunkTuple.Key.Z && LocationInWS.Z < (ChunkTuple.Key + FVector(ChunkTuple.Value->Extent)).Z;
		bAddThisTuple = bAddThisTuple || (LocationInWS + ZVector * BrushSize).Z > ChunkTuple.Key.Z && (LocationInWS + ZVector * BrushSize).Z < (ChunkTuple.Key + FVector(ChunkTuple.Value->Extent)).Z;
		bAddThisTuple = bAddThisTuple || (LocationInWS - ZVector * BrushSize).Z > ChunkTuple.Key.Z && (LocationInWS - ZVector * BrushSize).Z < (ChunkTuple.Key + FVector(ChunkTuple.Value->Extent)).Z;

		if (bAddThisTuple)
			ChunksToEdit.Add(ChunkTuple.Value);
	}

	for (auto& SingleChunk : ChunksToEdit)
	{
		SingleChunk->EditTerrain(LocationInWS, HitNormal, bAddTerrain, BrushSize, SurfaceAmount);
	}

	
}

void ANGGGeometry::AddChunkAtXAxis()
{
	SpawnChunk(FIntVector(1, 0, 0));
}

void ANGGGeometry::AddChunkAtYAxis()
{
	SpawnChunk(FIntVector(0, 1, 0));
}

void ANGGGeometry::AddChunkAtZAxis()
{
	SpawnChunk(FIntVector(0, 0, 1));
}

void ANGGGeometry::AddChunkAtXYAxis()
{
	SpawnChunk(FIntVector(1, 1, 0));
}

void ANGGGeometry::AddChunkAtXYZAxis()
{
	SpawnChunk(FIntVector(1, 1, 1));
}

// Called when the game starts or when spawned
void ANGGGeometry::BeginPlay()
{
	Super::BeginPlay();

	for (auto& SingleStartChunk : StartChunks)
	{
		if(SingleStartChunk)
			Chunks.Add(SingleStartChunk->GetActorLocation(), SingleStartChunk);
	}
}

void ANGGGeometry::RemoveExistingChunks()
{
	for (auto & SingleChunk : Chunks)
	{
		if(SingleChunk.Value)
			SingleChunk.Value->Destroy();
		SingleChunk.Value = nullptr;
	}
	Chunks.Empty();
}

ANGGChunk * ANGGGeometry::ItlCreateChunk()
{
	FTransform SpawnTransform;
	ANGGChunk* CreatedChunk = Cast<ANGGChunk>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ANGGChunk::StaticClass(), SpawnTransform));

	if (CreatedChunk != nullptr)
	{
		CreatedChunk->CubeResolution = ChunkCubeResolution;
		CreatedChunk->Extent = ChunkSize;
		CreatedChunk->bUseSmoothSurface = bUseSmoothGeometryGeneration;
		CreatedChunk->FeatureSize = ChunkFeatureSize;

		UGameplayStatics::FinishSpawningActor(CreatedChunk, SpawnTransform);
	}

	return CreatedChunk;
}

// Called every frame
void ANGGGeometry::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

