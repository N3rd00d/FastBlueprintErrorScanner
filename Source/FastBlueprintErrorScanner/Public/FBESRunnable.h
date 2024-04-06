// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FBESStruct.h"

class FASTBLUEPRINTERRORSCANNER_API FBESRunnable : public FRunnable
{
public:
	FBESRunnable(int InProcessIndex, int InRunProcessCount, FString InReportFilePath);
	~FBESRunnable();

	DECLARE_DELEGATE_TwoParams(FOnCompleteDelegate, int, int);
	FOnCompleteDelegate& GetCompleteDelegate();
	DECLARE_DELEGATE_OneParam(FOnProgressDelegate, FFBESBlueprintCompileProgressData const&);
	FOnProgressDelegate& GetProgressDelegate();

	void ParseReadPipeMessage(const FString& InLogMessage);
	virtual uint32 Run() override;
	virtual void Stop() override;
	FString GetExecutableForCommandlets();

protected:
	FOnCompleteDelegate OnCompleteDelegate;
	FOnProgressDelegate OnProgressDelegate;
	
	int ProcessIndex = -1;
	int RunProcessCount = 0;
	FString ReportFilePath;
	bool bStop = false;
	FRunnableThread* Thread = nullptr;
};
