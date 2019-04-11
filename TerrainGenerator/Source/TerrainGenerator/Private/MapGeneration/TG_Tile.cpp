// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#include "TG_Tile.h"
#include "TG_TerrainGenerator.h"

#include "RuntimeMeshLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogTile, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogTileAsync, Log, All);

// Sets default values
ATG_Tile::ATG_Tile()
{
  // Root
  USceneComponent* root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
  RootComponent = root;

  // Terrain
  RuntimeMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("RuntimeMeshC"));
  RuntimeMesh->SetupAttachment(RootComponent);

  // Water
  waterComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterC"));
  waterComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ATG_Tile::BeginPlay()
{
	Super::BeginPlay();

}

// Sets default values
void ATG_Tile::Init(int tileID, int coordX, int coordY, FTileSettings tSettings, ATG_TerrainGenerator* manager)
{
  UE_LOG(LogTile, Log, TEXT("TILE[%d] Init Tile"), tileID, coordX, coordY);
  // The Manager
  TerrainGenerator = manager;

  // Tile Info
  TileID = tileID;
  TileSeed = manager->Seed * coordX + coordY;
  TileX = coordX;
  TileY = coordY;
  tileSettings = tSettings;
 
  // Set the Name of the Tile
  AsyncTask(ENamedThreads::GameThread, [&]() { setTileName(TerrainGenerator->TileName); });

  // Set the terrain on the middle of the Actor
  AsyncTask(ENamedThreads::GameThread, [&]() {InitTerrainPosition(); });

  // Move Tile to World Position
  AsyncTask(ENamedThreads::GameThread, [&]() { SetTileWorldPosition(TileX, TileY, tileSettings); });

  // Initialize the values to default
  InitMeshToCreate();

  // Generate everything
  GenerateTriangles();
  GenerateVertices();
  GenerateNormalTangents(false);

  // Set Water
  AsyncTask(ENamedThreads::GameThread, [&]() { SetupWater(tileSettings, TerrainGenerator); });

  // Set the Biomes
  AsyncTask(ENamedThreads::GameThread, [&]() { SetupBiomes(tileSettings, TerrainGenerator); });

  if (Generated == false)
  {
    // Create the Mesh
    AsyncTask(ENamedThreads::GameThread, [&]() { GenerateMesh(TerrainGenerator->defaultMaterial); });
  }
  else
  {
    // Update the Mesh
    AsyncTask(ENamedThreads::GameThread, [&]() { UpdateMesh(TerrainGenerator->defaultMaterial); });
  }

  // Set Tile is Visible
  AsyncTask(ENamedThreads::GameThread, [&]() { SetVisibile(true); });

}

void ATG_Tile::Update(int coordX, int coordY) {

  // Calculate if this Tile is Visible or Not
  FVector viewer = TerrainGenerator->player->GetActorLocation();
  viewer.Z = 0.f;

  float viewerDstFromNearestEdge = FVector::Dist2D(this->GetActorLocation(), viewer);
  bool visibility = viewerDstFromNearestEdge <= TerrainGenerator->maxViewDistance;
  SetVisibile(visibility);
}

void ATG_Tile::InitMeshToCreate()
{
  UE_LOG(LogTile, Log, TEXT("TILE[%d] Initialize Mesh Values"), TileID);
  ZPositions.Init(0.0, tileSettings.ArraySize);
  MeshToCreate.Vertices.Init(FVector(0.0, 0.0, 0.0), tileSettings.ArraySize);
  MeshToCreate.Normals.Init(FVector(0, 0, 1), tileSettings.ArraySize);
  MeshToCreate.Tangents.Init(FRuntimeMeshTangent(0, -1, 0), tileSettings.ArraySize);
  MeshToCreate.UV.Init(FVector2D(0, 0), tileSettings.ArraySize);
  MeshToCreate.VertexColors.Init(FColor::White, tileSettings.ArraySize);
  int QuadSize = 6;
  int NumberOfQuadsPerLine =  tileSettings.getArrayLineSize();
  int TrianglesArraySize = NumberOfQuadsPerLine * NumberOfQuadsPerLine * QuadSize;
  MeshToCreate.Triangles.Init(0, TrianglesArraySize);
}

