// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FBESStruct.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFBES, Log, All);

UENUM()
namespace EFBESCompileResultState
{
	enum Type
	{
		Pass,
		Warning,
		Error
	};
}

USTRUCT()
struct FFBESCompileResult
{
	GENERATED_BODY()

	UPROPERTY()
	FString AssetPath;
	UPROPERTY()
	int State = 0;
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
	int WarningCount = 0;
	UPROPERTY()
	int ErrorCount = 0;
	UPROPERTY()
	int ProcessIndex = 0;
	UPROPERTY()
	int AssetTotalCount = 0;
};