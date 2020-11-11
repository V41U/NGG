// Fill out your copyright notice in the Description page of Project Settings.


#include "MyNGGAlgorithmMarchingCube.h"


void UMyNGGAlgorithmMarchingCube::Setup(FIntVector Extent, FIntVector Resolution)
{
	m_Extent = FVector(Extent);
	m_Resolution = FVector(Resolution);
	m_Increment = m_Extent / m_Resolution;
	GEngine->AddOnScreenDebugMessage(1, 1, FColor::Green, TEXT("THIS IS THE MARCHING CUBE"));
}


void UMyNGGAlgorithmMarchingCube::GenerateGeometry(TArray<float>& Input,
	TArray<FVector>& Vertices,
	TArray<int32>& Triangles,
	TArray<FVector>& Normals,
	TArray<FVector2D>& UVs,
	TArray<struct FProcMeshTangent>& Tangents,
	TArray<FColor>& VertexColors)
{

	//there's some weird off by one error going on...
	for (int x = 0; x < m_Increment.X - 1; ++x)
	{
		for (int y = 0; y < m_Increment.Y - 1; ++y)
		{
			for (int z = 0; z < m_Increment.Z - 1; ++z)
			{
				// March the cube for each position
				MarchCube(FVector(x, y, z), 
					Input,
					Vertices,
					Triangles,
					Normals,
					UVs,
					Tangents,
					VertexColors);
			}
		}
	}

}

void UMyNGGAlgorithmMarchingCube::MarchCube(FVector Position, 
	TArray<float>& Input,
	TArray<FVector>& Vertices,
	TArray<int32>& Triangles,
	TArray<FVector>& Normals,
	TArray<FVector2D>& UVs,
	TArray<struct FProcMeshTangent>& Tangents,
	TArray<FColor>& VertexColors)
{
	// Sample terrain values at each corner of the cube.
	TArray<float> CubeValues;
	CubeValues.SetNum(8);

	int iIdx;
	int iIdxOffset;
	for (int i = 0; i < 8; i++)
	{
		iIdx = Position.Z + Position.Y * m_Increment.Z + Position.X * m_Increment.Z * m_Increment.Y;
		iIdxOffset = CornerTable[i].Z + CornerTable[i].Y * m_Increment.Z + CornerTable[i].X * m_Increment.Y * m_Increment.Z;
		CubeValues[i] = Input[iIdx + iIdxOffset];
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
			FVector Vertex1 = Position * m_Resolution + CornerTable[EdgeTable[TriIndex][0]] * m_Resolution;
			FVector Vertex2 = Position * m_Resolution + CornerTable[EdgeTable[TriIndex][1]] * m_Resolution;

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
			UVs.Add(FVector2D(((int32)VertexPosition.X % (int32)(m_Extent.X + 1)) / (m_Extent.X + 1), ((int32)VertexPosition.Y % (int32)(m_Extent.Y + 1)) / (m_Extent.Y + 1)));
			FProcMeshTangent tangent(FVector(1, 0, 0), false);
			Tangents.Add(tangent);
			VertexColors.Add(FColor(1, 1, 1));
			EdgeIdx++;

		}
	}
}
