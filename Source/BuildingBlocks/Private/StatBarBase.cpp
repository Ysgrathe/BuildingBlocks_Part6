// Fill out your copyright notice in the Description page of Project Settings.

#include "StatBarBase.h"

#include "CustomLogging.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"


void UStatBarBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UE_LOG(BBLog, Warning, TEXT("NativeOnInitialized()"));
	UpdateWidget();
}

void UStatBarBase::OnIntStatUpdated(int32 OldValue, int32 NewValue, int32 MaxValue)
{
	// Just use the float version of the function!
	OnFloatStatUpdated(static_cast<float>(OldValue), static_cast<float>(NewValue), static_cast<float>(MaxValue));
}

void UStatBarBase::OnFloatStatUpdated(float OldValue, float NewValue, float MaxValue)
{
	// For now, we can ignore the OldValue, this will be used later for animating the bar.

	// Calculate the new percentage, and clamp the value between 0 and 1,
	// just in case someone passes invalid values

	// Prevent divide by zero errors.
	// The macro SMALL_NUMBER would be appropriate here..
	// but KINDA_SMALL_NUMBER is the sort of Name which goes against all the 'stuffy' coding conventions,
	// and it's official EPIC code, so I wanted to highlight that! ;-)
	// I think its 'KINDA' amusing.
	if (MaxValue == 0.f) MaxValue = KINDA_SMALL_NUMBER;

	CurrentPercentage = FMath::Clamp(NewValue / MaxValue, 0.f, 1.f);
	CurrentValue      = NewValue;
	UpdateWidget();
}

void UStatBarBase::ProcessCurrentValueText()
{
	// Ultimately this should be handled in a culture appropriate matter,
	// i.e suffixes like 'k' for thousands should be dependant on culture,
	// and may be prefixes in some cultures.
	// HOWEVER - for the purposes of this tutorial, we will keep it simple

	// if the number is <10 then display it as a float to 2DP : 0.01
	// if the number is <100, then display it as a float with 1DP : 99.9
	// if the number is <1000 then display it as an integer with 0DP: 986
	// if the number is >= 1000, then divide it by 1000 and apply the rules above, and add a 'k' on the end

	FString FloatString;

	if (CurrentValue < 1000.f)
	{
		FloatString = FString::SanitizeFloat(CurrentValue);

		if (CurrentValue < 100.f)
		{
			int32 StringLen = FloatString.Len();
			if (StringLen > 4)
				FloatString = FloatString.Left(4);
			else if (StringLen < 4)
				FloatString = FloatString.Append("0", 4 - StringLen);
		}
		else
		{
			FloatString = FloatString.Left(3);
		}
	}
	else
	{
		// scaled value
		float ScaledValue = CurrentValue / 1000.f;
		FloatString       = FString::SanitizeFloat(ScaledValue);
		if (ScaledValue < 10.f)
			FloatString = FloatString.Left(3).Append(TEXT("k"));
		else
		{
			FloatString = FloatString.Left(2).Append(TEXT("k"));
		}
	}

	CurrentValueText = FText::FromString(FloatString);
}

void UStatBarBase::UpdateWidget()
{
	// Check that the controls we want actually exist
	if (!PercentBar_Filled ||
		!PercentBar_Empty ||
		!MainBorder ||
		!IconImage) return;

	FSlateChildSize EmptySize = FSlateChildSize(ESlateSizeRule::Fill);
	EmptySize.Value           = 1.f - CurrentPercentage;

	FSlateChildSize FilledSize = FSlateChildSize(ESlateSizeRule::Fill);
	FilledSize.Value           = CurrentPercentage;

	if (UVerticalBoxSlot* FilledSlot = Cast<UVerticalBoxSlot>(PercentBar_Filled->Slot))
		FilledSlot->SetSize(FilledSize);

	if (UVerticalBoxSlot* EmptySlot = Cast<UVerticalBoxSlot>(PercentBar_Empty->Slot))
		EmptySlot->SetSize(EmptySize);

	MainBorder->SetBrushColor(BarBackgroundColor);
	PercentBar_Filled->SetBrushColor(BarForegroundColor);
	IconImage->SetBrush(IconBrush);

	ProcessCurrentValueText();

	ValueText->SetText(CurrentValueText);

	PercentBars->SetVisibility(IsFullSize ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

#if WITH_EDITOR

void UStatBarBase::OnDesignerChanged(const FDesignerChangedEventArgs& EventArgs)
{
	Super::OnDesignerChanged(EventArgs);
	// Update the widget, after editor changes due to layout,
	// for example, resizing the widget, or a container that the widget is in.
	UpdateWidget();
}

void UStatBarBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Update the widget, after it's properties have been changed.

	// In this case it IS sufficient to just call UpdateWidget()
	// which means changing ANY property will trigger the update.
	// The commented code demonstrates one way, you could run specific code
	// when when individual properties are changed, perhaps because you want to
	// do specific things only when specific properties are changed in the editor.

	//const FString PropertyName = ((PropertyChangedEvent.Property != nullptr)
	//	                              ? PropertyChangedEvent.Property->GetFName()
	//	                              : NAME_None).ToString();

	//if (PropertyName == TEXT("CurrentPercentage") || PropertyName == TEXT("CurrentValue"))
	//{
		UpdateWidget();
	//}
}

#endif
