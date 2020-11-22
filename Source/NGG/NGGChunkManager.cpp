// Fill out your copyright notice in the Description page of Project Settings.


#include "NGGChunkManager.h"

// Sets default values
ANGGChunkManager::ANGGChunkManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

bool ANGGChunkManager::RegisterChunk(FName ChunkFName, ANGGChunk* Chunk)
{
	if (Chunks.Contains(ChunkFName))
		return false;
	
	Chunks.Add(ChunkFName, Chunk);

	return true;
}

void ANGGChunkManager::UnregisterChunk(FName ChunkFName)
{
	Chunks.Remove(ChunkFName);
}

bool ANGGChunkManager::IsRegistered(FName ChunkFName)
{
	return Chunks.Contains(ChunkFName);
}

int32 ANGGChunkManager::EditChunks(FVector LocationInWS, FVector HitNormal, bool bAddTerrain, float BrushSize, float SurfaceAmount)
{
	TArray<ANGGChunk*> ChunksToEdit;

	FVector XVector(1, 0, 0);
	FVector YVector(0, 1, 0);
	FVector ZVector(0, 0, 1);

	for (auto & ChunkTuple : Chunks)
	{
		ANGGChunk* SingleChunk = Cast<ANGGChunk>(ChunkTuple.Value);

		if (IsValid(SingleChunk))
		{
			FVector ChunkLocation = SingleChunk->GetActorLocation();

			//X-Axis
			bool bXAddThisTuple = LocationInWS.X > ChunkLocation.X && LocationInWS.X < (ChunkLocation + FVector(SingleChunk->Extent)).X;
			bXAddThisTuple = bXAddThisTuple || (LocationInWS + XVector * BrushSize).X > ChunkLocation.X && (LocationInWS + XVector * BrushSize).X < (ChunkLocation + FVector(SingleChunk->Extent)).X;
			bXAddThisTuple = bXAddThisTuple || (LocationInWS - XVector * BrushSize).X > ChunkLocation.X && (LocationInWS - XVector * BrushSize).X < (ChunkLocation + FVector(SingleChunk->Extent)).X;
			//Y-Axis
			bool bYAddThisTuple = LocationInWS.Y > ChunkLocation.Y && LocationInWS.Y < (ChunkLocation + FVector(SingleChunk->Extent)).Y;
			bYAddThisTuple = bYAddThisTuple || (LocationInWS + YVector * BrushSize).Y > ChunkLocation.Y && (LocationInWS + YVector * BrushSize).Y < (ChunkLocation + FVector(SingleChunk->Extent)).Y;
			bYAddThisTuple = bYAddThisTuple || (LocationInWS - YVector * BrushSize).Y > ChunkLocation.Y && (LocationInWS - YVector * BrushSize).Y < (ChunkLocation + FVector(SingleChunk->Extent)).Y;
			//Z-Axis
			bool bZAddThisTuple = LocationInWS.Z > ChunkLocation.Z && LocationInWS.Z < (ChunkLocation + FVector(SingleChunk->Extent)).Z;
			bZAddThisTuple = bZAddThisTuple || (LocationInWS + ZVector * BrushSize).Z > ChunkLocation.Z && (LocationInWS + ZVector * BrushSize).Z < (ChunkLocation + FVector(SingleChunk->Extent)).Z;
			bZAddThisTuple = bZAddThisTuple || (LocationInWS - ZVector * BrushSize).Z > ChunkLocation.Z && (LocationInWS - ZVector * BrushSize).Z < (ChunkLocation + FVector(SingleChunk->Extent)).Z;

			if (bXAddThisTuple && bYAddThisTuple && bZAddThisTuple)
				ChunksToEdit.Add(SingleChunk);
		}
		else
			Chunks.Remove(ChunkTuple.Key);

	}

	for (auto& SingleChunk : ChunksToEdit)
	{
		SingleChunk->EditTerrainOVERRIDE(LocationInWS, HitNormal, bAddTerrain, BrushSize, SurfaceAmount);
	}

	return ChunksToEdit.Num();
}

// Called when the game starts or when spawned
void ANGGChunkManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANGGChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

