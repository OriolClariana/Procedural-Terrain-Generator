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
  root->SetMobility(EComponentMobility::Static);
  RootComponent = root;

  // Terrain
  RuntimeMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("RuntimeMeshC"));
  RuntimeMesh->SetupAttachment(RootComponent);
  RuntimeMesh->SetMobility(EComponentMobility::Static);

  // Water
  waterComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterC"));
  waterComponent->SetupAttachment(RootComponent);
  waterComponent->bCastDynamicShadow = false;
  waterComponent->CastShadow = false;
  waterComponent->SetMobility(EComponentMobility::Movable);

  // Assets for Biomes  6 = Number of Biomes
  for (int i = 0; i < 6; ++i){
    // Create InstancedStaticMesh
    UInstancedStaticMeshComponent *ISMComp = NewObject<UInstancedStaticMeshComponent>();
    ISMComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedStaticMeshC_Biome_" + i));
    ISMComp->SetupAttachment(RootComponent);
    ISMComp->bCastDynamicShadow = true;
    ISMComp->CastShadow = true;

    //Visibility
    ISMComp->SetHiddenInGame(false);

    //Mobility
    ISMComp->SetMobility(EComponentMobility::Stationary);

    //Collision
    ISMComp->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // Add this to the List
    InstancedList.Add(ISMComp);
  }

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

  maxDistanceForAssets = TerrainGenerator->maxViewDistance / TerrainGenerator->numReducesMaxViewDistAssets;
 
  // Set the Name of the Tile
  AsyncTask(ENamedThreads::GameThread, [&]() { setTileName(TerrainGenerator->TileName); });
  
  // Initialize the values to default
  InitMeshToCreate();

  // Generate everything
  GenerateTriangles();
  GenerateVertices();
  GenerateNormalTangents(false);

  // Set Water
  AsyncTask(ENamedThreads::GameThread, [&]() { SetupWater(tileSettings); });

  // Set the Biomes
  AsyncTask(ENamedThreads::GameThread, [&]() { SetupBiomes(tileSettings); });

  // Set the Assets
  AsyncTask(ENamedThreads::GameThread, [&]() { SetupAssets(tileSettings); });

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
  AsyncTask(ENamedThreads::GameThread, [&]() { SetVisibileAsset(true); });
}

void ATG_Tile::Update(int coordX, int coordY) {
  if (TerrainGenerator) {
    // Calculate if this Tile is Visible or Not
    FVector viewer = TerrainGenerator->player->GetActorLocation();
    viewer.Z = 0.f;

    float viewerDstFromNearestEdge = FVector::Dist2D(this->GetActorLocation(), viewer);

    // The Tile (Terrain & Water)
    bool vTerrainWater = viewerDstFromNearestEdge <= TerrainGenerator->maxViewDistance;
    SetVisibile(vTerrainWater);

    // If the Tile exist set the visibility of the assets
    if (vTerrainWater) {
      if (viewerDstFromNearestEdge <= maxDistanceForAssets) {
        SetVisibileAsset(true);
      }
    }
  }
}

bool ATG_Tile::DestroyTile() {
  // Hide the Tile
  SetVisibile(false);
  SetVisibileAsset(false);

  // Clear Reference to the Terrain Generator Manager
  TerrainGenerator = nullptr;

  // Destroy Instances
  for (auto instanceElement : InstancedList) {
    instanceElement->DestroyComponent(true);
  }

  // Destroy Water
  waterComponent->DestroyComponent(true);

  // Destroy RuntimeMesh
  RuntimeMesh->ClearAllMeshSections();
  RuntimeMesh->DestroyComponent(true);

  return Destroy(true);
}

