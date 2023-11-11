#include "HudBB.h"
#include "CustomLogging.h"
#include "CharacterBB.h"
#include "HSPBarBase.h"
#include "MinimalLayoutBase.h"
#include "ModerateLayoutBase.h"
#include "OverloadLayoutBase.h"
#include "StatBarBase.h"

void AHudBB::BeginPlay()
{
	Super::BeginPlay();

	// Grab a reference to the current world
	World = GetWorld();
	checkf(World, TEXT("Failed to reference world."));

	// Ensure we have valid values for the 3 classes of widget used by the HUD
	checkf(MinimalLayoutClass, TEXT("Invalid MinimalLayoutClass reference."));
	checkf(ModerateLayoutClass, TEXT("Invalid ModerateLayoutClass reference."));
	checkf(OverloadLayoutClass, TEXT("Invalid OverloadLayoutClass reference."));

	// create the 3 types of layout widget, and add them to the viewport
	// We could have been 'clever' here, and had maybe a single widget which 'mutates'
	// based on the requirements, but this IS a tutorial afterall, and we wanna keep it simple(er!)
	// When creating a widget, the first parameter (owning object) must be one of the following types:
	// UWidget, UWidgetTree, APlayerController, UGameInstance, or UWorld
	MinimalLayoutWidget = CreateWidget<UMinimalLayoutBase>(World, MinimalLayoutClass);
	MinimalLayoutWidget->AddToViewport();
	MinimalLayoutWidget->SetVisibility(ESlateVisibility::Collapsed);

	ModerateLayoutWidget = CreateWidget<UModerateLayoutBase>(World, ModerateLayoutClass);
	ModerateLayoutWidget->AddToViewport();
	ModerateLayoutWidget->SetVisibility(ESlateVisibility::Collapsed);

	OverloadLayoutWidget = CreateWidget<UOverloadLayoutBase>(World, OverloadLayoutClass);
	OverloadLayoutWidget->AddToViewport();
	OverloadLayoutWidget->SetVisibility(ESlateVisibility::Collapsed);

	// Get a reference to the character, and hook up the stat handlers
	if (APlayerController* PlayerController = GetOwningPlayerController())
		PlayerCharacter = Cast<ACharacterBB>(PlayerController->GetPawn());
	checkf(PlayerCharacter, TEXT("Unable to get a reference to the player character"));

	// Set the initial viewmode to the 'current' one, which allows setting via the editor.
	//SetCurrentViewMode(CurrentViewMode);
	UpdateWidgets();
}

void AHudBB::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Release any event handlers
	ClearAllHandlers();

	Super::EndPlay(EndPlayReason);
}

void AHudBB::SetCurrentViewMode(EHudViewMode NewViewMode)
{
	CurrentViewMode = NewViewMode;
	UpdateWidgets();
}

void AHudBB::CycleToNextViewMode()
{
	++CurrentViewMode;
	BBLOG(Log, "CycleToNextViewMode {0}", UEnum::GetValueAsString(CurrentViewMode));
	UpdateWidgets();
}

void AHudBB::UpdateWidgets()
{
	// Unhook any delegate handlers.
	ClearAllHandlers();

	// Set all the widgets so we see none of them
	MinimalLayoutWidget->SetVisibility(ESlateVisibility::Collapsed);
	ModerateLayoutWidget->SetVisibility(ESlateVisibility::Collapsed);
	OverloadLayoutWidget->SetVisibility(ESlateVisibility::Collapsed);

	switch (CurrentViewMode)
	{
	case EHudViewMode::CleanAndPristine:
		// Currently there isn't actually anything to do here!
		break;
	case EHudViewMode::Minimal:
		PlayerCharacter->OnHealthChanged.AddDynamic(MinimalLayoutWidget->HSPBar->HealthBar,
		                                            &UStatBarBase::OnIntStatUpdated);
		PlayerCharacter->OnStaminaChanged.AddDynamic(MinimalLayoutWidget->HSPBar->StaminaBar,
		                                             &UStatBarBase::OnFloatStatUpdated);
		PlayerCharacter->OnPsiPowerChanged.AddDynamic(MinimalLayoutWidget->HSPBar->PsiBar,
		                                              &UStatBarBase::OnFloatStatUpdated);
		MinimalLayoutWidget->SetVisibility(ESlateVisibility::Visible);
		break;
	case EHudViewMode::Moderate:
		PlayerCharacter->OnHealthChanged.AddDynamic(ModerateLayoutWidget->HSPBar->HealthBar,
		                                            &UStatBarBase::OnIntStatUpdated);
		PlayerCharacter->OnStaminaChanged.AddDynamic(ModerateLayoutWidget->HSPBar->StaminaBar,
		                                             &UStatBarBase::OnFloatStatUpdated);
		PlayerCharacter->OnPsiPowerChanged.AddDynamic(ModerateLayoutWidget->HSPBar->PsiBar,
		                                              &UStatBarBase::OnFloatStatUpdated);
		ModerateLayoutWidget->SetVisibility(ESlateVisibility::Visible);
		break;
	case EHudViewMode::SensoryOverload:
		PlayerCharacter->OnHealthChanged.AddDynamic(OverloadLayoutWidget->HSPBar->HealthBar,
		                                            &UStatBarBase::OnIntStatUpdated);
		PlayerCharacter->OnStaminaChanged.AddDynamic(OverloadLayoutWidget->HSPBar->StaminaBar,
		                                             &UStatBarBase::OnFloatStatUpdated);
		PlayerCharacter->OnPsiPowerChanged.AddDynamic(OverloadLayoutWidget->HSPBar->PsiBar,
		                                              &UStatBarBase::OnFloatStatUpdated);
		OverloadLayoutWidget->SetVisibility(ESlateVisibility::Visible);
		break;
	default: ;
	}

	// This ensures that even if something has not changed recently, the newly switched-to widget will get sent
	// the latest character stats, so it can update itself.
	PlayerCharacter->BroadcastCurrentStats();
}

void AHudBB::ClearAllHandlers()
{
	if (PlayerCharacter)
	{
		PlayerCharacter->OnHealthChanged.Clear();
		PlayerCharacter->OnStaminaChanged.Clear();
		PlayerCharacter->OnPsiPowerChanged.Clear();
		PlayerCharacter->OnKeyWalletAction.Clear();
	}
}
