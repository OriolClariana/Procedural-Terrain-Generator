// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#include "TG_TerrainGenerator.h"

#include "Async/Async.h"

DEFINE_LOG_CATEGORY_STATIC(LogTerrainGenerator, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogTileCreation, Log, All);

ATG_TerrainGenerator::ATG_TerrainGenerator()
{
 	PrimaryActorTick.bCanEverTick = true;

  default_biomes();
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
      //GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, FString::Printf(TEXT("%.1f  |  %.1f"), currentTile.X, currentTile.Y));
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

          // TODO: For now just update the Visibility option
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
  // Calculate the optimal Max View Distance
  if (optimalViewDistance) {
    maxViewDistance = (numberOfTiles * tileSettings.TileSize) / 1.8f;
  }

  // Init the tileArray Size
  tileSettings.Init();

  // Initialize the Algorithm selected
  InitAlgorithm();

  //Loop
  for (int x = -(numberOfTiles / 2); x <= (numberOfTiles / 2); ++x) {
    for (int y = -(numberOfTiles / 2); y <= (numberOfTiles / 2); ++y) {
      // Create new Tile
      CreateTile(x, y);
    }
  }

  generated = true;
}

void ATG_TerrainGenerator::CreateTile(int x, int y) {
  UE_LOG(LogTerrainGenerator, Log, TEXT("Create the Tile [%d, %d]"), x, y);
  ATG_Tile* tile = GetWorld()->SpawnActor<ATG_Tile>(FVector::ZeroVector, FRotator::ZeroRotator);

#if WITH_EDITOR
  if (TilePath != TEXT("")) {
    tile->SetFolderPath(TilePath);
  }
#endif

  // ID
  int newTileId = TileMap.Num();

  // Initialize the Tile
  auto future = Async<void>(EAsyncExecution::Thread, [&]() { tile->Init(newTileId, x, y, tileSettings, this); }, [&] { /* Callback */ });

  // Save the Tile
  TileMap.Add(FVector2D(x, y), tile);

  // SetVisible False for now
  if (useRuntime && infiniteTerrain) {
	  tile->SetVisibile(false);
  }

}

void ATG_TerrainGenerator::UpdateTerrain() {
  UE_LOG(LogTerrainGenerator, Log, TEXT("Update the Terrain"));

  // Init the tileArray Size
  tileSettings.Init();

  // Initialize the Algorithm selected
  InitAlgorithm();

  for (auto tile : TileMap) {
    // Initialize the Tile
    auto future = Async<void>(EAsyncExecution::Thread, [&]() { tile.Value->Init(tile.Value->TileID, tile.Value->TileX, tile.Value->TileY, tileSettings, this); }, [&] { /* Callback */ });
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

  if (randomSeed) {
    Seed = FMath::Rand();
  }

  // Initialize the Terrain Perlin Noise
  perlinNoiseTerrain.setNoiseSeed(Seed);

  // If has some biome
  if (biomeList.Num() > 0) {
    // Initialize the Biomes Perlin Noise
    perlinNoiseBiomes.setNoiseSeed(Seed + 1);
  }

  // If has assets to distribute
  if (biomeList.Num() > 0 && spawnAssets) {
    // Initialize the Assets Perlin Noise
    perlinNoiseAssets.setNoiseSeed(Seed + 2);
  }
}

double ATG_TerrainGenerator::GetAlgorithmValue(double x, double y) {
  double value = 0.0;

  value += perlinNoiseTerrain.octaveNoise0_1(Frequency * x, Frequency * y, Octaves);
  
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

void ATG_TerrainGenerator::default_biomes() {
  // Beach
  FBiomeSettings beach;
  beach.biomeName = TEXT("Beach");
  beach.minHeight = 0.0f;
  beach.maxHeight = 0.45f;
  FColor sandColor_1 = FColor(255, 235, 175, 1);
  FColor sandColor_2 = FColor(255, 230, 160, 1);
  FColor sandColor_3 = FColor(255, 225, 140, 1);
  beach.vertexColors.Add(sandColor_1);
  beach.vertexColors.Add(sandColor_2);
  beach.vertexColors.Add(sandColor_3);
  biomeList.Add(beach);

  // Plains
  FBiomeSettings plain;
  plain.biomeName = TEXT("Plain");
  plain.minHeight = 0.45f;
  plain.maxHeight = 0.75f;
  FColor grassColor_1 = FColor(110, 200, 110, 1);
  FColor grassColor_2 = FColor(95, 190, 95, 1);
  FColor grassColor_3 = FColor(120, 220, 120, 1);
  plain.vertexColors.Add(grassColor_1);
  plain.vertexColors.Add(grassColor_2);
  plain.vertexColors.Add(grassColor_3);
  biomeList.Add(plain);

  // Mountains
  FBiomeSettings mountain;
  mountain.biomeName = TEXT("Mountain");
  mountain.minHeight = 0.75f;
  mountain.maxHeight = 0.95f;
  FColor rockColor_1 = FColor(180, 180, 180, 1);
  FColor rockColor_2 = FColor(170, 170, 170, 1);
  FColor rockColor_3 = FColor(160, 160, 160, 1);
  mountain.vertexColors.Add(rockColor_1);
  mountain.vertexColors.Add(rockColor_2);
  mountain.vertexColors.Add(rockColor_3);
  biomeList.Add(mountain);

  // Snow
  FBiomeSettings snow;
  snow.biomeName = TEXT("Snow");
  snow.minHeight = 0.95f;
  snow.maxHeight = 1.0f;
  FColor snowColor_1 = FColor(240, 240, 240, 1);
  FColor snowColor_2 = FColor(230, 230, 230, 1);
  snow.vertexColors.Add(snowColor_1);
  snow.vertexColors.Add(snowColor_2);
  biomeList.Add(snow);
}