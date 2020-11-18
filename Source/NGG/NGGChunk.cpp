// Fill out your copyright notice in the Description page of Project Settings.


#include "NGGChunk.h"

//DEBUG STUFF TODO: DELETE ME LATER!!
#include "DrawDebugHelpers.h"

// Sets default values
ANGGChunk::ANGGChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = ProceduralMeshComponent;

}

void ANGGChunk::GenerateRandomizedMesh()
{
	ItlSetup(false);

	int iIndex = 0;
	int a = 0;
	// The data points for terrain are stored at the corners of our "cubes", so the terrainMap needs to be 1 larger
	// than the width/height of our mesh.
	for (int x = 0; x < Increment.X + 1; ++x) {
		for (int y = 0; y < Increment.Y + 1; ++y) {
			for (int z = 0; z < Increment.Z + 1; ++z) {
				// Get a terrain height using good old Perlin noise.
				float thisHeight = Extent.Z *
					((FMath::PerlinNoise3D(FVector(
					(((float)x * (float)CubeResolution.X + GetActorLocation().X + 0.001f) / CubeResolution.X) / FeatureSize,
					(((float)y  * (float)CubeResolution.Y + GetActorLocation().Y + 0.001f) / CubeResolution.Y) / FeatureSize,
						GetActorLocation().Z)
					) + 1.0f) / 2.0f);

				iIndex = z + y * (Increment.Z + 1) + x * (Increment.Z + 1) * (Increment.Y + 1);
				VoxelData[iIndex] = z * CubeResolution.Z - thisHeight;
			}
		}
	}

	UpdateChunk();
}

void ANGGChunk::EditTerrain(FVector LocationInWS, FVector HitNormal, bool bAddTerrain, float BrushSize, float SurfaceAmount)
{
	int BuildModifier = bAddTerrain ? -1 : 1;

	//TODO: Handle LocationInWS outside of extent that should still affect this individual chunk
	// There is already some functionality existing, however there's still some weird iffy behavior

	TArray<int32> Indices;
	FVector Location = LocationInWS - GetActorLocation();

	int32 LocationWSIndex = ItlGetVoxelIndexForVector(Location);
	int32 Index;

	int32 XOffset = FMath::RoundToInt(BrushSize / CubeResolution.X) * (Increment.Z + 1) * (Increment.Y + 1);
	int32 YOffset = FMath::RoundToInt(BrushSize / CubeResolution.Y) * (Increment.Z + 1);
	int32 ZOffset = FMath::RoundToInt(BrushSize / CubeResolution.Z);
	
	for (int32 x = - XOffset; x <= + XOffset; x+= (Increment.Z + 1) * (Increment.Y + 1))
	{
		for (int32 y = - YOffset; y <= + YOffset; y+= (Increment.Z + 1))
		{
			for (int32 z = - ZOffset; z <= + ZOffset; z++)
			{
				FVector OffsetPoint;
				if (ItlGetVectorForVoxelIndex(LocationWSIndex + x + y + z, OffsetPoint))
				{
					float Distance = FVector::Distance(OffsetPoint, Location);
					Index = ItlGetVoxelIndexForVector(OffsetPoint);
					if (Distance < BrushSize && Index >= 0 && Index < VoxelData.Num() && !Indices.Contains(Index))
					{
						float ModificationAmount = SurfaceAmount /*/ Distance*/ * BuildModifier;
						VoxelData[Index] += ModificationAmount;
						Indices.AddUnique(Index);
					}
				}
				
			}
		}
	}

	//only update terrain if actually anything changed :) 
	if(Indices.Num() > 0)
		UpdateChunk();
}

void ANGGChunk::UpdateChunk()
{
	if (IsValid(ProceduralMeshComponent))
	{
		ItlClearMeshData();

		GenerateGeometry();

		ProceduralMeshComponent->CreateMeshSection(0,
			Vertices,
			Triangles,
			Normals,
			UVs,
			VertexColors,
			Tangents,
			true);
	}
}

// Called when the game starts or when spawned
void ANGGChunk::BeginPlay()
{
	Super::BeginPlay();
	

	ItlSetup(true);
	GenerateRandomizedMesh();
}

void ANGGChunk::OnConstruction(const FTransform & Transform)
{
	bVoxelDataSetup = false;
}

void ANGGChunk::GenerateGeometry()
{

	int a = 0;
	for (int x = 0; x < Increment.X; ++x)
	{
		for (int y = 0; y < Increment.Y; ++y)
		{
			for (int z = 0; z < Increment.Z; ++z)
			{
				a++;
				// March the cube for each position
				MarchCube(FVector(x, y, z));
			}
		}
	}

	GEngine->AddOnScreenDebugMessage(-10, 1.f, FColor::Yellow, FString::Printf(TEXT("iterationen: %i"), a));
}

