// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
		virtual void TEST(float& rFloat) PURE_VIRTUAL(UNGGAlgorithmBase::TEST, );
	
};
