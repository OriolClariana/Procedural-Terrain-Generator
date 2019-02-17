// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#include "TG_TerrainGenerator.h"

DEFINE_LOG_CATEGORY_STATIC(LogTerrainGenerator, Log, All);

ATG_TerrainGenerator::ATG_TerrainGenerator()
{
 	PrimaryActorTick.bCanEverTick = true;

}

void ATG_TerrainGenerator::BeginPlay()
{
	Super::BeginPlay();
	
  CreateTerrain();
}

void ATG_TerrainGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/* If MyBool belongs to the ASomeActor */
#if WITH_EDITOR
void ATG_TerrainGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
  Super::PostEditChangeProperty(e);

  FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

  // Pre Bake World
  if (PropertyName == GET_MEMBER_NAME_CHECKED(ATG_TerrainGenerator, PreBakeWorld)) {
    /* If the Bool is pressed we call Create Terrain */
    if (PreBakeWorld) {
      PreBakeWorld = !PreBakeWorld;
      DestroyTerrain();
      CreateTerrain();
    }
  }

  // Destroy the Tiles
  if (PropertyName == GET_MEMBER_NAME_CHECKED(ATG_TerrainGenerator, DestroyWorld)) {
    /* If the Bool is pressed we call Create Terrain */
    if (DestroyWorld) {
      DestroyWorld = !DestroyWorld;
      DestroyTerrain();
    }
  }
}
#endif

void ATG_TerrainGenerator::CreateTerrain()
{
  UE_LOG(LogTerrainGenerator, Log, TEXT("Create the Terrain"));
  
  // Init the tileArray Size
  tileSettings.Init();

  // Initialize the Algorithm selected
  InitAlgorithm();

  // If Tile exists
  if (tileToCreate) {
    //Loop
    for (int x = -numberOfTilesX / 2; x <= (numberOfTilesX / 2); ++x) {
      for (int y = -(numberOfTilesY/2); y <= (numberOfTilesY/2); ++y) {
        // Create new Tile
        ATG_Tile* newTile = GetWorld()->SpawnActor<ATG_Tile>(tileToCreate, FVector::ZeroVector, FRotator::ZeroRotator);

#if WITH_EDITOR
        if (TilePath != TEXT("")) {
          newTile->SetFolderPath(TilePath);
        }
#endif

        // ID
        int newTileId = TilesList.Num();

        // Initialize the Tile
        newTile->Init(newTileId, x, y, tileSettings, this);

        // Save the Tile
        TilesList.Add(newTile);
      }
    }
  }
}

void ATG_TerrainGenerator::UpdateTerrain() {
  UE_LOG(LogTerrainGenerator, Log, TEXT("Update the Terrain Tiles"));

}

void ATG_TerrainGenerator::DestroyTerrain()
{
  UE_LOG(LogTerrainGenerator, Log, TEXT("Destroy all the Terrain Tiles"));
  // If Tiles exist
  if (TilesList.Num() > 0) {
    for (ATG_Tile* tile : TilesList)
    {
      tile->Destroy();
    }
    TilesList.Empty();
  }
}


void ATG_TerrainGenerator::InitAlgorithm() {

  if (Seed == 0) {
    Seed = FMath::Rand();
  }
  perlinNoise.setNoiseSeed(Seed);

}


double ATG_TerrainGenerator::GetAlgorithmValue(double x, double y) {
  double value = 0.0;

  value += perlinNoise.octaveNoise0_1(Frequency * x, Frequency * y, Octaves);
  
  //Apply the Amplitud to the results
  value *= Amplitude;

  return value;
}