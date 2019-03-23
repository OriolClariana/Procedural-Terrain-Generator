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

  // Get the player
  player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	
  if (useRuntime)
  {
    // Test
    tVisibleInViewDst = (int)(maxViewDistance / tileSettings.getTileSize());

    CreateTerrain();
  }
}

void ATG_TerrainGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

  // Endless Terrain
  if (useRuntime && infiniteTerrain)
  {
    FVector2D currentTile = getPlayerTileCoord();
    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, FString::Printf(TEXT("%.1f  |  %.1f"), currentTile.X, currentTile.Y));
    }

    // Set Visible to False the Current Tiles
    for (ATG_Tile* tile : TileList) {
      tile->SetVisibile(false);
    }
    TileList.Empty();

    // Check if it's needed a new Tile
    for (int yOffset = -tVisibleInViewDst; yOffset <= tVisibleInViewDst; yOffset++) {
      for (int xOffset = -tVisibleInViewDst; xOffset <= tVisibleInViewDst; xOffset++) {
        FVector2D viewedTileCoord = FVector2D(currentTile.X + xOffset, currentTile.Y + yOffset);

        // Check if contains this tile
        if (TileMap.Contains(viewedTileCoord)) {

          // TODO: For now just update the Visiblity option
          TileMap[viewedTileCoord]->Update(viewedTileCoord.X, viewedTileCoord.Y);

          // If the Tile is Visible save for next frame
          if (TileMap[viewedTileCoord]->Visible) {
            TileList.Add(TileMap[viewedTileCoord]);
          }

        }
        else {
          // For now Create a new one
          CreateTile(viewedTileCoord.X, viewedTileCoord.Y);
        }
      }
    }

  }
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
      CreateTile(x, y);
    }
  }

  generated = true;
}

void ATG_TerrainGenerator::CreateTile(int x, int y) {
  ATG_Tile* tile = GetWorld()->SpawnActor<ATG_Tile>(FVector::ZeroVector, FRotator::ZeroRotator);

#if WITH_EDITOR
  if (TilePath != TEXT("")) {
    tile->SetFolderPath(TilePath);
  }
#endif

  // ID
  int newTileId = TileMap.Num();

  // Initialize the Tile
  tile->Init(newTileId, x, y, tileSettings, this);

  // Save the Tile
  TileMap.Add(FVector2D(x, y), tile);
}

void ATG_TerrainGenerator::UpdateTerrain() {
  UE_LOG(LogTerrainGenerator, Log, TEXT("Update the Terrain"));

  // Init the tileArray Size
  tileSettings.Init();

  // Initialize the Algorithm selected
  InitAlgorithm();

  for (auto tile : TileMap) {
    // Initialize the Tile
    tile.Value->Init(tile.Value->TileID, tile.Value->TileX, tile.Value->TileY, tileSettings, this);
  }
}

void ATG_TerrainGenerator::DestroyTerrain()
{
  UE_LOG(LogTerrainGenerator, Log, TEXT("Destroy all the Terrain Tiles"));
  // If Tiles exist
  if (TileMap.Num() > 0) {
    for (auto tile : TileMap)
    {
      tile.Value->RuntimeMesh->ClearAllMeshSections();
      tile.Value->Destroy();
    }
    TileMap.Empty();
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

FVector2D ATG_TerrainGenerator::getPlayerTileCoord() {
  FVector pLocation = player->GetActorLocation();
  
  // Calculate the Tile X and Tile Y of the Player
  int pX = pLocation.X / tileSettings.getTileSize();
  int pY = pLocation.Y / tileSettings.getTileSize();

  // Return the Tile Coords
  return FVector2D(pX, pY);
}