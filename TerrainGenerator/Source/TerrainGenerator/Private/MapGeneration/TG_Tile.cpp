// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#include "TG_Tile.h"

DEFINE_LOG_CATEGORY_STATIC(LogTile, Log, All);

// Sets default values
ATG_Tile::ATG_Tile()
{
  RuntimeMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("RuntimeMeshC"));
}

// Called when the game starts or when spawned
void ATG_Tile::BeginPlay()
{
	Super::BeginPlay();

}

// Sets default values
void ATG_Tile::Init(int tileID, int coordX, int coordY, FTileSettings tSettings)
{
  UE_LOG(LogTile, Log, TEXT("INIT Tile ID [%d] Coords X: %d, Coords Y: %d"), tileID, coordX, coordY);
  TileID = tileID;
  TileX = coordX;
  TileY = coordY;

  tileSettings = tSettings;

  InitMeshToCreate();
  GenerateVertices();
  GenerateTriangles();
  GenerateMesh();

  SetTileWorldPosition(coordX, coordY);

  // Set Generated Tile to True
  Generated = true;
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
  int NumberOfQuadsPerLine =  tileSettings.ArrayLineSize - 1;
  int TrianglesArraySize = NumberOfQuadsPerLine * NumberOfQuadsPerLine * QuadSize;
  MeshToCreate.Triangles.Init(0, TrianglesArraySize);
}

void ATG_Tile::GenerateVertices()
{
  UE_LOG(LogTile, Log, TEXT("Generating vertices for Tile ID [%d]"), TileID);
  for (int y = 0; y < tileSettings.ArrayLineSize; y++) {
    for (int x = 0; x < tileSettings.ArrayLineSize; x++) {
      FVector2D Position = GetTileWorldPosition(x, y);
      float AlgorithmResult = (float)(FMath::RandRange(0, 100) / 100.f); // TEMPORAL, HERE PROCEDURAL VALUE
      float ZPos = GetZPositionFromAlgorithmResult(AlgorithmResult);
      FVector value = FVector(Position.X, Position.Y, ZPos);
      int index = 0; //GetNoiseIndexForCoordinates(x, y);
      SetAlgorithmValueAt(x, y, value);
      MeshToCreate.UV[index] = CalculateUV(x, y);
    }
  }
}

void ATG_Tile::GenerateTriangles()
{
  UE_LOG(LogTile, Log, TEXT("Generating Triangles for Tile ID [%d]"), TileID);
  int QuadSize = 6;
  int NumberOfQuadsPerLine = tileSettings.ArrayLineSize - 1;
  int TrianglesArraySize = NumberOfQuadsPerLine * NumberOfQuadsPerLine * QuadSize;
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
                                        //FVector ThirdEdgeVector = Vertices[topLeft] - Vertices[topRight];
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
  RuntimeMesh->SetMaterial(TileID, tileSettings.Material);
  RuntimeMesh->CreateMeshSection(TileID,
    MeshToCreate.Vertices,
    MeshToCreate.Triangles,
    MeshToCreate.Normals,
    MeshToCreate.UV,
    MeshToCreate.VertexColors,
    MeshToCreate.Tangents,
    true, EUpdateFrequency::Infrequent);
  //RuntimeMesh->SetMeshSectionCastsShadow(TileID, false);
}

FString ATG_Tile::GetTileNameCoords(int x, int y)
{
  return FString::Printf(TEXT("%i-%i:%i-%i"), TileX, TileY, x, y);
}

FVector2D ATG_Tile::GetTileWorldPosition(float x, float y)
{
  return FVector2D(
    x * tileSettings.AlgorithmResolution,
    y * tileSettings.AlgorithmResolution
  );
}

FVector2D ATG_Tile::CalculateUV(float x, float y)
{
  return FVector2D(
    x / tileSettings.TextureScale,
    y / tileSettings.TextureScale
  );
}

float ATG_Tile::GetZPositionFromAlgorithmResult(float AlgorithmResult) {
  return AlgorithmResult * tileSettings.HeightRange;
}

int ATG_Tile::GetValueIndexForCoordinates(int x, int y)
{
  return x + y * tileSettings.ArrayLineSize;
}

void ATG_Tile::SetAlgorithmValueAt(int x, int y, FVector value)
{
  MeshToCreate.Vertices[GetValueIndexForCoordinates(x, y)] = value;
}

void ATG_Tile::SetTileWorldPosition(int coordX, int coordY)
{
  UE_LOG(LogTile, Log, TEXT("Set Tile[%d] to Tile Pos: %d - %d"), TileID, coordX, coordY);
  FVector position = FVector(
    coordX * tileSettings.TileSize,
    coordY * tileSettings.TileSize,
    0.f
  );
  SetActorLocation(position);
}