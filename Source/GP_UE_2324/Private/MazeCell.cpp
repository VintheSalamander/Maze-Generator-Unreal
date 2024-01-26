// Fill out your copyright notice in the Description page of Project Settings.


#include "MazeCell.h"

// Sets default values
AMazeCell::AMazeCell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Create a default root component if not already present
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Initialize walls and procedural floor of the MazeCell
	LeftWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWall"));
	LeftWall->SetupAttachment(RootComponent);
	RightWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightWall"));
	RightWall->SetupAttachment(RootComponent);
	BottomWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottomWall"));
	BottomWall->SetupAttachment(RootComponent);
	TopWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopWall"));
	TopWall->SetupAttachment(RootComponent);

	Floor = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Floor"));
	Floor->SetupAttachment(RootComponent);

	// Initialize other defaults
	IsVisited = false;
	Color = FColor::Black;
	CellElevation = EElevation::None;

	//Part of the procedural mesh with 4 vertices and a quad made of two triangles
	CellVerts.SetNum(4);
}

// Called when the game starts or when spawned
void AMazeCell::BeginPlay()
{
	Super::BeginPlay();
	
}

//Functions for hiding walls when connecting the maze
void AMazeCell::BreakLeftWall()
{
	if (LeftWall)
	{
		LeftWall->SetVisibility(false);
		LeftWall->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
void AMazeCell::BreakRightWall()
{
	if (RightWall)
	{
		RightWall->SetVisibility(false);
		RightWall->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
void AMazeCell::BreakTopWall()
{
	if (TopWall)
	{
		TopWall->SetVisibility(false);
		TopWall->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
void AMazeCell::BreakBottomWall()
{
	if (BottomWall)
	{
		BottomWall->SetVisibility(false);
		BottomWall->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

//Call when cell is visited
void AMazeCell::Visit()
{
	IsVisited = true;
}

//Calculates the vertices of the floor for the mesh, to align with the previous MazeCell and 
//according to the elevation of the cell
void AMazeCell::GenerateMesh(float Elevation, EDirection Direction, const TArray<FVector>& PrevCellVerts, float CellSize)
{
	switch (Direction)
	{
	case EDirection::Right:
		SetVert(EVert::LeftBot, FVector(CellSize, 0.f, Elevation + PrevCellVerts[(int32)EVert::RightBot].Z));
		SetVert(EVert::RightBot, FVector(0.f, 0.f, 0.f));
		SetVert(EVert::LeftTop, FVector(CellSize, CellSize, Elevation + PrevCellVerts[(int32)EVert::RightTop].Z));
		SetVert(EVert::RightTop, FVector(0.f, CellSize, 0.f));
		break;
	case EDirection::Left:
		SetVert(EVert::LeftBot, FVector(CellSize, 0.f, 0.f));
		SetVert(EVert::RightBot, FVector(0.f, 0.f, Elevation + PrevCellVerts[(int32)EVert::LeftBot].Z));
		SetVert(EVert::LeftTop, FVector(CellSize, CellSize, 0.f));
		SetVert(EVert::RightTop, FVector(0.f, CellSize, Elevation + PrevCellVerts[(int32)EVert::LeftTop].Z));
		break;
	case EDirection::Bottom:
		SetVert(EVert::LeftBot, FVector(CellSize, 0.f, 0.f));
		SetVert(EVert::RightBot, FVector(0.f, 0.f, 0.f));
		SetVert(EVert::LeftTop, FVector(CellSize, CellSize, Elevation + PrevCellVerts[(int32)EVert::LeftBot].Z));
		SetVert(EVert::RightTop, FVector(0.f, CellSize, Elevation + PrevCellVerts[(int32)EVert::RightBot].Z));
		break;
	case EDirection::Top:
		SetVert(EVert::LeftBot, FVector(CellSize, 0.f, Elevation + PrevCellVerts[(int32)EVert::LeftTop].Z));
		SetVert(EVert::RightBot, FVector(0.f, 0.f, Elevation + PrevCellVerts[(int32)EVert::RightTop].Z));
		SetVert(EVert::LeftTop, FVector(CellSize, CellSize, 0.f));
		SetVert(EVert::RightTop, FVector(0.f, CellSize, 0.f));
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("Error in GenerateMesh()"));
		break;
	}

	//Set Triangles
	CellTris = { (int32)EVert::LeftTop, (int32)EVert::LeftBot, (int32)EVert::RightTop, (int32)EVert::RightTop, (int32)EVert::LeftBot, (int32)EVert::RightBot };

	//Set all vertex colors to white
	TArray<FLinearColor> VertexColors;
	VertexColors.Init(FLinearColor::Gray, CellVerts.Num());

	Floor->ClearMeshSection(0);
	// Create the mesh section, true for collission
	Floor->CreateMeshSection_LinearColor(0, CellVerts, CellTris, TArray<FVector>(), TArray<FVector2D>(), VertexColors, TArray<FProcMeshTangent>(), true);
	//Set Material
	if (FloorMaterial)
	{
		Floor->SetMaterial(0, FloorMaterial);
	}
}

const TArray<FVector>& AMazeCell::GetCellVerts()
{
	return CellVerts;
}

void AMazeCell::SetElevation(EElevation NewElevation)
{
	CellElevation = NewElevation;
}

const EElevation AMazeCell::GetElevation() {
	return CellElevation;
}

void AMazeCell::SetVert(EVert Vert, FVector Pos)
{
	CellVerts[(int32)Vert] = Pos;
}

void AMazeCell::SetWallsColor(const FColor& NewColor)
{
	// Helper function to set color of a wall
	auto SetWallColor = [NewColor](UStaticMeshComponent* Wall)
		{
			if (Wall)
			{
				UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Wall->GetMaterial(0));
				if (!DynamicMaterial)
				{
					// If the material is not a dynamic instance, create one
					UMaterialInterface* Material = Wall->GetMaterial(0);
					DynamicMaterial = UMaterialInstanceDynamic::Create(Material, Wall);
					Wall->SetMaterial(0, DynamicMaterial);
				}
				if (DynamicMaterial)
				{
					DynamicMaterial->SetVectorParameterValue(TEXT("Color"), NewColor);
					DynamicMaterial->PostEditChange();
				}
			}
		};

	// Set color for each wall
	SetWallColor(LeftWall);
	SetWallColor(RightWall);
	SetWallColor(BottomWall);
	SetWallColor(TopWall);

	Color = NewColor;
}

//Functions for the history of the MazeCell which is a path to this MazeCell from the first cell on the list
void AMazeCell::SetHistory(const TArray<AMazeCell*>& NewHistory)
{
	History = NewHistory;
}

const TArray<AMazeCell*>& AMazeCell::GetHistory()
{
	return History;
}

const int32 AMazeCell::GetHistoryCount()
{
	return History.Num();
}

void AMazeCell::AddHistory(AMazeCell* Addition)
{
	if (Addition)
	{
		History.Add(Addition);
	}
}

//Functions for the neighbours of the MazeCell, adjacent reachable cells
const TArray<AMazeCell*>& AMazeCell::GetNeighbours()
{
	return Neighbours;
}

void AMazeCell::AddNeighbour(AMazeCell* Neighbour)
{
	if (Neighbour)
	{
		Neighbours.Add(Neighbour);
	}
}

EDirection AMazeCell::GetOpenDirection()
{
    if (TopWall && !TopWall->IsVisible())
    {
        return EDirection::Top;
    }
    else if (BottomWall && !BottomWall->IsVisible())
    {
        return EDirection::Bottom;
    }
    else if (LeftWall && !LeftWall->IsVisible())
    {
        return EDirection::Left;
    }
    else
    {
        return EDirection::Right;
    }
}


