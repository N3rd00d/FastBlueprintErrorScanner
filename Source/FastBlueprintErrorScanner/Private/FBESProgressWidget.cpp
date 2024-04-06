#include "FBESProgressWidget.h"

#include "FBESRunnable.h"
#include "JsonObjectConverter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Async/Async.h"
#include "Components/Button.h"
#include "Components/CircularThrobber.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Misc/FileHelper.h"

FString GetReportDirectory()
{
	return FPaths::ProjectDir() + TEXT("Saved/FastBlueprintErrorScanner");
}

FString GetReportFilePath(int InProcessIndex)
{
	return GetReportDirectory() / TEXT("Report_") + FString::FromInt(InProcessIndex) + TEXT(".json");
}

void UFBESProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();

	StartTime = FDateTime::Now();
	DoneProcessCount = 0;
	bRunningRunnable = false;
	
	InitWidget();

	const FString Directory = GetReportDirectory();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.DirectoryExists(*Directory))
	{
		PlatformFile.DeleteDirectoryRecursively(*Directory);
	}
	PlatformFile.CreateDirectoryTree(*Directory);
}

void UFBESProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (bRunningRunnable)
	{
		UpdateProgressUI();
	}
}

void UFBESProgressWidget::InitWidget()
{
	Text_ProgressPercent->SetText(FText::FromString(TEXT("[0%]")));
	Text_CountPass->SetText(FText::FromString(TEXT("0")));
	Text_CountError->SetText(FText::FromString(TEXT("0")));
	Text_EclipseTime->SetText(FText::FromString(TEXT("00:00")));
	Button_Close->OnClicked.AddUniqueDynamic(this, &UFBESProgressWidget::OnClickedButtonClose);
	Button_Close->SetIsEnabled(false);
	ProgressBar_Percent->SetPercent(0);
	CircularThrobber_Circle->SetVisibility(ESlateVisibility::Visible);
	Text_Title->SetText(FText::FromString(TEXT("Scanning")));
}

void UFBESProgressWidget::OnCompleteRunnable(int InError, int InProcessIndex)
{
	AsyncTask(ENamedThreads::GameThread, [this, InProcessIndex, InError]()
	{
		DoneProcessCount += 1;
		const bool IsAllCompleteRunnable = DoneProcessCount >= RunProcessCount;
		if (IsAllCompleteRunnable)
		{
			OnCompleteRunnableAll();
		}

		if (InError != EFBESRunnableError::Type::Ok)
		{
			UE_LOG(LogFBES, Error, TEXT("Error : %d"), InError);
			return;
		}

		const FString JsonFile = GetReportFilePath(InProcessIndex);
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (false == PlatformFile.FileExists(*JsonFile))
		{
			UE_LOG(LogFBES, Error, TEXT("JsonFile not found : %s"), *JsonFile);
			return;
		}

		FString JsonContents;
		if (false == FFileHelper::LoadFileToString(JsonContents, *JsonFile))
		{
			UE_LOG(LogFBES, Error, TEXT("Failed to load file : %s"), *JsonFile);
			return;
		}
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonContents);
		if (false == FJsonSerializer::Deserialize(JsonReader, JsonObject))
		{
			UE_LOG(LogFBES, Error, TEXT("Failed to deserialize json : %s"), *JsonFile);
			return;
		}

		FFBESCompileResultJsonFormat JsonFormat;
		if (false == FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), FFBESCompileResultJsonFormat::StaticStruct(), &JsonFormat, 0))
		{
			UE_LOG(LogFBES, Error, TEXT("Failed to convert json to struct : %s"), *JsonFile);
			return;
		}

		ListViewData.Append(MoveTemp(JsonFormat.Data));
		UE_LOG(LogFBES, Log, TEXT("BlueprintCompileStateList.Num() : %d"), ListViewData.Num());
	});
}

void UFBESProgressWidget::OnProgressRunnable(FFBESBlueprintCompileProgressData const& InData)
{
	ProgressDataMap.FindOrAdd(InData.ProcessIndex, InData) = InData;
}

void UFBESProgressWidget::OnCompleteRunnableAll()
{
	bRunningRunnable = false;
	UpdateProgressUI();
	if (TimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
	CircularThrobber_Circle->SetVisibility(ESlateVisibility::Collapsed);
	Text_Title->SetText(FText::FromString(TEXT("Complited")));
	Button_Close->SetIsEnabled(true);
}

void UFBESProgressWidget::OnClickedButtonClose()
{
	OnCloseWidgetDelegate.ExecuteIfBound(ListViewData);

	TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(TakeWidget());
	if (Window.IsValid())
	{
		Window->RequestDestroyWindow();
	}
}

void UFBESProgressWidget::UpdateProgressUI()
{
	const FTimespan Gap = FDateTime::Now() - StartTime;
	Text_EclipseTime->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Gap.GetMinutes(), Gap.GetSeconds())));

	int PassCount = 0;
	int ErrorCount = 0;
	int TotalCount = 0;
	for (auto It = ProgressDataMap.CreateIterator(); It; ++It)
	{
		FFBESBlueprintCompileProgressData const& Data = It.Value();
		PassCount += Data.PassCount;
		ErrorCount += Data.ErrorCount;
		TotalCount = Data.TotalCount;
	}
	float ZeroToOne = 0;

	if (TotalCount > 0)
	{
		ZeroToOne = (float)(PassCount + ErrorCount) / (float)TotalCount;
		ZeroToOne = FMath::Clamp(ZeroToOne, 0.0f, 1.0f);
	}
	const int ProgressPercent = FMath::Clamp((int)(100.0f * ZeroToOne), 0, 100);

	ProgressBar_Percent->SetPercent(ZeroToOne);
	Text_ProgressPercent->SetText(FText::FromString(FString::Printf(TEXT("[%d%%]"), ProgressPercent)));
	Text_CountPass->SetText(FText::FromString(FString::FromInt(PassCount)));
	Text_CountError->SetText(FText::FromString(FString::FromInt(ErrorCount)));
}

UFBESProgressWidget::FOnCloseWidgetDelegate& UFBESProgressWidget::GetOnCloseDelegate()
{
	return OnCloseWidgetDelegate;
}

void UFBESProgressWidget::SetRunAsMultiThread(bool bMultiThread)
{
	bRunAsMultiThread = bMultiThread;
	UE_LOG(LogFBES, Log, TEXT("bRunAsMultiThread : %d"), bRunAsMultiThread);
}

void UFBESProgressWidget::Run()
{
	RunProcessCount = bRunAsMultiThread ? FPlatformMisc::NumberOfCores() : 1;
	for (int ProcessIndex = 0; ProcessIndex < RunProcessCount; ++ProcessIndex)
	{
		FBESRunnable* Runnable = new FBESRunnable(ProcessIndex, RunProcessCount, GetReportFilePath(ProcessIndex));
		Runnable->GetCompleteDelegate().BindUObject(this, &UFBESProgressWidget::OnCompleteRunnable);
		Runnable->GetProgressDelegate().BindUObject(this, &UFBESProgressWidget::OnProgressRunnable);
		bRunningRunnable = true;
	}

	UE_LOG(LogFBES, Log, TEXT("RunProcessCount : %d"), RunProcessCount);
}