// Fill out your copyright notice in the Description page of Project Settings.


#include "NMCActor.h"

//procedural mesh includes
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"

//debug stuff
#include "DrawDebugHelpers.h"

// Sets default values
ANMCActor::ANMCActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = ProceduralMeshComponent;

	StartMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StartMeshComponent->SetupAttachment(RootComponent);
}

void ANMCActor::GenerateRandomizedMesh()
{
	TerrainMap.Empty();
	Sections.Empty();

	int32 SecX = FMath::CeilToInt(SectionSize.X / CustomChunkSize.X);
	int32 SecY = FMath::CeilToInt(SectionSize.Y / CustomChunkSize.Y);
	for (int32 sX = 0; sX < SecX; ++sX)
	{
		for (int32 sY = 0; sY < SecY; ++sY)
		{
			FMeshSection SingleSection;
			SingleSection.StartPoint = FVector(sX * CustomChunkSize.X, sY * CustomChunkSize.Y, 0);
			SingleSection.Extent = FVector((sX + 1) * CustomChunkSize.X, (sY + 1) * CustomChunkSize.Y, SectionSize.Z);
			Sections.Add(sY + sX * SecY, SingleSection);
		}
	}

	// The data points for terrain are stored at the corners of our "cubes", so the terrainMap needs to be 1 larger
	// than the width/height of our mesh.
	for (int x = 0; x < SectionSize.X + 1; x += CubeResolution.X) {
		for (int y = 0; y < SectionSize.Y + 1; y += CubeResolution.Y) {
			for (int z = 0; z < SectionSize.Z + 1; z += CubeResolution.Z) {

				// Get a terrain height using regular old Perlin noise.
				float thisHeight = SectionSize.Z *
					((FMath::PerlinNoise3D(FVector(
						(((float)x + GetActorLocation().X + 0.001f) / CubeResolution.X) / FeatureSize,
						(((float)y + GetActorLocation().Y + 0.001f) / CubeResolution.Y) / FeatureSize,
						GetActorLocation().Z)
					) + 1.0f) / 2.0f);

				// Set the value of this point in the terrainMap.
				TerrainMap.Add(FVector(x, y, z), z - thisHeight);

			}
		}
	}

	ItlUpdateSections();
}

void ANMCActor::EditTerrain(FVector LocationInWS, FVector HitNormal, bool bAddTerrain, float BrushSize)
{
	int BuildModifier = bAddTerrain ? -1 : 1;

	FVector Location;
	ItlGetContainingCubeVector(LocationInWS, Location);

	int32 XOffset = FMath::CeilToInt(BrushSize / CubeResolution.X) * CubeResolution.X;
	int32 YOffset = FMath::CeilToInt(BrushSize / CubeResolution.Y) * CubeResolution.Y;
	int32 ZOffset = FMath::CeilToInt(BrushSize / CubeResolution.Z) * CubeResolution.Z;
	
	for (int32 x = Location.X - XOffset; x < Location.X + XOffset; x += CubeResolution.X)
	{
		for (int32 y = Location.Y - YOffset; y < Location.Y + YOffset; y += CubeResolution.Y)
		{
			for (int32 z = Location.Z - ZOffset; z < Location.Z + ZOffset; z += CubeResolution.Z)
			{
				FVector OffsetPoint(x,y,z);
				float Distance = FVector::Distance(OffsetPoint, Location);
				if (Distance < BrushSize)
				{
					float ModificationAmount = DensityChange /*/ Distance*/ * BuildModifier;
					ItlUpdateTerrain(OffsetPoint, ModificationAmount);
				}
				
			}
		}
	}

	ItlUpdateSections();
}

// Called when the game starts or when spawned
void ANMCActor::BeginPlay()
{
	Super::BeginPlay();


	GenerateRandomizedMesh();
}

