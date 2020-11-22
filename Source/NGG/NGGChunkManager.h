// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NGGChunk.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NGGChunkManager.generated.h"

UCLASS()
class NGG_API ANGGChunkManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANGGChunkManager();

	// New Chunks automatically register themselves when being constructed
	// They also unregister themselves upon destruction!
	UFUNCTION(BlueprintCallable, Category = "MANAGEMENT")
		bool RegisterChunk(FName ChunkFName, ANGGChunk* Chunk);
	UFUNCTION(BlueprintCallable, Category = "MANAGEMENT")
		void UnregisterChunk(FName ChunkFName);
	// Returns whether the chunk is already registered
	UFUNCTION(BlueprintCallable, Category = "MANAGEMENT")
		bool IsRegistered(FName ChunkFName);

	// Can be called externally to edit the chunks according to the brush size and surface amount
	//
	// LocationInWS - Is the exact world space coordinate of where the edit is done
	// HitNormal - Is the normal of the hit (currently unused)
	// bAddTerrain - Controls whether to add surface(=true) or remove surface(=false)
	// BrushSize - Defines the size of the spherical brush
	// SurfaceAmount - Adjusts how large the change is on a per-tick basis
	//
	// returns the number of affected chunks that must be updated
	UFUNCTION(BlueprintCallable, Category = "MANAGEMENT")
		int32 EditChunks(FVector LocationInWS, FVector HitNormal, bool bAddTerrain, float BrushSize, float SurfaceAmount);

	// The Map of FNames to the transforms
	UPROPERTY()
		TMap<FName, ANGGChunk*> Chunks;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
