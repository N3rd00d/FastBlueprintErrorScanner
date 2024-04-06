// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "FBESStruct.h"
#include "FBESMainWidget.generated.h"

class UCheckBox;
class UProgressBar;
class UListView;
class UButton;

UCLASS(Blueprintable, BlueprintType)
class FASTBLUEPRINTERRORSCANNER_API UFBESMainWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickedButtonRun();

	UFUNCTION()
	void OnCloseProgressWidget(TArray<FFBESCompileResult> const& InList);

	void UpdateListView();

	UFUNCTION()
	void OnChangeErrorFilter(bool bChecked);
	
	UFUNCTION()
	void OnChangeMultiThreadOnOff(bool bIsChecked);

public:
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Run;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* CheckBox_Error;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* CheckBox_MultiThread;

	UPROPERTY(meta = (BindWidget))
	UListView* ListView_Blueprint;

protected:
	FName ProgressWidgetTabId;

private:
	static TArray<FFBESCompileResult> BlueprintCompileStateList;
	static ECheckBoxState ErrorCheckedState;
	static ECheckBoxState MultiThreadCheckedState;
};
