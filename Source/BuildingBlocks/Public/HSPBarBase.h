// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetBBBase.h"
#include "Blueprint/UserWidget.h"
#include "HSPBarBase.generated.h"

class UStatBarBase;
/* */
UCLASS(Abstract)
class BUILDINGBLOCKS_API UHSPBarBase : public UWidgetBBBase
{
public:
	UPROPERTY(BlueprintReadOnly, Category = "Constituent Controls", meta = (BindWidget))
	TObjectPtr<UStatBarBase> HealthBar = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "Constituent Controls", meta = (BindWidget))
	TObjectPtr<UStatBarBase> StaminaBar = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "Constituent Controls", meta = (BindWidget))
	TObjectPtr<UStatBarBase> PsiBar = nullptr;

protected:


private:

	
	GENERATED_BODY()
};
