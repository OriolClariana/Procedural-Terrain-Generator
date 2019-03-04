// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#include "TG_Tile.h"
#include "TG_TerrainGenerator.h"

DEFINE_LOG_CATEGORY_STATIC(LogTile, Log, All);

// Sets default values
ATG_Tile::ATG_Tile()
{
  RuntimeMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("RuntimeMeshC"));
  RootComponent = RuntimeMesh;

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
  UE_LOG(LogTile, Log, TEXT("INIT Tile ID [%d] Coords X: %d, Coords Y: %d"), tileID, coordX, coordY);
  TerrainGenerator = manager;

  TileID = tileID;
  TileSeed = manager->Seed * coordX + coordY;
  TileX = coordX;
  TileY = coordY;
  tileSettings = tSettings;

  // Set Water
  SetupWater();

  // Initialize the values to default
  InitMeshToCreate();

  // Generate everything
  GenerateTriangles();
  GenerateVertices();

  if (Generated == false)
  {
    // Create the Mesh
    GenerateMesh();
  }
  else
  {
    // Update the Mesh
    UpdateMesh();
  }

  // Move Tile to World Position
  SetTileWorldPosition(coordX, coordY);

  // Set Generated Tile to True
  Generated = true;
}

void ATG_Tile::Update(int coordX, int coordY) {

}

void ATG_Tile::InitMeshToCreate()
{
  UE_LOG(LogTile, Log, TEXT("INIT Mesh to Create in Tile ID [%d]"), TileID);
  MeshToCreate.Vertices.Init(FVector(0.0, 0.0, 0.0), tileSettings.ArraySize);
  MeshToCreate.Normals.Init(FVector(0, 0, 1), tileSettings.ArraySize);
  MeshToCreate.Tangents.Init(FRuntimeMeshTangent(0, -1, 0), tileSettings.ArraySize);
  MeshToCreate.UV.Init(FVector2D(0, 0), tileSettings.ArraySize);
  MeshToCreate.VertexColors.Init(FColor::White, tileSettings.ArraySize);
  int QuadSize = 6;
  int NumberOfQuadsPerLine =  tileSettings.getArrayLineSize() - 1;
  int TrianglesArraySize = NumberOfQuadsPerLine * NumberOfQuadsPerLine * QuadSize;
  MeshToCreate.Triangles.Init(0, TrianglesArraySize);
}

void ATG_Tile::GenerateVertices()
{
  UE_LOG(LogTile, Log, TEXT("Generating Vertices for Tile ID [%d]"), TileID);
  
  for (int y = 0; y < tileSettings.getArrayLineSize(); y++) {
    for (int x = 0; x < tileSettings.getArrayLineSize(); x++) {
      FVector2D Position = GetVerticePosition(x, y);

      double AlgorithmZ = GetNoiseValueForGridCoordinates(Position.X, Position.Y);

      double ZPos = ScaleZWithHeightRange(AlgorithmZ);
      FVector value = FVector(Position.X, Position.Y, ZPos);
      int index = GetValueIndexForCoordinates(x, y);
      SetAlgorithmValueAt(x, y, value);
      MeshToCreate.UV[index] = CalculateUV(x, y);
    }
  }
}

void ATG_Tile::GenerateTriangles()
{
  UE_LOG(LogTile, Log, TEXT("Generating Triangles for Tile ID [%d]"), TileID);
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

      FVector FirstEdgeVector = MeshToCreate.Vertices[topLeft] - MeshToCreate.Vertices[botLeft]; // Line between bottom and top of triangle
      FVector SecondEdgeVector = MeshToCreate.Vertices[topRight] - MeshToCreate.Vertices[botLeft]; // Line between bottom and right of triangle
      FVector NormalVector = FVector::CrossProduct(FirstEdgeVector, SecondEdgeVector) * -1;
      NormalVector.Normalize();
      SecondEdgeVector.Normalize();
      MeshToCreate.Normals[QIndex] = NormalVector;
      MeshToCreate.Tangents[QIndex] = SecondEdgeVector;
    }
  }
}

void ATG_Tile::GenerateMesh()
{
  UE_LOG(LogTile, Log, TEXT("Generating mesh for Tile ID [%d]"), TileID);
  RuntimeMesh->SetMaterial(TileID, TerrainGenerator->defaultMaterial);
  RuntimeMesh->CreateMeshSection(TileID,
    MeshToCreate.Vertices,
    MeshToCreate.Triangles,
    MeshToCreate.Normals,
    MeshToCreate.UV,
    MeshToCreate.VertexColors,
    MeshToCreate.Tangents,
    true, EUpdateFrequency::Infrequent);
}

void ATG_Tile::UpdateMesh()
{
  UE_LOG(LogTile, Log, TEXT("Update mesh for Tile ID [%d]"), TileID);

  RuntimeMesh->UpdateMeshSection(TileID,
    MeshToCreate.Vertices,
    MeshToCreate.Triangles,
    MeshToCreate.Normals,
    MeshToCreate.UV,
    MeshToCreate.VertexColors,
    MeshToCreate.Tangents,
    ESectionUpdateFlags::None);
}

void ATG_Tile::SetupWater()
{
  // If not use the water hidde plane and disable collision JUST IN CASE
  if (false == TerrainGenerator->useWater)
  {
    waterComponent->SetHiddenInGame(true);
    waterComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    return;
  }

  // Set the mesh to use
  waterComponent->SetStaticMesh(TerrainGenerator->water);

  // Height of the Water
  float heightMap = tileSettings.getTerrainScaleValue(tileSettings.HeightScale);
  float waterHeightPos = TerrainGenerator->waterHeight * heightMap;
  FVector waterPos = FVector(tileSettings.getTileSize() / 2, tileSettings.getTileSize() / 2, waterHeightPos);
  waterComponent->SetRelativeLocation(waterPos);

  // Scale of the Water
  //FVector scale = TerrainGenerator->water->GetBounds().GetBox().GetSize();
  float waterPlaneScale = tileSettings.getTileSize() / 1500.f;
  waterComponent->SetRelativeScale3D(FVector(waterPlaneScale, waterPlaneScale, 1));

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

float ATG_Tile::ScaleZWithHeightRange(double AlgorithmZ) {
  return AlgorithmZ * tileSettings.getHeightRange();
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

void ATG_Tile::SetAlgorithmValueAt(int x, int y, FVector value)
{
  MeshToCreate.Vertices[GetValueIndexForCoordinates(x, y)] = value;
}

void ATG_Tile::SetTileWorldPosition(int coordX, int coordY)
{
  UE_LOG(LogTile, Log, TEXT("Set Tile[%d] to Tile Pos: %d - %d"), TileID, coordX, coordY);

  FVector position = FVector::ZeroVector;

  // Tile Centered or not
  switch (tileSettings.TileCentred)
  {
  case true:
    position = FVector(
      (coordX * tileSettings.getTileSize() - (tileSettings.getTileSize() / 2)),
      (coordY * tileSettings.getTileSize() - (tileSettings.getTileSize() / 2)),
      0.f
    );
    break;
  case false:
    position = FVector(
      coordX * tileSettings.getTileSize(),
      coordY * tileSettings.getTileSize(),
      0.f
    );
    break;
  }

  SetActorLocation(position);
}