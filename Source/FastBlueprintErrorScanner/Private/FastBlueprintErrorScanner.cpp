// Copyright Epic Games, Inc. All Rights Reserved.

#include "FastBlueprintErrorScanner.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "LevelEditor.h"

static FAutoConsoleCommand CVarOpenFastBlueprintErrorScannerWidget(
	TEXT("FastBlueprintErrorScanner"),
	TEXT("open the FastBlueprintErrorScanner widget."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		FastBlueprintErrorScannerModule::OpenFastBlueprintErrorScanner();
	})
);


#define LOCTEXT_NAMESPACE "FastBlueprintErrorScannerModule"

void FastBlueprintErrorScannerModule::StartupModule()
{
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuExtension("LevelEditor", EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateRaw(this, &FastBlueprintErrorScannerModule::AddMenuExtension));
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void FastBlueprintErrorScannerModule::ShutdownModule()
{
}

void FastBlueprintErrorScannerModule::AddMenuExtension(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("Plugin Extensions", LOCTEXT("PluginExtensions", "Plugin Extensions"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("FastBlueprintErrorScanner", "Fast Blueprint Error Scanner"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FastBlueprintErrorScannerModule::OpenFastBlueprintErrorScanner)));
	}
	MenuBuilder.EndSection();
}

void FastBlueprintErrorScannerModule::OpenFastBlueprintErrorScanner()
{
	const FStringAssetReference WidgetRef("/FastBlueprintErrorScanner/FastBlueprintErrorScanner");
	UEditorUtilityWidgetBlueprint* WidgetBP = Cast<UEditorUtilityWidgetBlueprint>(WidgetRef.TryLoad());
	WidgetBP->SetRegistrationName("Fast Blueprint Error Scanner");
	GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>()->SpawnAndRegisterTab(WidgetBP);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FastBlueprintErrorScannerModule, FastBlueprintErrorScanner)
