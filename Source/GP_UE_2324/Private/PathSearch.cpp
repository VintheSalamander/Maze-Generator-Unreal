// Fill out your copyright notice in the Description page of Project Settings.


#include "PathSearch.h"
#include "MazeGenerator.h"


TPair<AMazeCell*, AMazeCell*> PathSearch::GetExitAndKey(const TArray<AMazeCell*>& MazeGrid, AMazeCell* StartCell)
{
    TPair<TArray<AMazeCell*>, AMazeCell*> result = BreadthSearchExit(StartCell);
    AMazeCell* KeyCell = GetKeyCell(MazeGrid, result.Key);
    return TPair<AMazeCell*, AMazeCell*>(result.Value, KeyCell);
}

//Gets the Exit by doing a breadth search, the longest cell from the start is reached when the
//whole maze is already searched
TPair<TArray<AMazeCell*>, AMazeCell*> PathSearch::BreadthSearchExit(AMazeCell* StartCell)
{
    TArray<AMazeCell*> Result;
    TSet<AMazeCell*> Visited;
    TQueue<AMazeCell*> Work;

    Visited.Add(StartCell);
    Work.Enqueue(StartCell);

    while (!Work.IsEmpty())
    {
        AMazeCell* Current;
        Work.Dequeue(Current);
        TArray<AMazeCell*> CurrentNeighbours = Current->GetNeighbours();

        for (AMazeCell* Neighbour : CurrentNeighbours)
        {
            if (!Visited.Contains(Neighbour))
            {
                AMazeGenerator::GenerateCellMesh(Current, Neighbour);
                Neighbour->SetHistory(Current->GetHistory());
                Neighbour->AddHistory(Current);
                Visited.Add(Neighbour);
                Work.Enqueue(Neighbour);
            }
        }

        if (Work.IsEmpty())
        {
            Result = Current->GetHistory();
            Result.Add(Current);
            return TPair<TArray<AMazeCell*>, AMazeCell*>(Result, Current);
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Error in PathSearch::BreadthSearchExit()"));
    return TPair<TArray<AMazeCell*>, AMazeCell*>(TArray<AMazeCell*>(), nullptr);
}

//For the key cell we search the history of each cell which is the path from the start to that cell, then
//we compare that path to the exit path and the history who has the less matches its the second furthest cell
//which will be the key cell
AMazeCell* PathSearch::GetKeyCell(const TArray<AMazeCell*>& MazeGrid, const TArray<AMazeCell*>& ExitPath)
{
    AMazeCell* KeyCell = nullptr;
    int32 countCellsKey = 0;

    // Convert ExitPath to a TSet for faster lookup
    TSet<AMazeCell*> ExitPathSet(ExitPath);

    for (AMazeCell* CurrentCell : MazeGrid)
    {
        TArray <AMazeCell*> CurrentCellHistory = CurrentCell->GetHistory();
        int32 countNotInExit = 0;

        for (AMazeCell* CellinHistory : CurrentCellHistory)
        {
            if (!ExitPathSet.Contains(CellinHistory)) {
                countNotInExit++;
            }
        }

        if (countNotInExit > countCellsKey) {
            countCellsKey = countNotInExit;
            KeyCell = CurrentCell;
        }
    }

    if (KeyCell == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("KeyCell is null"));
    }

    return KeyCell;
}

//We do a breadth search from each cell to determine the closest voronoid also taking into account cases when
//there are two voronoids at the same distant from the cell
AMazeCell* PathSearch::GetClosestVoronoid(const TArray<AMazeCell*>& MazeGrid, AMazeCell* Start, const TSet<AMazeCell*>& VoronoidPoints)
{
    if (VoronoidPoints.Contains(Start))
    {
        return Start;
    }

    TArray<AMazeCell*> CloseVors;
    TSet<AMazeCell*> Visited;
    TQueue<AMazeCell*> Work;

    Visited.Add(Start);
    Work.Enqueue(Start);

    while (true)
    {
        AMazeCell* Current;
        Work.Dequeue(Current);

        if (VoronoidPoints.Contains(Current) || CloseVors.Num() > 0)
        {
            break;
        }
        else
        {
            TArray<AMazeCell*> CurrentNeighbours = Current->GetNeighbours();

            for (AMazeCell* Neighbour : CurrentNeighbours)
            {
                if (!Visited.Contains(Neighbour))
                {
                    Neighbour->SetHistory(Current->GetHistory());
                    Neighbour->AddHistory(Current);

                    if (VoronoidPoints.Contains(Neighbour))
                    {
                        CloseVors.Add(Neighbour);
                    }

                    Visited.Add(Neighbour);
                    Work.Enqueue(Neighbour);
                }
            }
        }
    }

    if (CloseVors.Num() == 1)
    {
        return CloseVors[0];
    }
    else
    {
        AMazeCell* ClosestVor = nullptr;
        TArray<AMazeCell*> Neighbours = Start->GetNeighbours();
        FColor MaxNeighboursColor = FColor::Black;

        if (Neighbours.Num() > 1)
        {
            TArray<int32> ColorsCount;
            TArray<FColor> Colors;

            for (AMazeCell* Neighbour : Neighbours)
            {
                if (Neighbour->Color == FColor::Black)
                {
                    continue;
                }

                if (!Colors.Contains(Neighbour->Color))
                {
                    Colors.Add(Neighbour->Color);
                    ColorsCount.Add(1);
                }
                else
                {
                    ColorsCount[Colors.IndexOfByKey(Neighbour->Color)]++;
                }
            }

            if (Colors.Num() != 0)
            {
                MaxNeighboursColor = Colors[ColorsCount.IndexOfByKey(FMath::Max(ColorsCount))];
            }
        }
        else
        {
            MaxNeighboursColor = Neighbours[0]->Color;
        }

        for (AMazeCell* CloseVor : CloseVors)
        {
            if (MaxNeighboursColor != FColor::Black)
            {
                if (MaxNeighboursColor == CloseVor->Color)
                {
                    ClosestVor = CloseVor;
                }
                else
                {
                    if (ClosestVor == nullptr)
                    {
                        ClosestVor = CloseVor;
                    }
                }
            }
            else
            {
                if (ClosestVor == nullptr)
                {
                    ClosestVor = CloseVor;
                }
                else if (FMath::RandRange(0, 2) == 0)
                {
                    ClosestVor = CloseVor;
                }
            }
        }
        return ClosestVor;
    }
}