void ANMCActor::MarchCube(FVector Position, int32 Key)
{
	// Sample terrain values at each corner of the cube.
	TArray<float> CubeValues;
	CubeValues.SetNum(8);
	for (int i = 0; i < 8; i++)
		CubeValues[i] = TerrainMap[Position + CornerTable[i] * CubeResolution];

	int32 TableIndex = ItlGetCubeConfiguration(CubeValues);

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
			FVector Vertex1 = Position + CornerTable[EdgeTable[TriIndex][0]] * CubeResolution;
			FVector Vertex2 = Position + CornerTable[EdgeTable[TriIndex][1]] * CubeResolution;

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
			Sections[Key].Vertices.Add(VertexPosition);
			Sections[Key].Triangles.Add(Sections[Key].Vertices.Num() - 1);
			Sections[Key].Normals.Add(FVector(0, 0, 1));
			Sections[Key].UV.Add(FVector2D(((int32)VertexPosition.X % (int32)(SectionSize.X + 1)) / (SectionSize.X + 1), ((int32)VertexPosition.Y % (int32)(SectionSize.Y + 1)) / (SectionSize.Y + 1)));
			FProcMeshTangent tangent(FVector(1, 0, 0), false);
			Sections[Key].Tangents.Add(tangent);
			Sections[Key].VertexColors.Add(FColor(1, 1, 1));
			EdgeIdx++;

		}
	}
}

void ANMCActor::ItlUpdateSections()
{

	TArray<int32> Keys;
	Sections.GetKeys(Keys);
	for (auto & Key : Keys)
	{
		FMeshSection SingleSection = Sections[Key];
		if (SingleSection.bChanged)
		{
			// Clear Mesh Data
			ItlClearMeshSection(Key);
			// Run Algorithm to update terrain map
			for (int x = SingleSection.StartPoint.X; x < SingleSection.Extent.X; x += CubeResolution.X)
			{
				for (int y = SingleSection.StartPoint.Y; y < SingleSection.Extent.Y; y += CubeResolution.Y)
				{
					for (int z = SingleSection.StartPoint.Z; z < SingleSection.Extent.Z; z += CubeResolution.Z)
					{
						MarchCube(FVector(x, y, z), Key);
					}
				}
			}
			// Recreate mesh section
			ItlUpdateMeshSection(Key);
			// Update changes
			SingleSection.bChanged = false;
		}
	}
}

void ANMCActor::ItlUpdateMeshSection(int32 SectionID)
{
	if (Sections.Contains(SectionID))
	{
		FMeshSection S = Sections[SectionID];
		
		//TODO: Get rid of the empty values? 
		TArray<FVector> EmptyVector;
		TArray<FColor> EmptyColor;
		TArray<FVector2D> Empty2D;
		TArray<FProcMeshTangent> EmptyTangents;
		ProceduralMeshComponent->CreateMeshSection(SectionID,
			S.Vertices,
			S.Triangles,
			EmptyVector,
			S.UV,
			EmptyColor,
			EmptyTangents,
			true);

	}
}

int ANMCActor::ItlGetCubeConfiguration(TArray<float> CubeValue)
{
	int32 TableIndex = 0;

	// If it is, use bit-magic to the set the corresponding bit to 1. So if only the 3rd point in the cube was below
	// the surface, the bit would look like 00100000, which represents the integer value 32.
	for (int i = 0; i < 8; i++)
		if (CubeValue[i] > TerrainSurface)
			TableIndex |= 1 << i;

	return TableIndex;
}

void ANMCActor::ItlClearMeshSection(int32 SectionID)
{
	Sections[SectionID].Vertices.Empty();
	Sections[SectionID].Triangles.Empty();
	Sections[SectionID].Normals.Empty();
	Sections[SectionID].UV.Empty();
	Sections[SectionID].Tangents.Empty();
	Sections[SectionID].VertexColors.Empty();
}

void ANMCActor::ItlUpdateTerrain(FVector Location, float Adjustment)
{
	FVector CubedLocation = Location;
	bool bContained = TerrainMap.Contains(CubedLocation);
	if (!bContained)
	{
		bContained = ItlGetContainingCubeVector(Location, CubedLocation);
	}
	if (bContained)
	{
		// update value
		TerrainMap.Emplace(CubedLocation, TerrainMap[CubedLocation] + Adjustment);
		// set section update necessary

		FVector Start, End;
		for (auto & singleSection : Sections)
		{
			Start = singleSection.Value.StartPoint;
			End = singleSection.Value.Extent;
			if (Start.X <= CubedLocation.X && CubedLocation.X <= End.X &&
				Start.Y <= CubedLocation.Y && CubedLocation.Y <= End.Y &&
				Start.Z <= CubedLocation.Z && CubedLocation.Z <= End.Z )
			{
				singleSection.Value.bChanged = true;
			}
		}
	}
}