void ATG_Tile::GenerateVertices()
{
  UE_LOG(LogTile, Log, TEXT("TILE[%d] Generating Vertices"), TileID);

  int NumberOfQuadsPerLine = tileSettings.getArrayLineSize();
  for (int y = 0; y < NumberOfQuadsPerLine; y++) {
    for (int x = 0; x < NumberOfQuadsPerLine; x++) {
      FVector2D Position = GetVerticePosition(x, y);

      double AlgorithmZ = GetNoiseValueForGridCoordinates(Position.X, Position.Y);
            
      double ZPos = ScaleZWithHeightRange(AlgorithmZ);
      FVector value = FVector(Position.X, Position.Y, ZPos);
      int index = GetValueIndexForCoordinates(x, y);
      // Set the Algorithm Value At X & Y coordinates
      MeshToCreate.Vertices[GetValueIndexForCoordinates(x, y)] = value;
      // Calculate the UV
      MeshToCreate.UV[index] = CalculateUV(x, y);

      // Save the Z Position
      ZPositions[GetValueIndexForCoordinates(x, y)] = ZPos;
      // Save the Maximum Z Position
      if (ZPos > TerrainGenerator->maxHeight) {
        TerrainGenerator->maxHeight = ZPos;
      }

    }
  }
}

void ATG_Tile::GenerateTriangles()
{
  UE_LOG(LogTile, Log, TEXT("TILE[%d] Generating Triangles"), TileID);
  int QuadSize = 6;
  int NumberOfQuadsPerLine = tileSettings.getArrayLineSize() - 1;

  for (int y = 0; y < NumberOfQuadsPerLine; y++) {
    for (int x = 0; x < NumberOfQuadsPerLine; x++) {
      int QIndex = x + y * NumberOfQuadsPerLine;
      int TIndex = QIndex * QuadSize;

      int botLeft = GetValueIndexForCoordinates(x, y);
      int topLeft = GetValueIndexForCoordinates(x, y + 1);
      int topRight = GetValueIndexForCoordinates(x + 1, y + 1);
      int botRight = GetValueIndexForCoordinates(x + 1, y);
      MeshToCreate.Triangles[TIndex] = botLeft;
      MeshToCreate.Triangles[TIndex + 1] = topLeft;
      MeshToCreate.Triangles[TIndex + 2] = topRight;
      MeshToCreate.Triangles[TIndex + 3] = botLeft;
      MeshToCreate.Triangles[TIndex + 4] = topRight;
      MeshToCreate.Triangles[TIndex + 5] = botRight;
    }
  }
}

void ATG_Tile::GenerateNormalTangents(bool SmoothNormals) {
  // Calculate Normals And Tangents
  URuntimeMeshLibrary::CalculateTangentsForMesh(MeshToCreate.Vertices, MeshToCreate.Triangles, MeshToCreate.Normals,
    MeshToCreate.UV, MeshToCreate.Tangents, SmoothNormals);
}

void ATG_Tile::GenerateMesh(UMaterialInterface* material)
{
  UE_LOG(LogTile, Log, TEXT("TILE[%d] Generating Mesh"), TileID);
  //RuntimeMesh->SetMaterial(TileID, material);
  RuntimeMesh->CreateMeshSection(TileID,
    MeshToCreate.Vertices,
    MeshToCreate.Triangles,
    MeshToCreate.Normals,
    MeshToCreate.UV,
    MeshToCreate.VertexColors,
    MeshToCreate.Tangents,
    true,
    EUpdateFrequency::Infrequent,
    ESectionUpdateFlags::None);

  RuntimeMesh->SetSectionMaterial(TileID, material);

  // Set Generated Tile to True
  Generated = true;
}

void ATG_Tile::UpdateMesh(UMaterialInterface* material)
{
  UE_LOG(LogTile, Log, TEXT("TILE[%d] Update Mesh"), TileID);
  //RuntimeMesh->SetMaterial(TileID, material);

  RuntimeMesh->UpdateMeshSection(TileID,
    MeshToCreate.Vertices,
    MeshToCreate.Triangles,
    MeshToCreate.Normals,
    MeshToCreate.UV,
    MeshToCreate.VertexColors,
    MeshToCreate.Tangents,
    ESectionUpdateFlags::None);

  RuntimeMesh->SetSectionMaterial(TileID, material);
}

void ATG_Tile::SetupWater(FTileSettings tSettings, ATG_TerrainGenerator* manager)
{
  UE_LOG(LogTile, Log, TEXT("TILE[%d] Setup Water"), TileID);
  // If not use the water hide plane and disable collision JUST IN CASE
  if (false == manager->useWater)
  {
    waterComponent->SetHiddenInGame(true);
    waterComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    return;
  }

  // Set the mesh to use
  waterComponent->SetStaticMesh(manager->water);

  // Set the material for the water
  waterComponent->SetMaterial(0, manager->waterMaterial);

  // Height of the Water
  float waterHeightPos = TerrainGenerator->maxHeight * manager->waterHeight;
  FVector waterPos = FVector(0.f, 0.f, waterHeightPos);
  waterComponent->SetRelativeLocation(waterPos);


  // Scale of the Water
  FVector scale = waterComponent->CalcBounds(waterComponent->GetRelativeTransform()).BoxExtent;
  float waterPlaneScaleX = (tSettings.getTileSize() / scale.X) / 2.f;
  float waterPlaneScaleY = (tSettings.getTileSize() / scale.Y) / 2.f;
  scale = FVector(waterPlaneScaleX, waterPlaneScaleY, 1.f);

  // Only set if the scale is greater than 1.f
  if ( !(scale == FVector(1.f, 1.f, 1.f)) ) {
    waterComponent->SetRelativeScale3D(FVector(waterPlaneScaleX, waterPlaneScaleY, 1.f));
  }
}

