// Fill out your copyright notice in the Description page of Project Settings.


#include "FBESMainWidget.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ListView.h"
#include "FBESListViewItemWidget.h"
#include "FBESProgressWidget.h"

void UFBESMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CheckBox_Warning->OnCheckStateChanged.AddUniqueDynamic(this, &UFBESMainWidget::OnCheckStateChanged);
	CheckBox_Error->OnCheckStateChanged.AddUniqueDynamic(this, &UFBESMainWidget::OnCheckStateChanged);
	Button_Run->OnClicked.AddUniqueDynamic(this, &UFBESMainWidget::OnClickedButtonRun);
}

void UFBESMainWidget::OnClickedButtonRun()
{
	const FStringAssetReference WidgetRef("/Game/BPProgressWidget");
	UEditorUtilityWidgetBlueprint* Blueprint = Cast<UEditorUtilityWidgetBlueprint>(WidgetRef.TryLoad());
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	UFBESProgressWidget* ProgressWidget = Cast<UFBESProgressWidget>(EditorUtilitySubsystem->SpawnAndRegisterTabAndGetID(Blueprint, ProgressWidgetTabId));
	if (false == IsValid(ProgressWidget))
	{
		UE_LOG(LogFBES, Error, TEXT("Failed to spawn widget"));
		return;
	}
	ProgressWidget->GetOnCloseDelegate().BindUObject(this, &UFBESMainWidget::OnCloseProgressWidget);

	const TSharedRef<SWindow> ModalWindow = SNew(SWindow).Title(FText::FromString(TEXT("Fast All Blueprint Error Checker"))).SizingRule(ESizingRule::Autosized).HasCloseButton(false).IsPopupWindow(true);
	ModalWindow->SetContent(ProgressWidget->TakeWidget());
	GEditor->EditorAddModalWindow(ModalWindow);

	Button_Run->SetIsEnabled(false);
}

void UFBESMainWidget::OnCloseProgressWidget(TArray<FFBESCompileResult> const& InList)
{
	GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>()->CloseTabByID(ProgressWidgetTabId);
	
	BlueprintCompileStateList = InList;
	Button_Run->SetIsEnabled(true);
	CheckBox_Error->SetCheckedState(ECheckBoxState::Unchecked);
	CheckBox_Warning->SetCheckedState(ECheckBoxState::Unchecked);
	UpdateUI();
}

void UFBESMainWidget::UpdateUI()
{
	ListView_Blueprint->ClearSelection();
	ListView_Blueprint->ClearListItems();
	
	bool bFilterError = CheckBox_Error->GetCheckedState() == ECheckBoxState::Checked;
	bool bFilterWarn = CheckBox_Warning->GetCheckedState() == ECheckBoxState::Checked;
	TArray<TArray<FFBESCompileResult>::ElementType> FilteredList = BlueprintCompileStateList.FilterByPredicate([bFilterError, bFilterWarn](auto const& BPCompileState)
	{
		bool bAddItem = false;
		bAddItem |= bFilterError && BPCompileState.State == EFBESCompileResultState::Error;
		bAddItem |= bFilterWarn && BPCompileState.State == EFBESCompileResultState::Warning;
		bAddItem |= !bFilterError && !bFilterWarn && BPCompileState.State == EFBESCompileResultState::Pass;
		return bAddItem;
	});
	
	for (FFBESCompileResult const& It : FilteredList)
	{
		UFBESListViewItemObject* Item = NewObject<UFBESListViewItemObject>(this, UFBESListViewItemObject::StaticClass());
		Item->AssetPath = It.AssetPath;
		Item->State = It.State;
		ListView_Blueprint->AddItem(Item);
	}
}

void UFBESMainWidget::OnCheckStateChanged(bool bChecked)
{
	UpdateUI();
}