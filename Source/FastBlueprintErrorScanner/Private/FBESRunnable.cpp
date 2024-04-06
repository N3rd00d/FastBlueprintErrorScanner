// Fill out your copyright notice in the Description page of Project Settings.


#include "FBESRunnable.h"
#include "FBESStruct.h"
#include "Commandlets/CommandletHelpers.h"

FBESRunnable::FBESRunnable(int InProcessIndex, int InRunProcessCount, FString InReportFilePath)
{
	ProcessIndex = InProcessIndex;
	RunProcessCount = InRunProcessCount;
	ReportFilePath = InReportFilePath;

	const FString ThreadName = FString::Printf(TEXT("FBESRunnable_%d"), ProcessIndex);
	Thread = FRunnableThread::Create(this, *ThreadName);
}

FBESRunnable::~FBESRunnable()
{
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
		UE_LOG(LogFBES, Log, TEXT("Thread is killed"));
	}
}

FBESRunnable::FOnCompleteDelegate& FBESRunnable::GetCompleteDelegate()
{
	return OnCompleteDelegate;
}

FBESRunnable::FOnProgressDelegate& FBESRunnable::GetProgressDelegate()
{
	return OnProgressDelegate;
}

void FBESRunnable::ParseReadPipeMessage(const FString& InLogMessage)
{
	TArray<FString> Lines;
	InLogMessage.ParseIntoArrayLines(Lines);
	const int LineIndex = Lines.FindLastByPredicate([](FString const& Line)
	{
		return Line.Contains(FBESPipeCode);	
	});
	if (LineIndex == INDEX_NONE)
	{
		UE_LOG(LogFBES, Error, TEXT("Index is INDEX_NONE"));
		return;
	}

	FString SplitLeft, Message;
	Lines[LineIndex].Split(FBESPipeCode, &SplitLeft, &Message);

	if (Message.IsEmpty())
	{
		UE_LOG(LogFBES, Error, TEXT("Message is empty"));
		return;
	}

	TArray<FString> ParsedData;
	Message.ParseIntoArray(ParsedData, TEXT(","));
	if (ParsedData.Num() < 2)
	{
		UE_LOG(LogFBES, Error, TEXT("ParsedData.Num() < 2"));
		return;
	}

	FFBESBlueprintCompileProgressData Data;
	Data.PassCount = FCString::Atoi(*ParsedData[0]);
	Data.ErrorCount = FCString::Atoi(*ParsedData[1]);
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

		const FString LogFileName = FString::Printf(TEXT("FastBlueprintErrorScanner_%d.log"), ProcessIndex);
		const FString ProjectFilePath = FString::Printf(TEXT("\"%s\""), *FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()));
		const FString AdditionalArgs = TEXT("-NoShaderCompile -ProcessIndex=") + FString::FromInt(ProcessIndex) + TEXT(" -TotalProcess=") + FString::FromInt(RunProcessCount) + TEXT(" -ReportFilePath=") + ReportFilePath + TEXT(" -LOG=") + LogFileName;
		const FString ProcessArguments = CommandletHelpers::BuildCommandletProcessArguments(TEXT("FastBlueprintErrorScanner"), *ProjectFilePath, *AdditionalArgs);
		const FString ExecutableURL = GetExecutableForCommandlets();
		FProcHandle CommandletProcessHandle = FPlatformProcess::CreateProc(*ExecutableURL, *ProcessArguments, true, true, true, nullptr, 0, nullptr, WritePipe);
		if (false == CommandletProcessHandle.IsValid())
		{
			UE_LOG(LogFBES, Error, TEXT("CreateProc failed"));
			ReturnCode = EFBESRunnableError::Type::CreateProcFailed;
			break;
		}

		while (false == bStop)
		{
			const FString PipeString = FPlatformProcess::ReadPipe(ReadPipe);
			if (false == PipeString.IsEmpty())
			{
				ParseReadPipeMessage(PipeString);
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
	bStop = true;	
}

FString FBESRunnable::GetExecutableForCommandlets()
{
	FString ExecutableName;
#if WITH_EDITOR
	ExecutableName = FString(FPlatformProcess::ExecutablePath());
#if PLATFORM_WINDOWS
	if (ExecutableName.EndsWith(".exe", ESearchCase::IgnoreCase) && !FPaths::GetBaseFilename(ExecutableName).EndsWith("-cmd", ESearchCase::IgnoreCase))
	{
		FString NewExeName = ExecutableName.Left(ExecutableName.Len() - 4) + "-Cmd.exe";
		if (FPaths::FileExists(NewExeName))
		{
			ExecutableName = NewExeName;
		}
	}
#elif PLATFORM_MAC
	if (false == FPaths::GetBaseFilename(ExecutableName).EndsWith("-cmd", ESearchCase::IgnoreCase))
	{
		FString NewExeName = ExecutableName + "-Cmd";
		if (FPaths::FileExists(NewExeName))
		{
			ExecutableName = NewExeName;
		}
	}
#endif
#endif
	return ExecutableName;
}