#pragma once

#include "CoreMinimal.h"
#include "GameEnums.generated.h"

UENUM(BlueprintType)
enum class EDirection : uint8
{
    Left UMETA(DisplayName = "Left"),
    Right UMETA(DisplayName = "Right"),
    Bottom UMETA(DisplayName = "Bottom"),
    Top UMETA(DisplayName = "Top")
};

UENUM(BlueprintType)
enum class EElevation : uint8
{
	MinusMax   UMETA(DisplayName = "-75"),
	MinusMin   UMETA(DisplayName = "-25"),
	None       UMETA(DisplayName = "0"),
	PlusMin    UMETA(DisplayName = "25"),
	PlusMax    UMETA(DisplayName = "75")
};

UENUM(BlueprintType)
enum class EVert : uint8
{
	LeftBot UMETA(DisplayName = "Left Bottom"),
	RightBot UMETA(DisplayName = "Right Bottom"),
	LeftTop UMETA(DisplayName = "Left Top"),
	RightTop UMETA(DisplayName = "Right Top")
};
