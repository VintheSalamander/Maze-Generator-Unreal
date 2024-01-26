// Fill out your copyright notice in the Description page of Project Settings.


#include "MazeGenerator.h"
#include "PathSearch.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
float AMazeGenerator::ElevationRatio;
int AMazeGenerator::CellSize;

// Sets default values
AMazeGenerator::AMazeGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    Root = RootComponent;
}

// Called when the game starts or when spawned
void AMazeGenerator::BeginPlay()
{
	Super::BeginPlay();

	//Validate default values and set them
    if (ElevationRatioIn <= 0.f) {
        UE_LOG(LogTemp, Error, TEXT("Invalid ElevationRatio: %f, ElevationRatio was set to 8.f"), ElevationRatioIn);
        ElevationRatio = 8.f;
    }
    else
    {
        ElevationRatio = ElevationRatioIn;
    }
    if (CellSizeIn <= 0) {
        UE_LOG(LogTemp, Error, TEXT("Invalid CellSize: %d, CellSize was set to 10"), CellSizeIn);
        CellSize = 10;
    }
    else
    {
        CellSize = CellSizeIn;
    }
    if (MazeWidth <= 0) {
        UE_LOG(LogTemp, Error, TEXT("Invalid MazeWidth: %d, MazeWidth was set to 5"), MazeWidth);
        MazeWidth = 5;
    }
    if (MazeDepth <= 0) {
        UE_LOG(LogTemp, Error, TEXT("Invalid MazeDepth: %d, MazeDepth was set to 5"), MazeDepth);
        MazeDepth = 5;
    }
    if (VoronoidCellSize <= 0) {
        UE_LOG(LogTemp, Error, TEXT("Invalid VoronoidCellSize: %d, VoronoidCellSize was set to 1"), VoronoidCellSize);
        VoronoidCellSize = 1;
    }
    if (MazeWidth % VoronoidCellSize != 0 || MazeDepth % VoronoidCellSize != 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid VoronoidCellSize: %d, VoronoidCellSize was set to 1"), VoronoidCellSize);
        UE_LOG(LogTemp, Error, TEXT("VoronoidCellSize must be divisible by maze width and depth"));
        VoronoidCellSize = 1;
    }
    if (PossibleColors.Num() == 0) {
        UE_LOG(LogTemp, Error, TEXT("Invalid number of Possible Colors: 0, Black was added to the array"));
        PossibleColors.Add(FColor::Black);
    }

	MazeGrid.Init(nullptr, MazeWidth * MazeDepth);
	VoronoidGridWidth = MazeWidth / VoronoidCellSize;
    VoronoidGridDepth = MazeDepth / VoronoidCellSize;
	VoronoidGrid.Init(nullptr, VoronoidGridWidth * VoronoidGridDepth);

	GenerateMaze();

	// Randomly select a start position
	int32 StartX = FMath::RandRange(0, MazeWidth - 1);
	// Set the start cell
	StartCell = MazeGrid[StartX];
	if (StartCell)
	{   
		StartCell->SetElevation(EElevation::None);
		StartCell->GenerateMesh(0.f, EDirection::Right, StartCell->GetCellVerts(), CellSize);
	}

    // Move the player
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        APawn* PlayerPawn = PlayerController->GetPawn();
        ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn);
        if (PlayerCharacter)
        {
            PlayerCharacter->SetActorLocation(FVector((StartX + 0.5f) * CellSize, CellSize / 2, 15.f));
        }
    }
	SetExitAndKey();
    SetColorVoronoid();
}

