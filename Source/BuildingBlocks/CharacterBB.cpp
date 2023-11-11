// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBB.h"

#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ACharacterBB::ACharacterBB()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickInterval(0.5f);
	AActor::SetActorTickEnabled(true);
}

// Called when the game starts or when spawned
void ACharacterBB::BeginPlay()
{
	Super::BeginPlay();
	if (GetMovementComponent()) GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	BroadcastCurrentStats();
}

void ACharacterBB::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce)
{
	// If the player is running, check that they have stamina available,
	// otherwise kick them out of running mode
	if (bIsRunning && CurrentStamina <= 0)
	{
		SetRunning(false);
	}

	Super::AddMovementInput(WorldDirection, ScaleValue, bForce);

	// set the flag to indicate if the character ran.
	if (bIsRunning) bHasRan = true;
}

void ACharacterBB::Jump()
{
	// Jump requires stamina
	if (CurrentStamina - JumpStaminaCost >= 0.f)
	{
		UnCrouch();
		Super::Jump();
		bHasJumped = true;
	}
}

void ACharacterBB::Crouch(bool bClientSimulation)
{
	SetRunning(false);
	Super::Crouch(bClientSimulation);
}

void ACharacterBB::Tick(float DeltaTime)
{
	// Call the super... it probably needs to do stuff!
	Super::Tick(DeltaTime);

	// There are 2 things which can restore over time, these are:
	// - Stamina
	// - Psi Power

#pragma region Update Stamina
	// How has stamina been affected?
	// We move from the worst-case scenario to the best.
	// if they jumped, otherwise if they ran, otherwise if they rested.
	// and if they did none of those they get the default recharge rate.

	float ActualStaminaRecuperationFactor = StaminaRecuperationFactor;

	if (bHasJumped) ActualStaminaRecuperationFactor = -JumpStaminaCost;
		// If the sprinted then they get a deduction
	else if (bHasRan) ActualStaminaRecuperationFactor = -RunStaminaCost;
		// Player gets double stamina regain when crouched.
	else if (bIsCrouched) ActualStaminaRecuperationFactor = RestStaminaRebate;

	// Keep track of the value before it is changed.
	const float PreviousStamina = CurrentStamina;

	// Update the current value, capping it to the min and max allowable
	CurrentStamina = FMath::Clamp(CurrentStamina + ActualStaminaRecuperationFactor, 0.f, MaxStamina);

	// If the value has actually changed, we need to notify any listeners
	if (CurrentStamina != PreviousStamina)
	{
		OnStaminaChanged.Broadcast(PreviousStamina, CurrentStamina, MaxStamina);
	}

	// Reset the flags indicating physical exertion
	bHasRan    = false;
	bHasJumped = false;

#pragma endregion

#pragma region Update Psi Power
	// This is a bit simpler than stamina, because nothing here needs to deduct from the current value,
	// only restore it, and at a constant rate.

	if (CurrentPsiPower != MaxPsiPower)
	{
		// Keep track of the value before it is changed.
		const float PreviousPsiPower = CurrentPsiPower;

		CurrentPsiPower = FMath::Clamp(CurrentPsiPower + PsiRechargeRate, 0, MaxPsiPower);
		OnPsiPowerChanged.Broadcast(PreviousPsiPower, CurrentPsiPower, MaxPsiPower);
	}

#pragma endregion

	// Temporarily display debug information
	/*
		GEngine->AddOnScreenDebugMessage(-1, 0.49f, FColor::Silver,
		                                 *(FString::Printf(
			                                 TEXT("Movement - IsCrouched:%d | IsSprinting:%d"), bIsCrouched, bIsRunning)));
		GEngine->AddOnScreenDebugMessage(-1, 0.49f, FColor::Red,
		                                 *(FString::Printf(
			                                 TEXT("Health - Current:%d | Maximum:%d"), CurrentHealth, MaxHealth)));
		GEngine->AddOnScreenDebugMessage(-1, 0.49f, FColor::Green,
		                                 *(FString::Printf(
			                                 TEXT("Stamina - Current:%f | Maximum:%f"), CurrentStamina, MaxStamina)));
		GEngine->AddOnScreenDebugMessage(-1, 0.49f, FColor::Cyan,
		                                 *(FString::Printf(
			                                 TEXT("PsiPower - Current:%f | Maximum:%f"), CurrentPsiPower, MaxPsiPower)));
		GEngine->AddOnScreenDebugMessage(-1, 0.49f, FColor::Orange,
		                                 *(FString::Printf(TEXT("Keys - %d Keys Currently held"), KeyWallet.Num())));
	*/
}

void ACharacterBB::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACharacterBB::SetRunning(bool IsRunning)
{
	// Set the value
	bIsRunning = IsRunning;

	// Set the speed at which the player moves, based on if they are walking or running
	GetCharacterMovement()->MaxWalkSpeed = bIsRunning ? RunningMaxWalkSpeed : NormalMaxWalkSpeed;
}

void ACharacterBB::ToggleRunning()
{
	SetRunning(!bIsRunning);
}

void ACharacterBB::SetHasJumped()
{
	bHasJumped = true;
}

void ACharacterBB::SetHasRan()
{
	bHasRan = true;
}

