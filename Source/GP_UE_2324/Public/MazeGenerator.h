// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "MazeCell.h"
#include "GameEnums.h"
#include "MazeGenerator.generated.h"

UCLASS()
class GP_UE_2324_API AMazeGenerator : public AActor
{
    GENERATED_BODY()

public:
    AMazeGenerator();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, Category = "Maze Configuration")
    TSubclassOf<AMazeCell> BPMazeCell;

    UPROPERTY(EditAnywhere, Category = "Maze Configuration")
    AActor* Exit;

    UPROPERTY(EditAnywhere, Category = "Maze Configuration")
    AActor* Key;

    UPROPERTY(EditAnywhere, Category = "Maze Configuration")
    int MazeWidth;

    UPROPERTY(EditAnywhere, Category = "Maze Configuration")
    int MazeDepth;

    UPROPERTY(EditAnywhere, Category = "Maze Configuration")
    float EllersMergeProb;

    UPROPERTY(EditAnywhere, Category = "Maze Configuration")
    float ElevationRatioIn;

    UPROPERTY(EditAnywhere, Category = "Maze Configuration")
    int CellSizeIn;

    static void GenerateCellMesh(AMazeCell* CurrentCell, AMazeCell* NextCell);

private:
    USceneComponent* Root;

    static float ElevationRatio;
    static int CellSize;

    UPROPERTY(EditAnywhere, Category = "Maze Configuration")
    int VoronoidCellSize;

    UPROPERTY(EditAnywhere, Category = "Maze Configuration")
    TArray<FColor> PossibleColors;

    UPROPERTY(EditAnywhere, Category = "Maze Configuration")
    FColor AreaEVA;

    UPROPERTY(EditAnywhere, Category = "Maze Configuration")
    FColor AreaPlant;

    TArray<AMazeCell*> MazeGrid;
    TMap<int, TArray<int>> CellSets;
    TArray<AMazeCell*> VoronoidGrid;
    int VoronoidGridWidth;
    int VoronoidGridDepth;
    AMazeCell* StartCell;

    void GenerateMaze();
    int32 FindSet(int32 CellIndex);
    void MergeSets(int32 SetFrom, int32 SetTo);
    void SetExitAndKey();

    static EDirection GetDirection(const AMazeCell* CurrentCell, const AMazeCell* NextCell);
    AMazeCell* GetCellInDirection(AMazeCell* CurrentCell, EDirection Direction);

    void SetColorVoronoid();
};