//Generates Maze using Ellers algorithm for maze generation
void AMazeGenerator::GenerateMaze()
{
    for (int32 Y = 0; Y < MazeDepth; Y++)
    {
        for (int32 X = 0; X < MazeWidth; X++)
        {
            int32 CellIndex = Y * MazeWidth + X;
            if (!ensure(MazeGrid.IsValidIndex(CellIndex)))
            {
                UE_LOG(LogTemp, Error, TEXT("Invalid CellIndex: %d"), CellIndex);
                continue;
            }
            //Instantiates the Prefabs and set for each cell in the row
            if (MazeGrid.IsValidIndex(CellIndex) && MazeGrid[CellIndex] == nullptr)
            {
                FVector Location(X * CellSize, Y * CellSize, 0.f);
                if (BPMazeCell) {
                    AMazeCell* NewCell = GetWorld()->SpawnActor<AMazeCell>(BPMazeCell, Location, FRotator::ZeroRotator);
                    NewCell->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
                    MazeGrid[CellIndex] = NewCell;
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Invalid BPMazeCell"));
                }
                CellSets.Add(CellIndex, { CellIndex });
            }

            //There is a probabily of joining different cells in different sets
            //or if its the last row join the different cells in different sets to create perfect maze
            if (X != 0 && (FMath::RandRange(0.f, 1.f) < EllersMergeProb || Y == MazeDepth - 1))
            {
                int32 CurrentSet = FindSet(CellIndex);
                int32 RightSet = FindSet(CellIndex - 1);
                if (CurrentSet != RightSet)
                {
                    AMazeCell* CurrentCell = MazeGrid[CellIndex];
                    AMazeCell* RightCell = MazeGrid[CellIndex - 1];

                    CurrentCell->BreakRightWall();
                    RightCell->BreakLeftWall();
                    CurrentCell->AddNeighbour(RightCell);
					RightCell->AddNeighbour(CurrentCell);

                    MergeSets(CurrentSet, RightSet);
                }
            }
        }

        if (Y == MazeDepth - 1)
        {
            continue;
        }

        //For each set remaining join one random cell with the above row
        for (const TPair<int32, TArray<int32>>& Set : CellSets)
        {
            int32 CurrentCellIndex = Set.Value[FMath::RandRange(0, Set.Value.Num() - 1)];
            int32 AboveCellIndex = CurrentCellIndex + MazeWidth;
            AMazeCell* CurrentCell = MazeGrid[CurrentCellIndex];

            if (MazeGrid.IsValidIndex(AboveCellIndex) && MazeGrid[AboveCellIndex] == nullptr)
            {
                FVector Location(AboveCellIndex % MazeWidth * CellSize, AboveCellIndex / MazeWidth * CellSize, 0.f);
                if (BPMazeCell) {
                    AMazeCell* NewCell = GetWorld()->SpawnActor<AMazeCell>(BPMazeCell, Location, FRotator::ZeroRotator);
                    NewCell->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
                    MazeGrid[AboveCellIndex] = NewCell;
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Invalid BPMazeCell"));
                }
            }

            AMazeCell* AboveCell = MazeGrid[AboveCellIndex];

            CurrentCell->BreakTopWall();
            AboveCell->BreakBottomWall();
            CurrentCell->AddNeighbour(AboveCell);
            AboveCell->AddNeighbour(CurrentCell);

            CellSets[Set.Key].Empty();
            CellSets[Set.Key].Add(AboveCellIndex);
        }

        
    }
}

