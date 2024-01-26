// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameEnums.h"
#include "ProceduralMeshComponent.h"
#include "MazeCell.generated.h"



UCLASS()
class GP_UE_2324_API AMazeCell : public AActor
{
    GENERATED_BODY()

public:
    AMazeCell();

protected:
    virtual void BeginPlay() override;

private:
    TArray<AMazeCell*> History;
    TArray<AMazeCell*> Neighbours;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MazeCell")
    UStaticMeshComponent* LeftWall;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MazeCell")
    UStaticMeshComponent* RightWall;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MazeCell")
    UStaticMeshComponent* BottomWall;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MazeCell")
    UStaticMeshComponent* TopWall;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MazeCell")
    UProceduralMeshComponent* Floor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MazeCell")
    UMaterialInterface* FloorMaterial;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MazeCell")
    TArray<FVector> CellVerts;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MazeCell")
    TArray<int32> CellTris;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MazeCell")
    bool IsVisited;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MazeCell")
    EElevation CellElevation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MazeCell")
    FColor Color;

  
    void BreakLeftWall();
    void BreakRightWall();
    void BreakTopWall();
    void BreakBottomWall();

    void Visit();
    void GenerateMesh(float Elevation, EDirection Direction, const TArray<FVector>& PrevCellVerts, float CellSize);
    const TArray<FVector>& GetCellVerts();
    void SetElevation(EElevation NewElevation);
    const EElevation GetElevation();
    void SetVert(EVert Vert, FVector Pos);
    void SetWallsColor(const FColor& NewColor);

    void SetHistory(const TArray<AMazeCell*>& NewHistory);
    const TArray<AMazeCell*>& GetHistory();
    const int32 GetHistoryCount() ;
    void AddHistory(AMazeCell* Addition);

    const TArray<AMazeCell*>& GetNeighbours();
    void AddNeighbour(AMazeCell* Neighbour);

    EDirection GetOpenDirection();



};
