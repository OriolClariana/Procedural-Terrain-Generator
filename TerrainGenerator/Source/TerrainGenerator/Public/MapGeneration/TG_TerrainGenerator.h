// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include "TG_Tile.h"
#include "TG_TileSettings.h"
#include "TG_BiomeSettings.h"

/* Algorithms */
#include "TG_PerlinNoise.h"

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "TG_TerrainGenerator.generated.h"

UCLASS(Blueprintable)
class TERRAINGENERATOR_API ATG_TerrainGenerator : public AInfo
{
	GENERATED_BODY()
	
public:	
	ATG_TerrainGenerator();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
  void OnConstruction(const FTransform& Transform) override;
#endif

  // PreBake the World
  void PostEditChangeProperty(struct FPropertyChangedEvent& e);

  UFUNCTION()
    void CreateTerrain();

  UFUNCTION()
    void UpdateTerrain();

  UFUNCTION()
    void DestroyTerrain();

  /* Algorithms Functions */
  UFUNCTION()
    void InitAlgorithm();

  UFUNCTION()
    double GetAlgorithmValue(double x, double y);

  /*
   CONFIGURABLE VARIABLES
  */
  /* Seed of the Map */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator")
    int Seed = 12345;
  // Tile to Create in X axis
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator", meta = (ClampMin = "1.0"))
    int numberOfTilesX = 3;
  // Tile to Create in Y axis
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator", meta = (ClampMin = "1.0"))
    int numberOfTilesY = 3;

  UPROPERTY(EditAnywhere, Category = "TerrainGenerator", meta = (ClampMin = "1"))
    double Amplitude = 1;
  UPROPERTY(EditAnywhere, Category = "TerrainGenerator", meta = (ClampMin = "0.0", ClampMax = "0.01"))
    double Frequency = 0.0001;
  UPROPERTY(EditAnywhere, Category = "TerrainGenerator")
    int Octaves = 8;

  // Settings of the Tile
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Tile")
    FTileSettings tileSettings;

  // List of the Tiles Created
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Tile|Lists")
    TArray<ATG_Tile*> TilesList;


  /* Default Material in case not exist Biome */
  UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "TerrainGenerator|Biomes", Meta = (ToolTip = "Material overrides."))
    UMaterialInterface* defaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);

  /* Activate Water */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Biomes|Water")
    bool useWater = true;

  /* Material for the Water */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Biomes|Water")
    UStaticMesh* water;

  /* Water Level position with respect to HeightRange */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Biomes|Water", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float waterHeight = 0.4f;

  /* Settings for the Biomes */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Biomes")
    TArray<FBiomeSettings> biomeList;

  /*
    RUNTIME OPTION
  */
  /* Mark this bool if you want to use the Runtime Option */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Runtime")
    bool useRuntime = true;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Runtime", Meta = (EditCondition = "useRuntime"))
    bool infiniteTerrain = false;

  /*
    PRE BACK OPTION
  */
  /* Mark this bool if you want to use the PreBake Option */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|PreBake")
    bool usePreBake = false;
  // Bool to generate the world in Editor Mode
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|PreBake", Meta = (EditCondition = "usePreBake"))
    bool CreateWorld = false;
  // Bool to destroy the world in Editor Mode
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|PreBake", Meta = (EditCondition = "usePreBake"))
    bool DestroyWorld = false;

  /*
    PATHS FOR EDITOR
  */
  // List of the Tiles Created
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Paths")
    FName TilePath = TEXT("Tiles");


protected:
  UPROPERTY()
  bool generated = false;

  TG_PerlinNoise perlinNoise;

private:
	
	
};
