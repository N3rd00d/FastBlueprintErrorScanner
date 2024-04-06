// Fill out your copyright notice in the Description page of Project Settings.


#include "FastBlueprintErrorScannerCommandlet.h"

#include <fstream>
#include "JsonObjectConverter.h"
#include "KismetCompilerModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Kismet2/KismetEditorUtilities.h"

DEFINE_LOG_CATEGORY(LogFBESCmd)

UFastBlueprintErrorScannerCommandlet::UFastBlueprintErrorScannerCommandlet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

int32 UFastBlueprintErrorScannerCommandlet::Main(const FString& Params)
{
	UE_LOG(LogFBESCmd, Log, TEXT("FastBlueprintErrorScannerCommandlet::Main"));
	ParseArguments(Params);

	KismetBlueprintCompilerModule = &FModuleManager::LoadModuleChecked<IKismetCompilerInterface>(TEXT("KismetCompiler"));
	TArray<FFBESCompileResult> CompileResults;

	TArray<FAssetData> BlueprintAssetList;
	TArray<FAssetData> WorldAssetList;
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetRegistryConstants::ModuleName);
	AssetRegistryModule.Get().SearchAllAssets(true);
	AssetRegistryModule.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetFName(), BlueprintAssetList, true);
	AssetRegistryModule.Get().GetAssetsByClass(UWorld::StaticClass()->GetFName(), WorldAssetList, true);
	UE_LOG(LogFBESCmd, Log, TEXT("BlueprintAssetList : %d, WorldAssetList : %d"), BlueprintAssetList.Num(), WorldAssetList.Num());

	FDateTime Time = FDateTime::Now();
	for (FAssetData const& Asset : BlueprintAssetList)
	{
		uint32 Hash = TextKeyUtil::HashString(Asset.ObjectPath.ToString());
		if (Hash % TotalProcess != ProcessIndex)
		{
			continue;
		}
		FString const AssetPath = Asset.ObjectPath.ToString();
		UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(Asset.GetClass(), nullptr, *AssetPath, nullptr, LOAD_NoWarn | LOAD_DisableCompileOnLoad));
		FFBESCompileResult CompileResult{Blueprint->GetPathName(), 0};
		if (IsValid(Blueprint))
		{
			CompileResult.NumErrors = GetBlueprintCompileErrorCount(Blueprint);
		}
		CompileResults.Emplace(MoveTemp(CompileResult));
		WritePipeMessageAtInterval2s(CompileResults, Time);
	}
	UE_LOG(LogFBESCmd, Log, TEXT("Blueprint CompileResults : %d"), CompileResults.Num());

	for (FAssetData const& Asset : WorldAssetList)
	{
		uint32 Hash = TextKeyUtil::HashString(Asset.ObjectPath.ToString());
		if (Hash % TotalProcess != ProcessIndex)
		{
			continue;
		}
		FString const AssetPath = Asset.ObjectPath.ToString();
		UObject* Object = StaticLoadObject(Asset.GetClass(), nullptr, *AssetPath, nullptr, LOAD_NoWarn | LOAD_DisableCompileOnLoad);
		UWorld* World = Cast<UWorld>(Object);
		FFBESCompileResult CompileResult{World->GetPathName(), 0};
		if (IsValid(World) && IsValid(World->PersistentLevel))
		{
			for (UBlueprint* Blueprint : World->PersistentLevel->GetLevelBlueprints())
			{
				if (IsValid(Blueprint))
				{
					CompileResult.NumErrors = GetBlueprintCompileErrorCount(Blueprint);
					if (CompileResult.NumErrors > 0)
					{
						break;
					}
				}
			}
		}
		CompileResults.Emplace(MoveTemp(CompileResult));
		WritePipeMessageAtInterval2s(CompileResults, Time);
	}
	WritePipeMessage(CompileResults);
	UE_LOG(LogFBESCmd, Log, TEXT("World CompileResults : %d"), CompileResults.Num());
	FPlatformProcess::Sleep(1.0);

	FFBESCompileResultJsonFormat Format;
	Format.Data = MoveTemp(CompileResults);
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(Format, JsonString);
	std::ofstream OutFile(TCHAR_TO_UTF8(*ReportFilePath));
	OutFile << TCHAR_TO_UTF8(*JsonString);
	OutFile.close();

	UE_LOG(LogFBESCmd, Log, TEXT("Commandlet Done"));
	return 0;
}

void UFastBlueprintErrorScannerCommandlet::ParseArguments(const FString& InParams)
{
	UE_LOG(LogFBESCmd, Log, TEXT("InitCommandLine : %s"), *InParams);

	ProcessIndex = -1;
	TotalProcess = 0;
	ReportFilePath.Empty();

	FString RefProcessIndex;
	FString RefTotalProcess;

	if (FParse::Value(*InParams, TEXT("-ProcessIndex="), RefProcessIndex))
	{
		ProcessIndex = FCString::Atoi(*RefProcessIndex);
	}
	if (FParse::Value(*InParams, TEXT("-TotalProcess="), RefTotalProcess))
	{
		TotalProcess = FCString::Atoi(*RefTotalProcess);
	}
	FParse::Value(*InParams, TEXT("-ReportFilePath="), ReportFilePath);

	UE_LOG(LogFBESCmd, Log, TEXT("ProcessIndex : %d, TotalProcess : %d, ReportFilePath : %s"), ProcessIndex, TotalProcess, *ReportFilePath);
}

int UFastBlueprintErrorScannerCommandlet::GetBlueprintCompileErrorCount(UBlueprint* InBlueprint)
{
	FCompilerResultsLog MessageLog;
	MessageLog.bSilentMode = true;
	MessageLog.SetSourcePath(InBlueprint->GetPathName());
	MessageLog.BeginEvent(TEXT("Compile"));
	FKismetEditorUtilities::CompileBlueprint(InBlueprint, EBlueprintCompileOptions::SkipSave | EBlueprintCompileOptions::BatchCompile, &MessageLog);
	MessageLog.EndEvent();

	return MessageLog.NumErrors;
}

void UFastBlueprintErrorScannerCommandlet::WritePipeMessageAtInterval2s(TArray<FFBESCompileResult>& RefCompileResults, FDateTime& RefTime)
{
	if (FDateTime::Now() - RefTime > FTimespan(0, 0, 2))
	{
		RefTime = FDateTime::Now();
		WritePipeMessage(RefCompileResults);
	}
}

void UFastBlueprintErrorScannerCommandlet::WritePipeMessage(TArray<FFBESCompileResult>& RefCompileResults)
{
	int PassCount = 0;
	int ErrorCount = 0;
	for (FFBESCompileResult const& Result : RefCompileResults)
	{
		if (Result.NumErrors == 0)
		{
			PassCount++;
		}
		else
		{
			ErrorCount++;
		}
	}

	UE_LOG(LogFBESCmd, Display, TEXT("%s%d,%d"), *FBESPipeCode, PassCount, ErrorCount);
}
