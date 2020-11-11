// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProceduralMeshComponent.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NGGAlgorithmBase.generated.h"

/**
 * 
 */
UCLASS()
class NGG_API UNGGAlgorithmBase : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION()
		virtual void Setup(FIntVector Extent, FIntVector Resolution) PURE_VIRTUAL(UNGGAlgorithmBase::Setup, );

	UFUNCTION()
		virtual void GenerateGeometry(TArray<float>& Input,
									  TArray<FVector>& Vertices, 
									  TArray<int32>& Triangles,
									  TArray<FVector>& Normals,
									  TArray<FVector2D>& UVs, 
									  TArray<struct FProcMeshTangent>& Tangents,
									  TArray<FColor>& VertexColors) PURE_VIRTUAL(UNGGAlgorithmBase::GenerateGeometry, );
	
};