void ATG_Tile::InitMeshToCreate()
{
  UE_LOG(LogTile, Log, TEXT("TILE[%d] Initialize Mesh Values"), TileID);
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
  if (TerrainGenerator) {
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
        ZPositions.Add(FVector2D(x, y), ZPos);
        // Save the Maximum Z Position
        if (ZPos > TerrainGenerator->maxHeight) {
          TerrainGenerator->maxHeight = ZPos;
        }

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

void ATG_Tile::SetupWater(FTileSettings tSettings)
{
  if (TerrainGenerator) {
    UE_LOG(LogTile, Log, TEXT("TILE[%d] Setup Water"), TileID);
    // If not use the water hide plane and disable collision JUST IN CASE
    if (false == TerrainGenerator->useWater)
    {
      waterComponent->SetHiddenInGame(true);
      waterComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
      return;
    }

    // Set the mesh to use
    waterComponent->SetStaticMesh(TerrainGenerator->water);

    // Set the material for the water
    waterComponent->SetMaterial(0, TerrainGenerator->waterMaterial);

    // Height of the Water
    float waterHeightPos = TerrainGenerator->maxHeight * TerrainGenerator->waterHeight;
    FVector waterPos = FVector(0.f, 0.f, waterHeightPos);
    waterComponent->SetRelativeLocation(waterPos);


    // Scale of the Water
    FVector scale = waterComponent->CalcBounds(waterComponent->GetRelativeTransform()).BoxExtent;
    float waterPlaneScaleX = (tSettings.getTileSize() / scale.X) / 2.f;
    float waterPlaneScaleY = (tSettings.getTileSize() / scale.Y) / 2.f;
    scale = FVector(waterPlaneScaleX, waterPlaneScaleY, 1.f);

    // Only set if the scale is greater than 1.f
    if (!(scale == FVector(1.f, 1.f, 1.f))) {
      waterComponent->SetRelativeScale3D(FVector(waterPlaneScaleX, waterPlaneScaleY, 1.f));
    }
  }
}

void ATG_Tile::SetupBiomes(FTileSettings tSettings)
{
  if (TerrainGenerator) {
    UE_LOG(LogTile, Log, TEXT("TILE[%d] Setup Biomes"), TileID);

    if (TerrainGenerator->useVertexColor == true) {

      // For each Biome
      for (int indexBiome = 0; indexBiome < TerrainGenerator->biomeList.Num(); indexBiome++)
      {
        // Vertices
        int i = 0;
        for (auto Elem : ZPositions) {
          // Get Perlin Value
          float ZPos = Elem.Value / TerrainGenerator->maxHeight;

          // Clamp
          if (ZPos < 0.0f) { ZPos = 0.0f; }
          else if (ZPos >= 1.0f) { ZPos = 1.f; }

          // Set the VertexColor depend the Height
          if (ZPos >= TerrainGenerator->biomeList[indexBiome].minHeight) {
            if (ZPos <= TerrainGenerator->biomeList[indexBiome].maxHeight) {

              // Select a random Color
              int numberOfColors = TerrainGenerator->biomeList[indexBiome].vertexColors.Num() - 1;
              int randomVertexColor = FMath::RandRange(0, numberOfColors);

              // Set the Vertex Color
              MeshToCreate.VertexColors[i] = TerrainGenerator->biomeList[indexBiome].vertexColors[randomVertexColor];
            }
          }
          ++i;
        }
      }
    }

    if (TerrainGenerator->useHeightMap == true) {
      // Calculate the Height Map
      for (int indexBiome = 0; indexBiome < TerrainGenerator->biomeList.Num(); indexBiome++)
      {
        FBiomeSettings lowerBiome;

        // Vertices
        int i = 0;
        for (auto Elem : ZPositions) {
          // Get Perlin Value
          float ZPos = Elem.Value / TerrainGenerator->maxHeight;

          float clamped = ZPos * 255;
          if (clamped < 0.0f) {
            clamped = 0.0f;
          }

          // Set the Vertex Color
          MeshToCreate.VertexColors[i] = FColor(clamped, clamped, clamped);
          ++i;
        }
      }
    }
  }
}

void ATG_Tile::SetupAssets(FTileSettings tSettings) {
  if (TerrainGenerator) {
    UE_LOG(LogTile, Log, TEXT("TILE[%d] Setup Assets"), TileID);
    if (TerrainGenerator->spawnAssets) {
      // Set Seed of this tile for the randome trees
      FMath::RandInit(TileSeed);

      // For each Biome
      for (int indexBiome = 0; indexBiome < TerrainGenerator->biomeList.Num(); indexBiome++)
      {
        // If exist some type of asset
        if (TerrainGenerator->biomeList[indexBiome].asset.mesh != nullptr) {

          //Set the Mesh to the Instance
          InstancedList[indexBiome]->SetStaticMesh(TerrainGenerator->biomeList[indexBiome].asset.mesh);

          // Vertices
          for (auto Elem : ZPositions) {
            // Get Perlin Value
            float ZPos = Elem.Value / TerrainGenerator->maxHeight;

            // Clamp
            if (ZPos < 0.0f) { ZPos = 0.0f; }
            else if (ZPos >= 1.0f) { ZPos = 1.f; }

            // Set the VertexColor depend the Height
            if (ZPos >= TerrainGenerator->biomeList[indexBiome].minHeight) {
              if (ZPos <= TerrainGenerator->biomeList[indexBiome].maxHeight) {
                // Get the asset Settings
                FAssetSettings asset = TerrainGenerator->biomeList[indexBiome].asset;

                // Get Perlin Noise from Assets value
                float randomAsset = FMath::RandRange(0.0f, 1.f);

                // If exist asset here
                if (randomAsset <= asset.probability) {
                  FVector thisTrans = this->GetActorLocation();
                  float posX = TileX * tSettings.getTileSize();
                  float posY = TileY * tSettings.getTileSize();
                  FVector2D localPos = Elem.Key * tSettings.getLOD();

                  // Asset Position
                  FVector assetLocation(localPos.X, localPos.Y, Elem.Value);

                  // Asset Scale
                  FVector assetScale = FVector(1.f, 1.f, 1.f);
                  if (asset.randomScale) {
                    assetScale.X = FMath::FRandRange(1.f, asset.maxRandomScale.X);
                    assetScale.Y = FMath::FRandRange(1.f, asset.maxRandomScale.Y);
                    assetScale.Z = FMath::FRandRange(1.f, asset.maxRandomScale.Z);
                  }

                  // Asset Rotation
                  FRotator assetRotation = FRotator::ZeroRotator;
                  if (asset.randomRotation) {
                    assetRotation.Pitch = FMath::FRandRange(0.f, 360.f);
                    assetRotation.Roll = FMath::FRandRange(0.f, 360.f);
                    assetRotation.Yaw = FMath::FRandRange(0.f, 360.f);
                  }

                  // Asset Collision
                  if (!asset.collision) {
                    InstancedList[indexBiome]->BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);
                  }

                  //Add the asset to the Instanced Object
                  InstancedList[indexBiome]->AddInstance(FTransform(assetRotation, assetLocation, assetScale));
                }
              } // biome max height
            } // biome min height
          } //end for each vertex
        } //end if existAsset
      } //end for biome list
    } //end if spawnAssets
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
  return x + (y * tileSettings.getArrayLineSize());
}

FVector2D ATG_Tile::GetCoordsWithIndex(int index)
{
  int y = index % tileSettings.ArrayLineSize;
  int x = index / tileSettings.ArrayLineSize;

  return FVector2D(x, y);
}

FVector2D ATG_Tile::CalculateWorldPosition(float x, float y) {
  double worldX = (TileX * tileSettings.getTileSize() + x);
  double worldY = (TileY * tileSettings.getTileSize() + y);
  return FVector2D(worldX, worldY);
}

double ATG_Tile::GetNoiseValueForGridCoordinates(double x, double y)
{   
  FVector2D world = CalculateWorldPosition(x, y);

  return TerrainGenerator->GetAlgorithmValue(world.X, world.Y);
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
  FVector position = FVector(tileSettings.getTileSize() / 2, tileSettings.getTileSize() / 2, 0.f);

  // Position of the Terrain
	RuntimeMesh->SetRelativeLocation(position * -1);

  // Position of the Instanced Asset
  for (int i = 0; i < InstancedList.Num(); ++i) {
    InstancedList[i]->SetRelativeLocation(position * -1);
  }  
}

void ATG_Tile::SetVisibile(bool option)
{
  Visible = option;
  
  // Show or Hide the Mesh
  RuntimeMesh->SetVisibility(option, true);

  // Show or Hide The Water
  waterComponent->SetVisibility(option, true);
}

void ATG_Tile::SetVisibileAsset(bool option)
{
  // Show or Hide the Assets
  for (int i = 0; i < InstancedList.Num(); ++i) {
    InstancedList[i]->SetVisibility(option, true);
  }
}

void ATG_Tile::setTileName(FName text) {
  //Name of the Tile
#if WITH_EDITOR
  if (text != TEXT("")) {
    this->SetActorLabel(text.ToString() + " [" + FString::FromInt(TileX) + "," + FString::FromInt(TileY) + "]");
  }
#endif
}