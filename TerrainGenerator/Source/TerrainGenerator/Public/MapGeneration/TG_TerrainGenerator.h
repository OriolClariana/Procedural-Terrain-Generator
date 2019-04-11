// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include "TG_Tile.h"
#include "TG_TileSettings.h"
#include "TG_BiomeSettings.h"

/* Algorithms */
#include "TG_PerlinNoise.h"

#include "GameFramework/Character.h"

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
    void CreateTile(int x, int y);
  
  UFUNCTION()
    void UpdateTerrain();

  UFUNCTION()
    void DestroyTerrain();

  UFUNCTION()
    FVector2D getPlayerTileCoord();

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
	bool randomSeed = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator")
    int Seed = 3140;
  // Tile to Create in X & Y axis
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator", meta = (ClampMin = "1.0"))
    int numberOfTiles = 10;
  UPROPERTY(EditAnywhere, Category = "TerrainGenerator", meta = (ClampMin = "1"))
    double Amplitude = 5;
  UPROPERTY(EditAnywhere, Category = "TerrainGenerator", meta = (ClampMin = "0.0", ClampMax = "0.01"))
    double Frequency = 0.000002;
  UPROPERTY(EditAnywhere, Category = "TerrainGenerator")
    int Octaves = 8;

  // Settings of the Tile
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Tile")
    FTileSettings tileSettings;

  // List of the Tiles Created
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Tile|Lists")
    TMap<FVector2D, ATG_Tile*> TileMap;


  /* Default Material in case not exist Biome */
  UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "TerrainGenerator|Biomes", Meta = (ToolTip = "Material overrides."))
    UMaterialInterface* defaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);

  /* Use vertex Color == true | Use Material == False */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Biomes")
    bool useVertexColor = false;

  /* Spawn the assets in the Biome List */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Biomes")
    bool spawnAssets = false;

  /* Settings for the Biomes */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Biomes")
    TArray<FBiomeSettings> biomeList;

  /* Activate Water */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Biomes|Water")
    bool useWater = true;

  /* Material for the Water */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Biomes|Water")
    UStaticMesh* water;

  UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "TerrainGenerator|Biomes|Water", Meta = (ToolTip = "Material overrides."))
    UMaterialInterface* waterMaterial = UMaterial::GetDefaultMaterial(MD_Surface);

  /* Water Level position with respect to HeightRange */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Biomes|Water", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float waterHeight = 0.4f;

  /*
    RUNTIME OPTION
  */
  /* Mark this bool if you want to use the Runtime Option */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Runtime")
    bool useRuntime = true;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Runtime", Meta = (EditCondition = "useRuntime"))
    bool infiniteTerrain = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Runtime|Infinite")
    bool optimalViewDistance = true;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Runtime|Infinite")
    float maxViewDistance = 25000.f;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Runtime|Infinite")
    int tVisibleInViewDst = 0;

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
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|EditorTexts")
    FName TilePath = TEXT("Tiles");

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|EditorTexts")
    FName TileName = TEXT("Tile");

  // The Player
  UPROPERTY()
    ACharacter* player = nullptr;

  UPROPERTY()
    double maxHeight = 0.0;

protected:
  UPROPERTY()
    bool generated = false;

  UPROPERTY()
    TArray<ATG_Tile*> TileList;

  TG_PerlinNoise perlinNoiseTerrain;
  TG_PerlinNoise perlinNoiseBiomes;
  TG_PerlinNoise perlinNoiseAssets;

private:
  void default_biomes();
	
};