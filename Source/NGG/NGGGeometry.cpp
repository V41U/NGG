// Fill out your copyright notice in the Description page of Project Settings.


#include "NGGGeometry.h"


#include "Components/ChildActorComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ANGGGeometry::ANGGGeometry()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ANGGGeometry::SpawnChunk(FIntVector AdditionDirection)
{
	UChildActorComponent * CreatedChunk = nullptr;

	bool bLegalAction = true;

	//TODO: what is a bLegalAction?
	

	if (bLegalAction)
	{
		CreatedChunk = ItlCreateChunk();
	}

	if (CreatedChunk)
	{
		FVector ChunkLocation(0, 0, 0);
		while (Chunks.Contains(ChunkLocation))
		{
			if (IsValid(Chunks[ChunkLocation]))
			{
				ANGGChunk* SingleChunk = Cast<ANGGChunk>(Chunks[ChunkLocation]->GetChildActor());
				if (IsValid(SingleChunk))
					ChunkLocation += FVector(AdditionDirection) * FVector(SingleChunk->Extent);
			}
			else
			{
				Chunks.Remove(ChunkLocation);
				break;
			}
		}

		CreatedChunk->AddRelativeLocation(ChunkLocation);
		CreatedChunk->AttachTo(RootComponent);
		ANGGChunk* ActualCastedChunk = Cast<ANGGChunk>(CreatedChunk->GetChildActor());
		if (ActualCastedChunk)
			ActualCastedChunk->GenerateRandomizedMesh();

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

		for (auto & ChunkTuple : Chunks)
	{
		ANGGChunk* SingleChunk = Cast<ANGGChunk>(ChunkTuple.Value->GetChildActor());

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


	//TODO: DO THIS
	//for (auto& SingleStartChunk : StartChunks)
	//{
	//	if(SingleStartChunk)
	//		Chunks.Add(SingleStartChunk->GetActorLocation(), SingleStartChunk);
	//}
}

void ANGGGeometry::RemoveExistingChunks()
{
	for (auto & SingleChunk : Chunks)
	{
		if(IsValid(SingleChunk.Value))
			SingleChunk.Value->DestroyComponent();
	}
	Chunks.Empty();
}

UChildActorComponent * ANGGGeometry::ItlCreateChunk()
{
	UChildActorComponent* FreshObject = nullptr;
	TSubclassOf<UChildActorComponent> ClassToCreate = UChildActorComponent::StaticClass();
	if (ClassToCreate->IsValidLowLevelFast())
	{
		FString Chunk("Chunk");
		Chunk.Append(FString::FromInt(Chunks.Num()));
		FName ChunkFName(Chunk);

		FreshObject = NewObject<UChildActorComponent>(this, ChunkFName, RF_NoFlags, ClassToCreate->GetDefaultObject());
		FreshObject->SetChildActorClass(ANGGChunk::StaticClass());
		FreshObject->CreateChildActor();
		ANGGChunk* CreatedChunk = Cast<ANGGChunk>(FreshObject->GetChildActor());
		if (CreatedChunk)
		{
			CreatedChunk->CubeResolution = ChunkCubeResolution;
			CreatedChunk->Extent = ChunkSize;
			CreatedChunk->bUseSmoothSurface = bUseSmoothGeometryGeneration;
			CreatedChunk->FeatureSize = ChunkFeatureSize;
		}
	}

	return FreshObject;
}

// Called every frame
void ANGGGeometry::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

