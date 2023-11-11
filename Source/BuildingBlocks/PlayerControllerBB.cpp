// Fill out your copyright notice in the Description page of Project Settings.
#include "PlayerControllerBB.h"
#include "CharacterBB.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HudBB.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

class IImageWrapper;

void APlayerControllerBBBase::OnPossess(APawn* aPawn)
{
	// Call the parent method, to let it do anything it needs to
	Super::OnPossess(aPawn);

	// Store a reference to the HUD
	PlayerHud = Cast<AHudBB>(GetHUD());
	checkf(PlayerHud, TEXT("Unable to get reference to the HUD"));

	// Store a reference to the Player's Pawn
	PlayerCharacter = Cast<ACharacterBB>(aPawn);
	checkf(PlayerCharacter,
	       TEXT("APlayerControllerBBBase derived classes should only posess ACharacterBBBase derived pawns"));

	// Get a reference to the EnhancedInputComponent
	EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	checkf(EnhancedInputComponent,
	       TEXT("Unable to get reference to the EnhancedInputComponent."));

	// Get the local player subsystem
	// Just a local variable, we dont need to refer to it again after this
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	checkf(InputSubsystem,
	       TEXT("Unable to get reference to the EnhancedInputLocalPlayerSubsystem."));

	// Wipe existing mappings, and add our mapping.
	checkf(InputMappingContent, TEXT("InputMappingContent was not specified."));
	InputSubsystem->ClearAllMappings();
	InputSubsystem->AddMappingContext(InputMappingContent, 0);

	// Bind the input actions.
	// Only attempt to bind if valid values were provided.
	if (ActionMove)
		EnhancedInputComponent->BindAction(ActionMove, ETriggerEvent::Triggered, this,
		                                   &APlayerControllerBBBase::HandleMove);

	if (ActionLook)
		EnhancedInputComponent->BindAction(ActionLook, ETriggerEvent::Triggered, this,
		                                   &APlayerControllerBBBase::HandleLook);

	if (ActionJump)
		EnhancedInputComponent->BindAction(ActionJump, ETriggerEvent::Triggered, this,
		                                   &APlayerControllerBBBase::HandleJump);

	if (ActionPsiBlast)
		EnhancedInputComponent->BindAction(ActionPsiBlast, ETriggerEvent::Triggered, this,
		                                   &APlayerControllerBBBase::HandlePsiBlast);

	if (ActionToggleCrouch)
		EnhancedInputComponent->BindAction(ActionToggleCrouch, ETriggerEvent::Triggered, this,
		                                   &APlayerControllerBBBase::HandleToggleCrouch);

	if (ActionToggleSprint)
		EnhancedInputComponent->BindAction(ActionToggleSprint, ETriggerEvent::Triggered, this,
		                                   &APlayerControllerBBBase::HandleToggleSprint);

	if (ActionCycleUIMode)
		EnhancedInputComponent->BindAction(ActionCycleUIMode, ETriggerEvent::Triggered, this,
		                                   &APlayerControllerBBBase::HandleCycleUIMode);


	GEngine->GameViewport->OnScreenshotCaptured().AddUObject(this, &APlayerControllerBBBase::AcceptScreenshot);
}

void APlayerControllerBBBase::OnUnPossess()
{
	// Unbind things here...
	EnhancedInputComponent->ClearActionBindings();

	// Call the parent method, in case it needs to do anything.
	Super::OnUnPossess();
}

void APlayerControllerBBBase::HandleLook(const FInputActionValue& InputActionValue)
{
	// Input is a Vector2D
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

	// Add yaw and pitch input to controller
	AddYawInput(LookAxisVector.X);
	AddPitchInput(LookAxisVector.Y);
}

void APlayerControllerBBBase::HandleMove(const FInputActionValue& InputActionValue)
{
	// Value is a Vector2D
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	// Add movement to the Player's Character Pawn
	if (PlayerCharacter)
	{
		PlayerCharacter->AddMovementInput(PlayerCharacter->GetActorForwardVector(), MovementVector.Y);
		PlayerCharacter->AddMovementInput(PlayerCharacter->GetActorRightVector(), MovementVector.X);
	}
}

void APlayerControllerBBBase::HandleJump()
{
	// Input is 'Digital' (value not used here)
	// Make the Player's Character Pawn jump, disabling crouch if it was active
	if (PlayerCharacter)
	{
		PlayerCharacter->UnCrouch();
		PlayerCharacter->Jump();
	}
}

void APlayerControllerBBBase::HandlePsiBlast()
{
	if (PlayerCharacter) PlayerCharacter->PsiBlast();
}

void APlayerControllerBBBase::HandleToggleSprint()
{
	if (PlayerCharacter) PlayerCharacter->ToggleRunning();
}

void APlayerControllerBBBase::HandleToggleCrouch()
{
	if (PlayerCharacter && PlayerCharacter->bIsCrouched)
		PlayerCharacter->UnCrouch();
	else
		PlayerCharacter->Crouch();
}

void APlayerControllerBBBase::HandleCycleUIMode()
{
	if (PlayerHud)
		PlayerHud->CycleToNextViewMode();
}

void APlayerControllerBBBase::AcceptScreenshot(int Width, int Height, const TArray<FColor>& Colors)
{
	static int32 Index = 0;

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(
		FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	// Provide image wrapper with raw pixels (TArray<FColor>)
	ImageWrapper->SetRaw(&Colors[0], Colors.Num() * sizeof(FColor), Width, Height, ERGBFormat::BGRA, 8);

	FString SaveLoc = FString::Printf(TEXT("F:\\Work\\Projects\\YetiGame\\SS_%d.png"), Index);
	Index++;

	AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask, [ImageWrapper,SaveLoc]()
	{
		// Get the image using 90% JPEG compression
		const TArray64<uint8> CompressedImage = ImageWrapper->GetCompressed(90);
		FFileHelper::SaveArrayToFile(CompressedImage, *SaveLoc);
	});
}
