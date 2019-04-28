// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include "TG_TileSettings.h"
#include "TG_MeshSettings.h"
#include "TG_AssetSettings.h"
#include "RuntimeMeshComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TG_Tile.generated.h"


/* Forward Declaration */
class ATG_TerrainGenerator;

UCLASS()
class TERRAINGENERATOR_API ATG_Tile : public AActor
{
	GENERATED_BODY()
	
public:
  ATG_Tile();

  UFUNCTION()
    void Init(int tileID, int coordX, int coordY, FTileSettings tSettings, ATG_TerrainGenerator* manager);
  UFUNCTION()
    void Update(int coordX, int coordY);
  UFUNCTION()
    bool DestroyTile();

  /* GETTER */
  UFUNCTION()
    FString GetTileNameCoords(int x, int y);
  UFUNCTION()
    FVector2D GetVerticePosition(float x, float y);
  UFUNCTION()
    FVector2D CalculateUV(float x, float y);
  UFUNCTION()
    float ScaleZWithHeightRange(double value);
  UFUNCTION()
    int GetValueIndexForCoordinates(int x, int y);
  UFUNCTION()
    FVector2D GetCoordsWithIndex(int index);
  UFUNCTION()
    FVector2D CalculateWorldPosition(float x, float y);
  UFUNCTION()
    double GetNoiseValueForGridCoordinates(double x, double y);


  /* SETTER */
  UFUNCTION()
    void SetTileWorldPosition(int coordX, int coordY, FTileSettings tSettings);
  UFUNCTION()
    void SetVisibile(bool option);
  UFUNCTION()
    void SetVisibileAsset(bool option);

 
  /*
   CONFIGURABLE VARIABLES
  */
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
    bool Generated = false;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
    bool Visible = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
    int TileID = -1;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
    int TileSeed = -1;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
    int TileX = 0;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
    int TileY = 0;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
    float maxDistanceForAssets = 0.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
    URuntimeMeshComponent* RuntimeMesh;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
    UStaticMeshComponent* waterComponent;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile")
    TArray<UInstancedStaticMeshComponent*> InstancedList;

protected:
  /* Initialize all the Mesh values to DEFAULT value */
  UFUNCTION()
    void InitMeshToCreate();

  /* Generate the Vertices on the Mesh with Algorithm result */
  UFUNCTION()
    void GenerateVertices();
  UFUNCTION()
    void GenerateTriangles();
  UFUNCTION()
    void GenerateNormalTangents(bool SmoothNormals);

  /* Generate the Mesh with the values modified in other functions */
  UFUNCTION()
    void GenerateMesh(UMaterialInterface* material);

  /* Update the Mesh with the values modified in other functions */
  UFUNCTION()
    void UpdateMesh(UMaterialInterface* material);

  /* Setup the Water settings*/
  UFUNCTION()
    void SetupWater(FTileSettings tSettings);

  /* Setup the Biomes settings*/
  UFUNCTION()
    void SetupBiomes(FTileSettings tSettings);

  /* Setup the Biomes settings*/
  UFUNCTION()
    void SetupAssets(FTileSettings tSettings);

  /* Set the Terrain Position on the middle the Tile */
  UFUNCTION()
	  void InitTerrainPosition();

  UFUNCTION()
    void setTileName(FName text);

  // Perlin Value Array
  TMap<FVector2D, double> ZPositions;

private:
  UPROPERTY()
    ATG_TerrainGenerator* TerrainGenerator;

  UPROPERTY()
    FTileSettings tileSettings;

  UPROPERTY()
    FMeshSettings MeshToCreate;
};