void ACharacterBB::BroadcastCurrentStats()
{
	OnHealthChanged.Broadcast(CurrentHealth, CurrentHealth, MaxHealth);
	OnStaminaChanged.Broadcast(CurrentStamina, CurrentStamina, MaxStamina);
	OnPsiPowerChanged.Broadcast(CurrentPsiPower, CurrentPsiPower, MaxPsiPower);

	// Make a string of all the keys
	// If there are ANY members, the string will end with a trailing comma ','
	// We dont care to remove that here, it doesnt matter.
	FString AllKeys = FString();
	for (FString Key : KeyWallet)
	{
		AllKeys.Appendf(TEXT("%s,"),&Key);
	}

	OnKeyWalletAction.Broadcast(AllKeys, EPlayerKeyAction::CountKeys, true);
}

int ACharacterBB::GetHealth()
{
	return CurrentHealth;
}

int ACharacterBB::GetMaxHealth()
{
	return MaxHealth;
}

void ACharacterBB::UpdateHealth(int DeltaHealth)
{
	// If the player is already dead, their health cannot be modified again.
	// This prevents multiple effects 'stacking' and a player becoming dead 
	// and instantly reviving. DEAD IS DEAD.
	if (CurrentHealth <= 0.f) return;

	// What is the value, before we change it?
	int OldValue = CurrentHealth;

	CurrentHealth += DeltaHealth;

	// Make sure that the new CurrentHealth value is inside an acceptable range
	// In this case it will never be less than -1, or more than the MaxHealth
	CurrentHealth = FMath::Clamp(CurrentHealth, -1.f, MaxHealth);

	// Compare the value before we changed it with the new value.
	// We only want to notify listeners if this is different.
	// Why wouldn't it be? 
	// Because, the player might drink a healing potion, 
	// when they are already at full health, etc.
	if (CurrentHealth != OldValue)
	{
		OnHealthChanged.Broadcast(OldValue, CurrentHealth, MaxHealth);
	}

	// Did the player just die?
	if (CurrentHealth <= 0.f)
	{
		// The player is dead! Do something!
		OnPlayerDied.Broadcast();
	}
}

void ACharacterBB::RestoreToFullHealth()
{
	// Only do something if we are not already at max health.
	if (CurrentHealth < MaxHealth)
	{
		int OldValue  = CurrentHealth;
		CurrentHealth = MaxHealth;
		OnHealthChanged.Broadcast(OldValue, CurrentHealth, MaxHealth);
	}
}

void ACharacterBB::SetMaxHealth(int NewMaxHealth)
{
	int OldValue = MaxHealth;

	// We just assume that the new value is within an acceptable range.
	// Might be better if we had some range checking?
	MaxHealth = NewMaxHealth;

	// Changing the MaxHealth 'might' also change the CurrentHealth,
	// if it is now less than the current health.
	// Regardless of that, we should fire the notification,
	// just in case there are any widgets listening which need to calculate a new %

	if (MaxHealth != OldValue) // We need to fire a notification
	{
		if (MaxHealth < OldValue)
		{
			// MaxHealth decreased, 
			// so we need to also cap the CurrentHealth to the new Max.
			if (CurrentHealth > MaxHealth) CurrentHealth = MaxHealth;
		}

		// There was a change, so notify any listeners
		OnHealthChanged.Broadcast(OldValue, CurrentHealth, MaxHealth);
	}
}

float ACharacterBB::GetStamina()
{
	return CurrentStamina;
}

float ACharacterBB::GetStaminaRecuperationFactor()
{
	return StaminaRecuperationFactor;
}

void ACharacterBB::SetStaminaRecuperationFactor(float NewStaminaRecuperationFactor)
{
	// Might be sensible to check that this is a +ve value, within some
	// sensible range.
	StaminaRecuperationFactor = NewStaminaRecuperationFactor;
}

float ACharacterBB::GetPsiPower()
{
	return CurrentPsiPower;
}

void ACharacterBB::PsiBlast()
{
	// The cost of the psi blast is 150.0f
	// Check we have atleast that before allowing the function to work
	if (CurrentPsiPower >= PsiBlastCost)
	{
		// Do the Psi Blast

		// Deduct the power used
		CurrentPsiPower -= PsiBlastCost;
	}
}

void ACharacterBB::AddKey(FString KeyToAdd)
{
	if (KeyWallet.Contains(KeyToAdd))
	{
		// Key already in there, play a noise
		OnKeyWalletAction.Broadcast(KeyToAdd, EPlayerKeyAction::AddKey, false);
	}
	else
	{
		KeyWallet.Add(KeyToAdd);
		// And maybe play a sound effect?
		OnKeyWalletAction.Broadcast(KeyToAdd, EPlayerKeyAction::AddKey, true);
	}
}

void ACharacterBB::RemoveKey(FString KeyToRemove)
{
	if (KeyWallet.Contains(KeyToRemove))
	{
		KeyWallet.Remove(KeyToRemove);
		OnKeyWalletAction.Broadcast(KeyToRemove, EPlayerKeyAction::RemoveKey, true);
	}
	else
	{
		OnKeyWalletAction.Broadcast(KeyToRemove, EPlayerKeyAction::RemoveKey, true);
	}
}

bool ACharacterBB::IsPlayerCarryingKey(FString DesiredKey)
{
	bool Result = KeyWallet.Contains(DesiredKey);
	OnKeyWalletAction.Broadcast(DesiredKey, EPlayerKeyAction::TestKey, Result);
	return Result;
}
