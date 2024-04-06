// Fill out your copyright notice in the Description page of Project Settings.


#include "FBESMainWidget.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ListView.h"
#include "FBESListViewItemWidget.h"
#include "FBESProgressWidget.h"

TArray<FFBESCompileResult> UFBESMainWidget::BlueprintCompileStateList;
ECheckBoxState UFBESMainWidget::ErrorCheckedState = ECheckBoxState::Unchecked;
ECheckBoxState UFBESMainWidget::MultiThreadCheckedState = ECheckBoxState::Checked;

void UFBESMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CheckBox_MultiThread->SetIsChecked(MultiThreadCheckedState == ECheckBoxState::Checked);
	CheckBox_MultiThread->OnCheckStateChanged.AddUniqueDynamic(this, &UFBESMainWidget::OnChangeMultiThreadOnOff);
	CheckBox_Error->SetIsChecked(ErrorCheckedState == ECheckBoxState::Checked);
	CheckBox_Error->OnCheckStateChanged.AddUniqueDynamic(this, &UFBESMainWidget::OnChangeErrorFilter);
	Button_Run->OnClicked.AddUniqueDynamic(this, &UFBESMainWidget::OnClickedButtonRun);

	UpdateListView();
}

void UFBESMainWidget::OnClickedButtonRun()
{
	const FStringAssetReference WidgetRef("/FastBlueprintErrorScanner/FastBlueprintErrorScannerProgress");
	UEditorUtilityWidgetBlueprint* WidgetBlueprint = Cast<UEditorUtilityWidgetBlueprint>(WidgetRef.TryLoad());
	UFBESProgressWidget* ProgressWidget = Cast<UFBESProgressWidget>(GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>()->SpawnAndRegisterTabAndGetID(WidgetBlueprint, ProgressWidgetTabId));
	const bool bMultiThread = CheckBox_MultiThread->GetCheckedState() == ECheckBoxState::Checked;
	ProgressWidget->SetRunAsMultiThread(bMultiThread);
	ProgressWidget->GetOnCloseDelegate().BindUObject(this, &UFBESMainWidget::OnCloseProgressWidget);
	ProgressWidget->Run();

	const TSharedRef<SWindow> ModalWindow = SNew(SWindow).Title(FText::FromString(TEXT("Fast Blueprint Error Scanner"))).SizingRule(ESizingRule::Autosized).HasCloseButton(false).IsPopupWindow(true);
	ModalWindow->SetContent(ProgressWidget->TakeWidget());
	GEditor->EditorAddModalWindow(ModalWindow);

	Button_Run->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UFBESMainWidget::OnCloseProgressWidget(TArray<FFBESCompileResult> const& InList)
{
	GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>()->CloseTabByID(ProgressWidgetTabId);

	BlueprintCompileStateList = InList;
	Button_Run->SetVisibility(ESlateVisibility::Visible);
	CheckBox_Error->SetCheckedState(ECheckBoxState::Unchecked);
	UpdateListView();
}

void UFBESMainWidget::UpdateListView()
{
	ListView_Blueprint->ClearSelection();
	ListView_Blueprint->ClearListItems();

	bool bFilterError = CheckBox_Error->GetCheckedState() == ECheckBoxState::Checked;
	TArray<TArray<FFBESCompileResult>::ElementType> FilteredList = BlueprintCompileStateList.FilterByPredicate([bFilterError](auto const& BPCompileState)
	{
		if (bFilterError)
		{
			return BPCompileState.NumErrors > 0;
		}
		return true;
	});

	for (FFBESCompileResult const& It : FilteredList)
	{
		UFBESListViewItemObject* Item = NewObject<UFBESListViewItemObject>(this, UFBESListViewItemObject::StaticClass());
		Item->AssetPath = It.AssetPath;
		Item->NumErrors = It.NumErrors;
		ListView_Blueprint->AddItem(Item);
	}
}

void UFBESMainWidget::OnChangeErrorFilter(bool bChecked)
{
	UpdateListView();
	ErrorCheckedState = CheckBox_Error->GetCheckedState();
	UE_LOG(LogFBES, Log, TEXT("ErrorCheckedState : %d"), (int)ErrorCheckedState);
}

void UFBESMainWidget::OnChangeMultiThreadOnOff(bool bIsChecked)
{
	MultiThreadCheckedState = CheckBox_MultiThread->GetCheckedState();
	UE_LOG(LogFBES, Log, TEXT("MultiThreadCheckedState : %d"), (int)MultiThreadCheckedState);
}