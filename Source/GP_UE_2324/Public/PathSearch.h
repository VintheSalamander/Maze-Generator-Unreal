// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MazeCell.h"

class AMazeCell;

class GP_UE_2324_API PathSearch
{
public:
    static TPair<AMazeCell*, AMazeCell*> GetExitAndKey(const TArray<AMazeCell*>& MazeGrid, AMazeCell* StartCell);
    static TPair<TArray<AMazeCell*>, AMazeCell*> BreadthSearchExit(AMazeCell* StartCell);
    static AMazeCell* GetKeyCell(const TArray<AMazeCell*>& MazeGrid, const TArray<AMazeCell*>& ExitPath);
    static AMazeCell* GetClosestVoronoid(const TArray<AMazeCell*>& MazeGrid, AMazeCell* Start, const TSet<AMazeCell*>& VoronoidPoints);
    
};

