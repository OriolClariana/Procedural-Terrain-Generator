// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include <Components/InstancedStaticMeshComponent.h>

#include "TG_AssetSettings.generated.h"

USTRUCT(BlueprintType)
struct FInstancedArray {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, Category = "AssetList")
    TArray<UInstancedStaticMeshComponent*> list;

  int GetAvailable() {
    int avalaible = -1;
    for (int i = 0; (i < list.Num()) || (avalaible == -1); ++i) {
      if (list[i]->bHiddenInGame == true) {
        avalaible = i;
      }
    }
    return avalaible;
  };
};

USTRUCT(BlueprintType)
struct FAssetSettings {
  GENERATED_USTRUCT_BODY()

  UPROPERTY(EditAnywhere, Category = "BiomeSettings", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float probability = 0.25f;

  UPROPERTY(EditAnywhere, Category = "AssetSettings")
    bool collision = false;
  UPROPERTY(EditAnywhere, Category = "AssetSettings")
    bool randomRotation = false;
  UPROPERTY(EditAnywhere, Category = "AssetSettings")
    bool randomScale = false;
  UPROPERTY(EditAnywhere, Category = "AssetSettings")
    FVector maxRandomScale = FVector(10.f, 10.f, 10.f);

  UPROPERTY(EditAnywhere, Category = "AssetSettings")
    UStaticMesh* mesh = nullptr;

  //UPROPERTY(VisibleAnywhere, Category = "AssetSettings")
    //TMap<FString, FInstancedArray> assetsInstanced;

};