#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "FBESStruct.h"
#include "FBESProgressWidget.generated.h"

class UProgressBar;
class UButton;
class UTextBlock;
class SWindow;
class UCircularThrobber;

UCLASS()
class FASTBLUEPRINTERRORSCANNER_API UFBESProgressWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_ProgressPercent;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_CountPass;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_CountError;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_EclipseTime;
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Close;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ProgressBar_Percent;
	UPROPERTY(meta = (BindWidget))
	UCircularThrobber* CircularThrobber_Circle;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Title;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void InitWidget();
	void OnCompleteRunnable(int InError, int InProcessIndex);
	void OnProgressRunnable(FFBESBlueprintCompileProgressData const& InData);
	void OnCompleteRunnableAll();

	UFUNCTION()
	void OnClickedButtonClose();
	void UpdateProgressUI();

public:
	DECLARE_DELEGATE_OneParam(FOnCloseWidgetDelegate, TArray<FFBESCompileResult> const&);
	FOnCloseWidgetDelegate& GetOnCloseDelegate();
	
	void SetRunAsMultiThread(bool bMultiThread);
	void Run();

protected:
	int RunProcessCount = 0;
	int DoneProcessCount = 0;
	FDateTime StartTime;
	TArray<FFBESCompileResult> ListViewData;
	TMap<int, FFBESBlueprintCompileProgressData> ProgressDataMap;
	FOnCloseWidgetDelegate OnCloseWidgetDelegate;
	FTimerHandle TimerHandle;
	bool bRunAsMultiThread = true;
	bool bRunningRunnable = false;
};
