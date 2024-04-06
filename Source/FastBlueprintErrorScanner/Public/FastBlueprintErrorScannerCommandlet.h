// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "AssetRegistry/AssetData.h"
#include "FBESStruct.h"
#include "FastBlueprintErrorScannerCommandlet.generated.h"

class IKismetCompilerInterface;

DECLARE_LOG_CATEGORY_EXTERN(LogFBESCmd, Log, All);

UCLASS()
class FASTBLUEPRINTERRORSCANNER_API UFastBlueprintErrorScannerCommandlet : public UCommandlet
{
	GENERATED_UCLASS_BODY()
	virtual int32 Main(const FString& Params) override;

	void ParseArguments(const FString& InParams);
	int GetBlueprintCompileErrorCount(UBlueprint* InBlueprint);
	void WritePipeMessageAtInterval2s(TArray<FFBESCompileResult>& RefCompileResults, FDateTime& RefTime);
	void WritePipeMessage(TArray<FFBESCompileResult>& RefCompileResults);

	int ProcessIndex = -1;
	int TotalProcess = 0;
	int TotalAssetCount = 0;
	FString ReportFilePath;
	IKismetCompilerInterface* KismetBlueprintCompilerModule;
};
