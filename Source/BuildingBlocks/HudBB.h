#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HudBB.generated.h"

class ACharacterBB;
class UMinimalLayoutBase;
class UModerateLayoutBase;
class UOverloadLayoutBase;

UENUM(BlueprintType)
enum class EHudViewMode: uint8
{
	CleanAndPristine UMETA(Tooltip="Get that mess outta my face!"),
	Minimal UMETA(Tooltip="Just the facts, maam."),
	Moderate UMETA(Tooltip="Keep me well informed"),
	SensoryOverload UMETA(Tooltip="My other UI is a derivatives trading screen")
};

inline EHudViewMode& operator++(EHudViewMode& ViewMode)
{	
	if (ViewMode == EHudViewMode::SensoryOverload)
		ViewMode = EHudViewMode::CleanAndPristine;
	else
		ViewMode = static_cast<EHudViewMode>(static_cast<int>(ViewMode) + 1);

	return ViewMode;
}

inline EHudViewMode& operator--(EHudViewMode& ViewMode)
{
	if (ViewMode == EHudViewMode::CleanAndPristine)
		ViewMode = EHudViewMode::SensoryOverload;
	else
		ViewMode = static_cast<EHudViewMode>(static_cast<int>(ViewMode) - 1);
	return ViewMode;
}

UCLASS(Abstract)
class BUILDINGBLOCKS_API AHudBB : public AHUD
{
public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UMinimalLayoutBase> MinimalLayoutClass = nullptr;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UModerateLayoutBase> ModerateLayoutClass = nullptr;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverloadLayoutBase> OverloadLayoutClass = nullptr;

	// Allow code and blueprints to put the hud in a specific viewmode directly
	// Possibly useful for cinematic cutscenes etc?
	UFUNCTION(BlueprintCallable)
	void SetCurrentViewMode(EHudViewMode NewViewMode);

	// Change to the next viewmode 
	UFUNCTION(BlueprintCallable) 
	void CycleToNextViewMode();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Determines what UI elements should be displayed.
	UPROPERTY(EditAnywhere)
	EHudViewMode CurrentViewMode = EHudViewMode::Minimal;

	// whenever we change the view mode, this private function is called to show the appropriate widgets.
	void UpdateWidgets();

	// Release any delegate bindings.
	void ClearAllHandlers();

	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;

	UPROPERTY()
	TObjectPtr<UMinimalLayoutBase>  MinimalLayoutWidget  = nullptr;
	
	UPROPERTY()
	TObjectPtr<UModerateLayoutBase> ModerateLayoutWidget = nullptr;
	
	UPROPERTY()
	TObjectPtr<UOverloadLayoutBase> OverloadLayoutWidget = nullptr;

	UPROPERTY()
	TObjectPtr<ACharacterBB> PlayerCharacter = nullptr;

	GENERATED_BODY()
};
