// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include "TG_Tile.h"
#include "TG_TileSettings.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TG_TerrainGenerator.generated.h"

UCLASS()
class TERRAINGENERATOR_API ATG_TerrainGenerator : public AActor
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
    void DestroyTerrain();

  /*
   CONFIGURABLE VARIABLES
  */
  // Bool to generate the world in Editor Mode
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Editor")
    bool PreBakeWorld = false;
  // Bool to destroy the world in Editor Mode
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator|Editor")
    bool DestroyWorld = false;

  // Tile to Create
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator")
    TSubclassOf<ATG_Tile> tileToCreate = LoadClass<ATG_Tile>(NULL, TEXT("Blueprint'/Game/MapGeneration/TerrainGenerator.TG_TerrainGenerator_C'"), NULL, LOAD_None, NULL);

  // Tile to Create in X axis
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator")
    int numberOfTilesX = 3;
  // Tile to Create in Y axis
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator")
    int numberOfTilesY = 3;

  // Settings of the Tile
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator")
    FTileSettings tileSettings;

  // List of the Tiles Created
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainGenerator")
    TArray<ATG_Tile*> TilesList;

protected:

private:
	
	
};