//Functions used for the maze generation
int32 AMazeGenerator::FindSet(int32 CellIndex)
{
	for (const TPair<int32, TArray<int32>>& Entry : CellSets)
	{
		if (Entry.Value.Contains(CellIndex))
		{
			return Entry.Key;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("FindSet() Error with cell at index: %d"), CellIndex);
	return -1;
}

void AMazeGenerator::MergeSets(int32 SetFrom, int32 SetTo)
{
	for (int32 CellIndex : CellSets[SetFrom])
	{
		CellSets[SetTo].Add(CellIndex);
	}

	CellSets.Remove(SetFrom);
}

//Determines nextCells Elevation and generates its mesh
//To generate some sense of terrain there is a 1/4 of probability staying the same elevation as the currentCell, 
//if elevation is None then it changes slightly, if its already slightly
//is has a probality of 1/3 to change back to None, if not it will change drastically
//and if its already changed drastically it will go to a slight change
void AMazeGenerator::GenerateCellMesh(AMazeCell* CurrentCell, AMazeCell* NextCell)
{
    float Elevation;
    EElevation CurrentElev = CurrentCell->GetElevation();

    if (FMath::FRand() > 0.25f) {
        switch (CurrentElev)
        {
        case EElevation::MinusMax:
            NextCell->SetElevation(EElevation::MinusMin);
            break;
        case EElevation::MinusMin:
            if (FMath::FRand() < 1.f / 3.f)
            {
                NextCell->SetElevation(EElevation::None);
            }
            else
            {
                NextCell->SetElevation(EElevation::MinusMax);
                
            }
            break;
        case EElevation::None:
            if (FMath::FRand() < 0.5f)
            {
                NextCell->SetElevation(EElevation::PlusMin);
                
            }
            else
            {
                NextCell->SetElevation(EElevation::MinusMin);
            }
            break;
        case EElevation::PlusMin:
            if (FMath::FRand() < 1.f / 3.f)
            {
                NextCell->SetElevation(EElevation::None);
            }
            else
            {
                NextCell->SetElevation(EElevation::PlusMax);
                
            }
            break;
        case EElevation::PlusMax:
            NextCell->SetElevation(EElevation::PlusMin);
            break;
        default:
            Elevation = 0.f;
            NextCell->SetElevation(CurrentElev);
            UE_LOG(LogTemp, Error, TEXT("Error in GenerateCellMesh() switch CurrentElev"));
            break;
        }
    }
    else
    {
        NextCell->SetElevation(CurrentElev);
    }

    switch (NextCell->GetElevation())
    {
        case EElevation::PlusMax:
            Elevation = 75.f + FMath::RandRange(-15.f, 10.f); 
            break;
        case EElevation::PlusMin:
            Elevation = 25.f + FMath::RandRange(-10.f, 30.f);
            break;
        case EElevation::None:
            Elevation = 0.f;
            break;
        case EElevation::MinusMin:
            Elevation = -25.f + FMath::RandRange(-30.f, 10.f);
            break;
        case EElevation::MinusMax:
            Elevation = -75.f + FMath::RandRange(-10.f, 15.f);
            break;
        default:
            Elevation = 0.f;
            UE_LOG(LogTemp, Error, TEXT("Error in GenerateCellMesh() switch Next Elevation"));
            break;
    }

    FVector NewPosition = NextCell->GetActorLocation();
    NewPosition.Z += CurrentCell->GetActorLocation().Z + Elevation / ElevationRatio;
    NextCell->SetActorLocation(NewPosition);
    NextCell->GenerateMesh(-Elevation / ElevationRatio, GetDirection(CurrentCell, NextCell), CurrentCell->GetCellVerts(), CellSize);
}

EDirection AMazeGenerator::GetDirection(const AMazeCell* CurrentCell, const AMazeCell* NextCell)
{
    FVector CurrentPosition = CurrentCell->GetActorLocation();
    FVector NextPosition = NextCell->GetActorLocation();
    FVector Difference = NextPosition - CurrentPosition;

    if (FMath::Abs(Difference.X) > FMath::Abs(Difference.Y))
    {
        return Difference.X > 0 ? EDirection::Left : EDirection::Right;
    }
    else
    {
        return Difference.Y > 0 ? EDirection::Top : EDirection::Bottom;
    }
}

void AMazeGenerator::SetExitAndKey()
{
    
    TPair<AMazeCell*, AMazeCell*> EndAndKey = PathSearch::GetExitAndKey(MazeGrid, StartCell);
    AMazeCell* ExitCell = EndAndKey.Key;
    AMazeCell* KeyCell = EndAndKey.Value;
    if (ExitCell && KeyCell) {
        if (Exit && Key) {
            Exit->SetActorLocation(FVector(ExitCell->GetActorLocation().X + CellSize / 2, ExitCell->GetActorLocation().Y + CellSize / 2, ExitCell->GetActorLocation().Z + CellSize / 2));
            Key->SetActorLocation(FVector(KeyCell->GetActorLocation().X + CellSize / 2, KeyCell->GetActorLocation().Y + CellSize / 2, KeyCell->GetActorLocation().Z + CellSize / 2));
        }
        else {
            UE_LOG(LogTemp, Error, TEXT("Error in SetExitAndKey Exit and Key"));
        }
    }
    else 
    {
        if (!ExitCell) {
            UE_LOG(LogTemp, Error, TEXT("End Cell: %f, %f"), ExitCell->GetActorLocation().X, ExitCell->GetActorLocation().Y);
        }
        if (!KeyCell) {
            UE_LOG(LogTemp, Error, TEXT("Key Cell: %f, %f"), KeyCell->GetActorLocation().X, KeyCell->GetActorLocation().Y);
        }
        UE_LOG(LogTemp, Error, TEXT("Error in SetExitAndKey()"));
    }
}

//A voronoid grid is used to create areas with different colors in the maze
void  AMazeGenerator::SetColorVoronoid()
{
    TSet<AMazeCell*> voronoidPoints;
    for (int I = 0; I < VoronoidGridDepth; I++)
    {
        for (int J = 0; J < VoronoidGridWidth; J++)
        {
            int Index = FMath::RandRange(I * VoronoidCellSize, (I + 1) * VoronoidCellSize - 1) * MazeWidth + FMath::RandRange(J * VoronoidCellSize, (J + 1) * VoronoidCellSize - 1);
            int VoronoiIndex = I * VoronoidGridWidth + J;
            if (MazeGrid.IsValidIndex(Index)) {
                VoronoidGrid[VoronoiIndex] = MazeGrid[Index];
                VoronoidGrid[VoronoiIndex]->SetWallsColor(PossibleColors[FMath::RandRange(0, PossibleColors.Num() - 1)]);
                voronoidPoints.Add(VoronoidGrid[VoronoiIndex]);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("SetColorVoronoid MazeIndex Out of Bounds %d"), Index);
            }
        }
    }

    int XExit = FMath::RoundToInt(Exit->GetActorLocation().X - CellSize / 2) / CellSize;
    int YExit = FMath::RoundToInt(Exit->GetActorLocation().Y - CellSize / 2) / CellSize;
    int XKey = FMath::RoundToInt(Key->GetActorLocation().X - CellSize/2) / CellSize;
    int YKey = FMath::RoundToInt(Key->GetActorLocation().Y - CellSize / 2) / CellSize;
    AMazeCell* exitCell = MazeGrid[YExit * MazeWidth + XExit];
    AMazeCell* keyCell = MazeGrid[YKey * MazeWidth + XKey];
    exitCell->SetWallsColor(AreaEVA);
    keyCell->SetWallsColor(AreaPlant);
    PathSearch::GetClosestVoronoid(MazeGrid, exitCell, voronoidPoints)->SetWallsColor(AreaEVA);
    PathSearch::GetClosestVoronoid(MazeGrid, keyCell, voronoidPoints)->SetWallsColor(AreaPlant);

    for (int Y = 0; Y < MazeDepth; Y++)
    {
        for (int X = 0; X < MazeWidth; X++)
        {
            if ((X == XExit && Y == YExit) || (X == XKey && Y == YKey))
            {
                continue;
            }
            int32 VoronoidIndex = Y * VoronoidGridWidth + X;
            int32 MazeIndex = Y * MazeWidth + X;
            if (voronoidPoints.Contains(MazeGrid[Y * MazeWidth + X]))
            {
                continue;
            }
            AMazeCell* NearestVor = PathSearch::GetClosestVoronoid(MazeGrid, MazeGrid[MazeIndex], voronoidPoints);
            MazeGrid[MazeIndex]->SetWallsColor(NearestVor->Color);
        }
    }
}


AMazeCell* AMazeGenerator::GetCellInDirection(AMazeCell* Cell, EDirection Direction)
{
    int32 X = Cell->GetActorLocation().X / CellSize;
    int32 Y = Cell->GetActorLocation().Y / CellSize;

    switch (Direction)
    {
    case EDirection::Left:
        X++;
        break;
    case EDirection::Right:
        X--;
        break;
    case EDirection::Bottom:
        Y--;
        break;
    case EDirection::Top:
        Y++;
        break;
    }
    int32 CellIndex = Y * MazeWidth + X;
    if (MazeGrid.IsValidIndex(CellIndex)) {
        return MazeGrid[CellIndex];
    }
    else {
        return nullptr;
    }
}