bool ANMCActor::ItlGetContainingCubeVector(FVector InVec, FVector & OutVec)
{
	// Ensure vector is empty and contains no bogus
	OutVec = FVector::ZeroVector;

	// Transform to local space
	FVector LocalVec = InVec - GetActorLocation();

	// The TerrainMap contains only local space coordinates between 0 and SectionSize.AXIS
	bool bContained = LocalVec.X >= 0.f && LocalVec.X <= SectionSize.X &&
		LocalVec.Y >= 0.f && LocalVec.Y <= SectionSize.Y &&
		LocalVec.Z >= 0.f && LocalVec.Z <= SectionSize.Z;

	if (bContained)
	{
		//Only now can we determine the closest cube in the terrain map
		float AdjustedX;
		float AdjustedY;
		float AdjustedZ;

		AdjustedX = FMath::RoundToInt(LocalVec.X / CubeResolution.X) * CubeResolution.X;
		AdjustedY = FMath::RoundToInt(LocalVec.Y / CubeResolution.Y) * CubeResolution.Y;
		AdjustedZ = FMath::RoundToInt(LocalVec.Z / CubeResolution.Z) * CubeResolution.Z;

		OutVec = FVector(AdjustedX, AdjustedY, AdjustedZ);
	}

	return bContained;
}

FVector ANMCActor::ItlDetermineHitDirection(FVector HitNormal)
{
	HitNormal.Normalize();
	float DegreeXAxis = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitNormal, FVector(1, 0, 0))));
	float DegreeYAxis = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitNormal, FVector(0, 1, 0))));
	float DegreeZAxis = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitNormal, FVector(0, 0, 1))));
	//GEngine->AddOnScreenDebugMessage(-10, 1.f, FColor::Yellow, FString::Printf(TEXT("X: %f Y: %f Z: %f"), DegreeXAxis, DegreeYAxis, DegreeZAxis));

	float VecX = 0.0f;
	float VecY = 0.0f;
	float VecZ = 0.0f;

	if (DegreeXAxis < 60.0f) //points up
		VecX = 1.0f;
	else if (DegreeXAxis > 120.0f) //points down
		VecX = -1.0f;
	if (DegreeZAxis < 60.0f) //points towards X
		VecZ = 1.0f;
	else if (DegreeZAxis > 120.0f) //points away from X
		VecZ = -1.0f;
	if (DegreeYAxis < 60.0f) //points towards Y
		VecY = 1.0f;
	else if (DegreeYAxis > 120.0f) //points away from Y
		VecY = -1.0f;

	return FVector(VecX, VecY, VecZ);
}

void ANMCActor::OnConstruction(const FTransform & Transform)
{

	if (bUseCustomChunkSize)
	{
		// We have to ensure that the chunk is smaller than the entire section...
		CustomChunkSize.X = CustomChunkSize.X > SectionSize.X ? SectionSize.X : CustomChunkSize.X;
		CustomChunkSize.Y = CustomChunkSize.Y > SectionSize.Y ? SectionSize.Y : CustomChunkSize.Y;
	}
	else
	{
		CustomChunkSize.X = SectionSize.X;
		CustomChunkSize.Y = SectionSize.Y;
	}


	////First everything must be setup
	//bool bConstruct = IsValid(StartMesh) && IsValid(StartMeshComponent) && IsValid(ProceduralMeshComponent);
	////And the mesh must be actually changed (otherwise this would be expensive...)
	//bConstruct = bConstruct && StartMesh != StartMeshComponent->GetStaticMesh();
	//if (bConstruct)
	//{
	//	ProceduralMeshComponent->ClearAllMeshSections();
	//	StartMeshComponent->SetStaticMesh(StartMesh);
	//	UKismetProceduralMeshLibrary::CopyProceduralMeshFromStaticMeshComponent(StartMeshComponent, 0, ProceduralMeshComponent, true);
	//}
}

// Called every frame
void ANMCActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

