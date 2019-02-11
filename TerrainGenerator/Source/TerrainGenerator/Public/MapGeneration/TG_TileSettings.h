// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include "TG_TileSettings.generated.h"

USTRUCT(BlueprintType)
struct FTileSettings {
  GENERATED_BODY()
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSettings")
    float TileSize = 1000.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSettings")
    float AlgorithmResolution = 10.0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSettings")
    float HeightRange = 100.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSettings")
    float TextureScale = 6.0;
  UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Rendering, Meta = (ToolTip = "Material overrides."))
    class UMaterialInterface* Material = UMaterial::GetDefaultMaterial(MD_Surface);

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TileSettings")
    int ArrayLineSize = 0;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TileSettings")
    int ArraySize = 0;
};