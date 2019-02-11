// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include "TG_TileSettings.h"
#include "TG_MeshSettings.h"
#include "RuntimeMeshComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TG_Tile.generated.h"

UCLASS()
class TERRAINGENERATOR_API ATG_Tile : public AActor
{
	GENERATED_BODY()
	
public:
  ATG_Tile();
	virtual void BeginPlay() override;

  UFUNCTION()
    void Init(int tileID, int coordX, int coordY, FTileSettings tSettings);
  UFUNCTION()
    void InitMeshToCreate();
  UFUNCTION()
    void GenerateVertices();
  UFUNCTION()
    void GenerateTriangles();
  UFUNCTION()
    void GenerateMesh();

  /* GETTER */
  UFUNCTION()
    FString GetTileNameCoords(int x, int y);
  UFUNCTION()
    FVector2D GetTileWorldPosition(float x, float y);
  UFUNCTION()
    FVector2D CalculateUV(float x, float y);
  UFUNCTION()
    float GetZPositionFromAlgorithmResult(float AlgorithmResult);
  UFUNCTION()
    int GetValueIndexForCoordinates(int x, int y);

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
    int TileX = 0;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile")
    int TileY = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
    URuntimeMeshComponent* RuntimeMesh;

protected:

private:
  UPROPERTY()
    FTileSettings tileSettings;

  UPROPERTY()
    FMeshSettings MeshToCreate;
	
};