void ANGGChunk::MarchCube(FVector Position)
{
	// Sample terrain values at each corner of the cube.
	TArray<float> CubeValues;
	CubeValues.SetNum(8);

	int iIdx;
	int iIdxOffset;
	for (int i = 0; i < 8; i++)
	{
		iIdx = Position.Z + Position.Y * (Increment.Z + 1) + Position.X * (Increment.Z + 1) * (Increment.Y + 1);
		iIdxOffset = CornerTable[i].Z + CornerTable[i].Y * (Increment.Z + 1) + CornerTable[i].X * (Increment.Y + 1) * (Increment.Z + 1);
		CubeValues[i] = VoxelData[iIdx + iIdxOffset];
	}

	int32 TableIndex = 0;

	// If it is, use bit-magic to the set the corresponding bit to 1. So if only the 3rd point in the cube was below
	// the surface, the bit would look like 00100000, which represents the integer value 32.
	for (int i = 0; i < 8; i++)
		if (CubeValues[i] > TerrainSurface)
			TableIndex |= 1 << i;

	if (TableIndex == 0 || TableIndex == 255)
		return;

	int32 EdgeIdx = 0;
	for (int iTriangle = 0; iTriangle < 5; ++iTriangle) {
		for (int iVertex = 0; iVertex < 3; ++iVertex) {

			// Get the current indice. We increment triangleIndex through each loop.
			int TriIndex = TriangleTable[255 - TableIndex][EdgeIdx];

			// If the current edgeIndex is -1, there are no more indices and we can exit the function.
			if (TriIndex == -1)
				return;

			// Get the vertices for the start and end of this edge.
			FVector Resolution(CubeResolution);
			FVector Vertex1 = Position * Resolution + CornerTable[EdgeTable[TriIndex][0]] * Resolution;
			FVector Vertex2 = Position * Resolution + CornerTable[EdgeTable[TriIndex][1]] * Resolution;

			FVector VertexPosition;
			if (bUseSmoothSurface)
			{
				// Get the terrain values at either end of our current edge from the cube array created above.
				float vert1Sample = CubeValues[EdgeTable[TriIndex][0]];
				float vert2Sample = CubeValues[EdgeTable[TriIndex][1]];

				// Calculate the difference between the terrain values.
				float difference = vert2Sample - vert1Sample;

				// If the difference is 0, then the terrain passes through the middle.
				if (difference == 0)
					difference = TerrainSurface;
				else
					difference = (TerrainSurface - vert1Sample) / difference;

				// Calculate the point along the edge that passes through.
				VertexPosition = Vertex1 + ((Vertex2 - Vertex1) * difference);
			}
			else // Get the midpoint of this edge.
				VertexPosition = (Vertex1 + Vertex2) / 2.f;

			// TODO: determine correct section


			// Add to our vertices and triangles list and incremement the edgeIndex.
			Vertices.Add(VertexPosition);
			Triangles.Add(Vertices.Num() - 1);
			Normals.Add(FVector(0, 0, 1));
			UVs.Add(FVector2D(((int32)VertexPosition.X % (int32)(Extent.X + 1)) / (Extent.X + 1), ((int32)VertexPosition.Y % (int32)(Extent.Y + 1)) / (Extent.Y + 1)));
			FProcMeshTangent tangent(FVector(1, 0, 0), false);
			Tangents.Add(tangent);
			VertexColors.Add(FColor(1, 1, 1));
			EdgeIdx++;

		}
	}
}


void ANGGChunk::ItlSetup(bool bDestroyIfNecessary)
{
	if (!bVoxelDataSetup)
	{
		bool bSetup = (Extent.X % CubeResolution.X) == 0;
		bSetup = bSetup && (Extent.Y % CubeResolution.Y) == 0;
		bSetup = bSetup && (Extent.Z % CubeResolution.Z) == 0;

		if (bSetup)
		{
			Increment = FIntVector(FVector(Extent) / FVector(CubeResolution));
			VoxelDataLength = (Extent.X / CubeResolution.X + 1) * (Extent.Y / CubeResolution.Y + 1) * (Extent.Z / CubeResolution.Z + 1);
			VoxelData.Empty();
			VoxelData.SetNum(VoxelDataLength);
			bVoxelDataSetup = true;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(42069, 10.0f, FColor::Red, TEXT("NGG_Chunk Error: Cube resolution does not fit exactly into given Extent. Ensure that for each axis the % equals 0!"));
			if (bDestroyIfNecessary)
				Destroy();
		}
	}
}

void ANGGChunk::ItlClearMeshData()
{
	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	UVs.Empty();
	VertexColors.Empty();
	Tangents.Empty();
}

int32 ANGGChunk::ItlGetVoxelIndexForVector(const FVector & Location)
{
	int32 Index = 0;
	
	float AdjustedX;
	float AdjustedY;
	float AdjustedZ;

	AdjustedX = FMath::RoundToInt(Location.X / CubeResolution.X);
	AdjustedY = FMath::RoundToInt(Location.Y / CubeResolution.Y);
	AdjustedZ = FMath::RoundToInt(Location.Z / CubeResolution.Z);

	Index = AdjustedZ + AdjustedY * (Increment.Z + 1) + AdjustedX * (Increment.Z + 1) * (Increment.Y + 1);
	

	return Index;
}

bool ANGGChunk::ItlGetVectorForVoxelIndex(const int32 VectorIndex, FVector& OutVec)
{
	OutVec = FVector(-1, -1, -1);
	bool bContained = VectorIndex >= 0 && VectorIndex < VoxelData.Num();
	
	if (bContained)
	{
		int X= 0.0f;
		int Y= 0.0f;
		int Z= 0.0f;

		X = FMath::FloorToInt(VectorIndex / ((Increment.Z + 1) * (Increment.Y + 1)));
		Y = FMath::FloorToInt((VectorIndex - X * (Increment.Z + 1) * (Increment.Y + 1)) / (Increment.Z + 1));
		Z = VectorIndex - X * (Increment.Z + 1) * (Increment.Y + 1) - Y * (Increment.Z + 1);

		OutVec = FVector(X, Y, Z) * FVector(CubeResolution);
	}

	return bContained;
}

// Called every frame
void ANGGChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

