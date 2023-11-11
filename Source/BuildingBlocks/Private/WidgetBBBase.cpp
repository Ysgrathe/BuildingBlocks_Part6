// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildingBlocks/Public/WidgetBBBase.h"

#if WITH_EDITOR
const FText UWidgetBBBase::GetPaletteCategory()
{
	return NSLOCTEXT("UMG", "CustomPaletteCategory", "GGameDev!");
}
#endif
