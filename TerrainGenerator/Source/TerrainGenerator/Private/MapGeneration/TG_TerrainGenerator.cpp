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
  if (useRuntime && infiniteTerrain && player)
  {
    FVector2D currentTile = getPlayerTileCoord();
    if (GEngine) {
      //GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, FString::Printf(TEXT("%.1f  |  %.1f"), currentTile.X, currentTile.Y));
    }

    // Set Visible to False the Current Tiles
    for (ATG_Tile* tile : TileList) {
      tile->SetVisibile(false);
      tile->SetVisibileAsset(false);
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
          /*
          // If is not visible and DestroyTilesOutOfRange is true DESTROY
          else if(true == DestroyTilesOutOfRange){
            // Destroy the Tile
            if (true == TileMap[viewedTileCoord]->DestroyTile()) {
              // Remove the Tile from the TMap
              TileMap.Remove(viewedTileCoord);
            }
          }
          */
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
  
  // Set the Coordinates
   FVector position = FVector( x * tileSettings.getTileSize(), y * tileSettings.getTileSize(), 0.f );

  // Spawn the Tile
  ATG_Tile* tile = GetWorld()->SpawnActor<ATG_Tile>(position, FRotator::ZeroRotator);

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
    tile->SetVisibileAsset(false);
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
      tile.Value->DestroyTile();
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
}

double ATG_TerrainGenerator::GetAlgorithmValue(double x, double y) {
  double value = 0.0;

  double totalFreq = Frequency * tileSettings.getTileSize();
  value += perlinNoiseTerrain.octaveNoise0_1(x / totalFreq, y / totalFreq, 0.0, Octaves);
  
  //Apply the Amplitude to the results
  value *= Amplitude;

  return value;
}

double ATG_TerrainGenerator::GetSpecifiedAlgorithmValue(PerlinType type, double x, double y, double amplitude, double frequency, int octaves) {
  TG_PerlinNoise perlinAlgorithm;

  switch (type) {
  case PerlinType::Perlin_Terrain:
    perlinAlgorithm = perlinNoiseTerrain;
    break;
  case PerlinType::Perlin_Biome:
    perlinAlgorithm = perlinNoiseBiomes;
    break;
  }

  double value = 0.0;
  double totalFreq = frequency * tileSettings.getTileSize();
  
  if (octaves > 1) {
    value += perlinAlgorithm.octaveNoise0_1(x / totalFreq, y / totalFreq, 0.0, octaves);
  }
  else {
    value += perlinAlgorithm.noise0_1(x / totalFreq, y / totalFreq);
  }

  value *= amplitude;

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
  // Water Mesh
  ConstructorHelpers::FObjectFinder<UStaticMesh> water_mesh(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
  if (water_mesh.Succeeded()) {
    water = water_mesh.Object;
  }

  // Underwater
  FBiomeSettings underwater;
  underwater.biomeName = TEXT("Underwater");
  underwater.minHeight = 0.0f;
  underwater.maxHeight = 0.35f;
  //FColor underwaterColor_1 = FColor(28, 155, 170, 1);
  FColor underwaterColor_2 = FColor(38, 97, 174, 1);
  //FColor underwaterColor_3 = FColor(36, 80, 188, 1);
  //underwater.vertexColors.Add(underwaterColor_1);
  underwater.vertexColors.Add(underwaterColor_2);
  //underwater.vertexColors.Add(underwaterColor_3);
  biomeList.Add(underwater);

  // Beach
  FBiomeSettings beach;
  beach.biomeName = TEXT("Beach");
  beach.minHeight = 0.35f;
  beach.maxHeight = 0.45f;
  FColor sandColor_1 = FColor(255, 235, 175, 1);
  //FColor sandColor_2 = FColor(255, 230, 160, 1);
  //FColor sandColor_3 = FColor(255, 225, 140, 1);
  beach.vertexColors.Add(sandColor_1);
  //beach.vertexColors.Add(sandColor_2);
  //beach.vertexColors.Add(sandColor_3);
  biomeList.Add(beach);

  // Beach / Plains
  FBiomeSettings beach_plain;
  beach_plain.biomeName = TEXT("Beach-Plain");
  beach_plain.minHeight = 0.45f;
  beach_plain.maxHeight = 0.55f;
  FColor sandgrassColor_1 = FColor(190, 220, 170, 1);
  //FColor sandgrassColor_2 = FColor(220, 240, 180, 1);
  //FColor sandgrassColor_3 = FColor(240, 245, 190, 1);
  beach_plain.vertexColors.Add(sandgrassColor_1);
  //beach_plain.vertexColors.Add(sandgrassColor_2);
  //beach_plain.vertexColors.Add(sandgrassColor_3);
  // Bush
  FAssetSettings beachplainMesh;
  ConstructorHelpers::FObjectFinder<UStaticMesh> bush(TEXT("StaticMesh'/Game/Meshes/Bush/SM_bush6.SM_bush6'"));
  if (bush.Succeeded()) {
    beachplainMesh.mesh = bush.Object;
  }
  beach_plain.asset = beachplainMesh;
  beach_plain.asset.probability = 0.1f;
  biomeList.Add(beach_plain);

  // Plains
  FBiomeSettings plain;
  plain.biomeName = TEXT("Plain");
  plain.minHeight = 0.55f;
  plain.maxHeight = 0.75f;
  FColor grassColor_1 = FColor(110, 200, 110, 1);
  //FColor grassColor_2 = FColor(95, 190, 95, 1);
  //FColor grassColor_3 = FColor(120, 220, 120, 1);
  plain.vertexColors.Add(grassColor_1);
  //plain.vertexColors.Add(grassColor_2);
  //plain.vertexColors.Add(grassColor_3);
  // Trees
  FAssetSettings plainMesh;
  ConstructorHelpers::FObjectFinder<UStaticMesh> tree(TEXT("StaticMesh'/Game/Meshes/Trees/SM_tree8.SM_tree8'"));
  if (tree.Succeeded()) {
    plainMesh.mesh = tree.Object;
  }
  plain.asset = plainMesh;
  plain.asset.probability = 0.1f;
  biomeList.Add(plain);

  // Mountains
  FBiomeSettings mountain;
  mountain.biomeName = TEXT("Mountain");
  mountain.minHeight = 0.75f;
  mountain.maxHeight = 0.95f;
  FColor rockColor_1 = FColor(65, 65, 65, 1);
  //FColor rockColor_2 = FColor(170, 170, 170, 1);
  //FColor rockColor_3 = FColor(160, 160, 160, 1);
  mountain.vertexColors.Add(rockColor_1);
  //mountain.vertexColors.Add(rockColor_2);
  //mountain.vertexColors.Add(rockColor_3);
  // Rocks
  FAssetSettings mountainMesh;
  ConstructorHelpers::FObjectFinder<UStaticMesh> rock(TEXT("StaticMesh'/Game/SoulCave/Environment/Meshes/Rocks/SM_Cave_Rock_Medium01.SM_Cave_Rock_Medium01'"));
  if (tree.Succeeded()) {
    mountainMesh.mesh = rock.Object;
  }
  mountain.asset = mountainMesh;
  mountain.asset.probability = 0.02f;
  mountain.asset.randomRotation = true;
  mountain.asset.randomScale = true;
  mountain.asset.collision = true;
  biomeList.Add(mountain);

  // Snow
  FBiomeSettings snow;
  snow.biomeName = TEXT("Snow");
  snow.minHeight = 0.95f;
  snow.maxHeight = 1.0f;
  FColor snowColor_1 = FColor(255, 255, 255, 1);
  //FColor snowColor_2 = FColor(230, 230, 230, 1);
  snow.vertexColors.Add(snowColor_1);
  //snow.vertexColors.Add(snowColor_2);
  biomeList.Add(snow);
}