#include "FBESListViewItemWidget.h"

#include "Components/Button.h"
#include "Components/RichTextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "FBESStruct.h"

void UFBESListViewItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	UFBESListViewItemObject* ItemObject = Cast<UFBESListViewItemObject>(ListItemObject);
	AssetPath = ItemObject->AssetPath;
	
	Text_AssetPath->SetText(FText::FromString(ItemObject->AssetPath));
	Switcher_State->SetActiveWidgetIndex(ItemObject->State);
	Button_OpenAsset->OnClicked.AddUniqueDynamic(this, &UFBESListViewItemWidget::OnClickedButtonOpenAsset);
}

void UFBESListViewItemWidget::OnClickedButtonOpenAsset()
{
	UObject* Asset = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);
	if (false == IsValid(Asset))
	{
		UE_LOG(LogFBES, Log, TEXT("Failed load asset"))
		return;
	}

	const TArray<UObject*> Assets { Asset };
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAssets(Assets);
}