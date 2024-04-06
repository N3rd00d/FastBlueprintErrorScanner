
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FBESStruct.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFBES, Log, All);

USTRUCT()
struct FFBESCompileResult
{
	GENERATED_BODY()

	UPROPERTY()
	FString AssetPath;
	UPROPERTY()
	int NumErrors = 0;
};

USTRUCT()
struct FFBESCompileResultJsonFormat
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FFBESCompileResult> Data;
};

UENUM()
namespace EFBESRunnableError
{
	enum Type
	{
		Ok,
		CreatePipeFailed,
		CreateProcFailed
	};
}

USTRUCT()
struct FFBESBlueprintCompileProgressData
{
	GENERATED_BODY()

	UPROPERTY()
	int PassCount = 0;
	UPROPERTY()
	int ErrorCount = 0;
	UPROPERTY()
	int ProcessIndex = 0;
	UPROPERTY()
	int TotalCount = 0;
};

const FString FBESPipeCode = TEXT("BPCR>");