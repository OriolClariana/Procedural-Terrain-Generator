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
	
  if (useRuntime)
  {
    CreateTerrain();
  }
}

void ATG_TerrainGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

#if WITH_EDITOR
void ATG_TerrainGenerator::OnConstruction(const FTransform & Transform)
{
  TArray<AActor*> actors;

  UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATG_TerrainGenerator::StaticClass(), actors);

  if (actors.Num() > 1)
  {
    Destroy();
    DestroyConstructedComponents();
    GEngine->ForceGarbageCollection(true);
    FMessageDialog::Debugf(FText::FromString("Already exist a TerrainGenerator Object"));
  }
}
#endif

#if WITH_EDITOR
void ATG_TerrainGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
  Super::PostEditChangeProperty(e);

  FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

  // Runtime Bool
  if (PropertyName == GET_MEMBER_NAME_CHECKED(ATG_TerrainGenerator, useRuntime)) {
    if (useRuntime) {
      usePreBake = false;
    }
  }
  // PreBake Bool
  if (PropertyName == GET_MEMBER_NAME_CHECKED(ATG_TerrainGenerator, usePreBake)) {
    if (usePreBake) {
      useRuntime = false;
    }
  }

  // Create World
  if (PropertyName == GET_MEMBER_NAME_CHECKED(ATG_TerrainGenerator, CreateWorld)) {
    /* If the Bool is pressed we call Create Terrain */
    if (CreateWorld) {
      CreateWorld = !CreateWorld;

      if (false == generated)
      {
        CreateTerrain();
      }
      else {
        UpdateTerrain();
      }
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

  //Loop
  for (int x = -(numberOfTilesX / 2); x <= (numberOfTilesX / 2); ++x) {
    for (int y = -(numberOfTilesY / 2); y <= (numberOfTilesY / 2); ++y) {

      // Create new Tile
      ATG_Tile* tile = GetWorld()->SpawnActor<ATG_Tile>(FVector::ZeroVector, FRotator::ZeroRotator);

#if WITH_EDITOR
      if (TilePath != TEXT("")) {
        tile->SetFolderPath(TilePath);
      }
#endif

      // ID
      int newTileId = TilesList.Num();

      // Initialize the Tile
      tile->Init(newTileId, x, y, tileSettings, this);

      // Save the Tile
      TilesList.Add(tile);
    }
  }

  generated = true;
}

void ATG_TerrainGenerator::UpdateTerrain() {
  UE_LOG(LogTerrainGenerator, Log, TEXT("Update the Terrain"));

  // Init the tileArray Size
  tileSettings.Init();

  // Initialize the Algorithm selected
  InitAlgorithm();

  for (ATG_Tile* tile : TilesList) {
    // Initialize the Tile
    tile->Init(tile->TileID, tile->TileX, tile->TileY, tileSettings, this);
  }
}

void ATG_TerrainGenerator::DestroyTerrain()
{
  UE_LOG(LogTerrainGenerator, Log, TEXT("Destroy all the Terrain Tiles"));
  // If Tiles exist
  if (TilesList.Num() > 0) {
    for (ATG_Tile* tile : TilesList)
    {
      tile->RuntimeMesh->ClearAllMeshSections();
      tile->Destroy();
    }
    TilesList.Empty();
  }

  generated = false;
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
  
  //Apply the Amplitude to the results
  value *= Amplitude;

  return value;
}