void ATG_Tile::SetupBiomes(FTileSettings tSettings, ATG_TerrainGenerator* manager)
{
  UE_LOG(LogTile, Log, TEXT("TILE[%d] Setup Biomes"), TileID);

  if (manager->useVertexColor == true) {

    // For each Biome
    for (int indexBiome = 0; indexBiome < manager->biomeList.Num(); indexBiome++)
    {
      FBiomeSettings lowerBiome;

      // Vertices
      for (int i = 0; i < tSettings.ArraySize; i++) {
        // Get Perlin Value
        float ZPos = ZPositions[i] / TerrainGenerator->maxHeight;

        // Set the VertexColor depend the Height
        if (ZPos < 0.0 && manager->biomeList[indexBiome].minHeight == 0.0) {
          // Select a random Color
          int numberOfColors = manager->biomeList[indexBiome].vertexColors.Num() - 1;
          int randomVertexColor = FMath::RandRange(0, numberOfColors);

          // Set the Vertex Color
          MeshToCreate.VertexColors[i] = manager->biomeList[indexBiome].vertexColors[randomVertexColor];
        }
        else if (ZPos >= manager->biomeList[indexBiome].minHeight) {
          if (ZPos < manager->biomeList[indexBiome].maxHeight) {

            // Select a random Color
            int numberOfColors = manager->biomeList[indexBiome].vertexColors.Num() - 1;
            int randomVertexColor = FMath::RandRange(0, numberOfColors);

            // Set the Vertex Color
            MeshToCreate.VertexColors[i] = manager->biomeList[indexBiome].vertexColors[randomVertexColor];
          }
        }
      }
    }
  }
}

FString ATG_Tile::GetTileNameCoords(int x, int y)
{
  return FString::Printf(TEXT("%i-%i:%i-%i"), TileX, TileY, x, y);
}

FVector2D ATG_Tile::GetVerticePosition(float x, float y)
{
  return FVector2D(
    x * tileSettings.getLOD(),
    y * tileSettings.getLOD()
  );
}

FVector2D ATG_Tile::CalculateUV(float x, float y)
{
  return FVector2D(
    x / tileSettings.TextureScale,
    y / tileSettings.TextureScale
  );
}

float ATG_Tile::ScaleZWithHeightRange(double value) {
  return value * tileSettings.getHeightRange();
}

int ATG_Tile::GetValueIndexForCoordinates(int x, int y)
{
  return x + y * tileSettings.getArrayLineSize();
}

double ATG_Tile::GetNoiseValueForGridCoordinates(double x, double y)
{
  double worldX = (TileX * tileSettings.getTileSize() + x);
  double worldY = (TileY * tileSettings.getTileSize() + y);
   
  return TerrainGenerator->GetAlgorithmValue(worldX, worldY);
}

void ATG_Tile::SetTileWorldPosition(int coordX, int coordY, FTileSettings tSettings)
{
  FVector position = FVector::ZeroVector;

  // Set the Coordinates
  position = FVector(
    coordX * tSettings.getTileSize(),
    coordY * tSettings.getTileSize(),
    0.f
  );

  SetActorLocation(position);
}

void ATG_Tile::InitTerrainPosition() {
	// Position of the Terrain
	FVector position = FVector(tileSettings.getTileSize() / 2, tileSettings.getTileSize() / 2, 0.f);
	RuntimeMesh->SetRelativeLocation(position * -1);
}

void ATG_Tile::SetVisibile(bool option)
{
  Visible = option;
  
  // Show or Hide the Mesh
  RuntimeMesh->SetVisibility(option, true);

  // Show or Hide The Water
  waterComponent->SetVisibility(option, true);
}

void ATG_Tile::setTileName(FName text) {
  //Name of the Tile
#if WITH_EDITOR
  if (text != TEXT("")) {
    this->SetActorLabel(text.ToString() + " [" + FString::FromInt(TileX) + "," + FString::FromInt(TileY) + "]");
  }
#endif
}