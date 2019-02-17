// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include "TG_Tile.h"
#include "TG_TileSettings.h"

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
  // Tile to Create
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator")
    TSubclassOf<ATG_Tile> tileToCreate;

  // Tile to Create in X axis
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator", meta = (ClampMin = "1.0"))
    int numberOfTilesX = 3;
  // Tile to Create in Y axis
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator", meta = (ClampMin = "1.0"))
    int numberOfTilesY = 3;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator")
    int Seed = 12345;
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
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Tile|Lists")
    TArray<ATG_Tile*> TilesList;

  /*
    PATHS FOR EDITOR
  */
  // List of the Tiles Created
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Paths")
    FName TilePath = TEXT("Tiles");

  /*
    PRE BACK OPTION
  */
  // Bool to generate the world in Editor Mode
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Editor")
    bool PreBakeWorld = false;
  // Bool to destroy the world in Editor Mode
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Editor")
    bool DestroyWorld = false;

protected:
  TG_PerlinNoise perlinNoise;

private:
	
	
};
