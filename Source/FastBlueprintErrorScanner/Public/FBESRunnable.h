// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FBESStruct.h"

class FASTBLUEPRINTERRORSCANNER_API FBESRunnable : public FRunnable
{
public:
    FBESRunnable(int InProcessIndex, int InRunProcessCount, FString InReportFilePath);

	DECLARE_DELEGATE_TwoParams(FOnCompleteDelegate, int, int);
	FOnCompleteDelegate& GetCompleteDelegate();
	DECLARE_DELEGATE_OneParam(FOnProgressDelegate, FFBESBlueprintCompileProgressData const&);
    FOnProgressDelegate& GetProgressDelegate();

    void HandlePipeMessage(const FString& InCommandletLogMessage);
	virtual uint32 Run() override;
    virtual void Stop() override;

protected:
    int ProcessIndex = -1;
    int RunProcessCount = 0;
    FString ReportFilePath;
	FOnCompleteDelegate OnCompleteDelegate;
	FOnProgressDelegate OnProgressDelegate;
};