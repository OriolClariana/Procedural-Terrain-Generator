// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include "TG_TileSettings.h"
#include "TG_MeshSettings.h"
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
	virtual void BeginPlay() override;

  UFUNCTION()
    void Init(int tileID, int coordX, int coordY, FTileSettings tSettings, ATG_TerrainGenerator* manager);

  UFUNCTION()
    void Update();

  /* GETTER */
  UFUNCTION()
    FString GetTileNameCoords(int x, int y);
  UFUNCTION()
    FVector2D GetVerticePosition(float x, float y);
  UFUNCTION()
    FVector2D CalculateUV(float x, float y);
  UFUNCTION()
    float ScaleZWithHeightRange(double AlgorithmZ);
  UFUNCTION()
    int GetValueIndexForCoordinates(int x, int y);
  UFUNCTION()
    double GetNoiseValueForGridCoordinates(double x, double y);

  /* SETTER */
  UFUNCTION()
    void SetAlgorithmValueAt(int x, int y, FVector value);
  UFUNCTION()
    void SetTileWorldPosition(int coordX, int coordY);

  /*
   CONFIGURABLE VARIABLES
  */
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
    bool Generated = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
    int TileID = -1;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
    int TileSeed = -1;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
    int TileX = 0;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
    int TileY = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
    URuntimeMeshComponent* RuntimeMesh;

protected:
  /* Initialize all the Mesh values to DEFAULT value */
  UFUNCTION()
    void InitMeshToCreate();

  /* Generate the Vertices on the Mesh with Algorithm result */
  UFUNCTION()
    void GenerateVertices();
  UFUNCTION()
    void GenerateTriangles();

  /* Generate the Mesh with the values modified in other functions */
  UFUNCTION()
    void GenerateMesh();

private:
  UPROPERTY()
    ATG_TerrainGenerator* TerrainGenerator;

  UPROPERTY()
    FTileSettings tileSettings;

  UPROPERTY()
    FMeshSettings MeshToCreate;
	
};
