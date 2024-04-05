// Fill out your copyright notice in the Description page of Project Settings.


#include "FBESRunnable.h"
#include "FBESStruct.h"
#include "Commandlets/CommandletHelpers.h"

FBESRunnable::FBESRunnable(int InProcessIndex, int InRunProcessCount, FString InReportFilePath)
{
	ProcessIndex = InProcessIndex;
	RunProcessCount = InRunProcessCount;
	ReportFilePath = InReportFilePath;
}

FBESRunnable::FOnCompleteDelegate& FBESRunnable::GetCompleteDelegate()
{
	return OnCompleteDelegate;
}

FBESRunnable::FOnProgressDelegate& FBESRunnable::GetProgressDelegate()
{
	return OnProgressDelegate;
}

void FBESRunnable::HandlePipeMessage(const FString& InCommandletLogMessage)
{
	const FString MessageCode = TEXT("BPCP>");
	if (false == InCommandletLogMessage.Contains(MessageCode))
	{
		return;	
	}

	FString SplitLeft, Message;
	InCommandletLogMessage.Split(MessageCode, &SplitLeft, &Message);

	if (Message.IsEmpty())
	{
		UE_LOG(LogFBES, Error, TEXT("HandlePipeMessage: SplitRight is empty"));
		return;
	}

	TArray<FString> ParsedData;
	Message.ParseIntoArray(ParsedData, TEXT(","));
	if (ParsedData.Num() < 4)
	{
		UE_LOG(LogFBES, Error, TEXT("HandlePipeMessage: Parse.Num() < 4"));
		return;
	}

	FFBESBlueprintCompileProgressData Data;
	Data.PassCount = FCString::Atoi(*ParsedData[0]);
	Data.WarningCount = FCString::Atoi(*ParsedData[1]);
	Data.ErrorCount = FCString::Atoi(*ParsedData[2]);
	Data.AssetTotalCount = FCString::Atoi(*ParsedData[3]);
	Data.ProcessIndex = ProcessIndex;
	OnProgressDelegate.ExecuteIfBound(Data);
}

uint32 FBESRunnable::Run()
{
	UE_LOG(LogFBES, Log, TEXT("FBESRunnable::Run"));
	
	int ReturnCode = EFBESRunnableError::Type::Ok;
	void* ReadPipe = nullptr;
	void* WritePipe = nullptr;
	do
	{
		if (false == FPlatformProcess::CreatePipe(ReadPipe, WritePipe))
		{
			UE_LOG(LogFBES, Error, TEXT("CreatePipe failed"));
			ReturnCode = EFBESRunnableError::Type::CreatePipeFailed;
			break;
		}

		const FString ProjectFilePath = FString::Printf(TEXT("\"%s\""), *FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()));
		const FString AdditionalArgs = TEXT("-NoShaderCompile -ProcessIndex=") + FString::FromInt(ProcessIndex) + TEXT(" -TotalProcess=") + FString::FromInt(RunProcessCount) + TEXT(" -ReportFilePath=") + ReportFilePath;
		const FString ProcessArguments = CommandletHelpers::BuildCommandletProcessArguments(TEXT("FastBlueprintErrorScanner"), *ProjectFilePath, *AdditionalArgs);
		const FString ExecutableURL = FUnrealEdMisc::Get().GetExecutableForCommandlets();
		FProcHandle CommandletProcessHandle = FPlatformProcess::CreateProc(*ExecutableURL, *ProcessArguments, true, true, true, nullptr, 0, nullptr, WritePipe);
		if (false == CommandletProcessHandle.IsValid())
		{
			UE_LOG(LogFBES, Error, TEXT("CreateProc failed"));
			ReturnCode = EFBESRunnableError::Type::CreateProcFailed;
			break;
		}

		while (true)
		{
			const FString PipeString = FPlatformProcess::ReadPipe(ReadPipe);
			if (false == PipeString.IsEmpty())
			{
				HandlePipeMessage(PipeString);
			}

			if (false == FPlatformProcess::IsProcRunning(CommandletProcessHandle) && PipeString.IsEmpty())
			{
				UE_LOG(LogFBES, Log, TEXT("CommandletProcessHandle is not running"));
				break;
			}
			FPlatformProcess::Sleep(1.0f);
		}
		FPlatformProcess::CloseProc(CommandletProcessHandle);
		FPlatformProcess::GetProcReturnCode(CommandletProcessHandle, &ReturnCode);
	}
	while (false);

	FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
	OnCompleteDelegate.ExecuteIfBound(ReturnCode, ProcessIndex);
	return ReturnCode;
}

void FBESRunnable::Stop()
{
	UE_LOG(LogFBES, Log, TEXT("FBESRunnable::Stop"));
}