// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include "TG_TileSettings.generated.h"

UENUM(BlueprintType)
enum class TerrainSizeIn : uint8 {
  TerrainSizeIn_CM  UMETA(DisplayName = "Centimeters"),
  TerrainSizeIn_M   UMETA(DisplayName = "Meters"),
  TerrainSizeIn_KM  UMETA(DisplayName = "Kilometers"),
};

USTRUCT(BlueprintType)
struct FTileSettings {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSettings")
    bool TileCentred = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSettings", meta = (ClampMin = "1.0"))
    float TileSize = 1000.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSettings")
    TerrainSizeIn TileScaleIn = TerrainSizeIn::TerrainSizeIn_CM;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSettings", meta = (ClampMin = "1.0"))
    float LevelOfDetail = 10.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSettings")
    TerrainSizeIn LODScale = TerrainSizeIn::TerrainSizeIn_CM;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSettings", meta = (ClampMin = "1.0"))
    float HeightRange = 100.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSettings")
    TerrainSizeIn HeightScale = TerrainSizeIn::TerrainSizeIn_CM;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSettings", meta = (ClampMin = "1.0"))
    float TextureScale = 1.f;
  UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Rendering, Meta = (ToolTip = "Material overrides."))
    UMaterialInterface* Material = UMaterial::GetDefaultMaterial(MD_Surface);

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TileSettings")
    int ArrayLineSize = 0;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TileSettings")
    int ArraySize = 0;

  /* FUNCTIONS */
  /* Constructor */
  void Init() {
    ArrayLineSize = (getTileSize() / getLOD()) + 1;
    ArraySize = ArrayLineSize * ArrayLineSize;
  }

  /* Get the tileSize with the correct measure */
  float getTileSize() {
    return TileSize * getTerrainScaleValue(TileScaleIn);
  }

  /* Get the tessellation number with the correct measure */
  float getLOD() {
    return LevelOfDetail * getTerrainScaleValue(LODScale);
  }

  /* Get the tessellation number with the correct measure */
  float getHeightRange() {
    return HeightRange * getTerrainScaleValue(HeightScale);
  }

  /* Calculate the correct measure */
  int getTerrainScaleValue(TerrainSizeIn measure) {
    switch (measure)
    {
    case TerrainSizeIn::TerrainSizeIn_CM:
      return 1;
      break;
    case TerrainSizeIn::TerrainSizeIn_M:
      return 100;
      break;
    case TerrainSizeIn::TerrainSizeIn_KM:
      return 100000;
      break;
    default:
      return 1;
      break;
    }
  }

};