// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include "TG_BiomeSettings.generated.h"

USTRUCT(BlueprintType)
struct FBiomeSettings {
  GENERATED_USTRUCT_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeSettings")
    FName biomeName = TEXT("None");

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeSettings", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float minHeight = 0.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeSettings", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float maxHeight = 0.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeSettings")
    TSubclassOf<AActor> assets;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeSettings")
    TArray<FColor> vertexColors;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeSettings")
    TArray<UMaterialInterface*> materialList;
};