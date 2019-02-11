// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include "RuntimeMeshCore.h"
#include "TG_MeshSettings.generated.h"

USTRUCT(BlueprintType)
struct FMeshSettings {
  GENERATED_USTRUCT_BODY()

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MeshSettings")
    TArray<FVector> Vertices;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MeshSettings")
    TArray<FVector> Normals;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MeshSettings")
    TArray<FRuntimeMeshTangent> Tangents;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MeshSettings")
    TArray<FVector2D> UV;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MeshSettings")
    TArray<FColor> VertexColors;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MeshSettings")
    TArray<int> Triangles;
};