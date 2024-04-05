#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "FBESListViewItemWidget.generated.h"

class URichTextBlock;
class UButton;
class UWidgetSwitcher;

UCLASS()
class FASTBLUEPRINTERRORSCANNER_API UFBESListViewItemObject : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FString AssetPath;

	UPROPERTY()
	int State;
};

UCLASS()
class FASTBLUEPRINTERRORSCANNER_API UFBESListViewItemWidget : public UEditorUtilityWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* Switcher_State;
	UPROPERTY(meta = (BindWidget))
	URichTextBlock* Text_AssetPath;
	UPROPERTY(meta = (BindWidget))
	UButton* Button_OpenAsset;

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

public:
	UFUNCTION()
	void OnClickedButtonOpenAsset();

private:
	FString AssetPath;
};