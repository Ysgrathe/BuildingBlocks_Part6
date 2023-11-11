// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WidgetBBBase.generated.h"

/* */
UCLASS(Abstract)
class BUILDINGBLOCKS_API UWidgetBBBase : public UUserWidget
{
public:
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

	UPROPERTY(EditAnywhere)
	uint16 SomeValue = 0;

protected:


private:
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FSomeStruct
{

	UPROPERTY(EditAnywhere)
	uint16 SomeValue = 0;

	
	GENERATED_BODY()
	
};
