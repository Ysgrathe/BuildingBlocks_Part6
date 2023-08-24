// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBB.generated.h"


// Delegate for when stats based on integers are changed.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FIntStatUpdated,
                                               int32, OldValue,
                                               int32, NewValue,
                                               int32, MaxValue);

// Delegate for when the player dies
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerIsDead);

// Delegate for when stats based on floats are changed.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFloatStatUpdated,
                                               float, OldValue,
                                               float, NewValue,
                                               float, MaxValue);

// Different actions involving the key wallet.
UENUM(BlueprintType)
enum class EPlayerKeyAction: uint8
{
	AddKey UMETA(Tooltip = "Attempt to add a key to player's wallet."),
	RemoveKey UMETA(Tooltip = "Attempt to remove a key from player's wallet."),
	TestKey UMETA(Tooltip = "Check if the player has a specific key.")
};

// Delegate for when actions occur with the player's keys.
// KeyString is the key involved in the action.
// KeyAction shows what was attempted.
// IsSuccess shows if the attempted action happened, or didn't.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FKeyWalletAction,
                                               FString, KeyString,
                                               EPlayerKeyAction, KeyAction,
                                               bool, IsSuccess);

UCLASS()
class BUILDINGBLOCKS_API ACharacterBB : public ACharacter
{
public:
	ACharacterBB();

	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;

	virtual void Jump() override;

	virtual void Crouch(bool bClientSimulation = false) override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// The normal walking speed of the character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement", meta = (AllowPrivateAccess = "true"))
	float NormalMaxWalkSpeed = 400.0f;

	// The sprinting speed of the character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement", meta = (AllowPrivateAccess = "true"))
	float RunningMaxWalkSpeed = 800.0f;

	UFUNCTION(BlueprintCallable, Category="Player|Movement")
	void SetRunning(bool IsRunning);

	UFUNCTION(BlueprintCallable, Category="Player|Movement")
	void ToggleRunning();

	// Called to set the flag indicating the player jumped since the last update.
	UFUNCTION(BlueprintCallable, Category="Player|Movement")
	void SetHasJumped();

	// Called to se the flag indicating the player sprinted since the last update.
	UFUNCTION(BlueprintCallable, Category="Player|Movement")
	void SetHasRan();

#pragma region Health

	// Return the player's current health.
	UFUNCTION(BlueprintPure, Category="Player|Health")
	int GetHealth();

	// Return the player's max health.
	UFUNCTION(BlueprintPure, Category="Player|Health")
	int GetMaxHealth();

	// Modify the player's health by the specified amount
	// -ve values are subtracted, +ve values added.
	UFUNCTION(BlueprintCallable, Category="Player|Health")
	void UpdateHealth(int DeltaHealth);

	// Sets player's current health to maximum allowable.
	UFUNCTION(BlueprintCallable, Category="Player|Health")
	void RestoreToFullHealth();

	// Sets the maximum allowable health.
	UFUNCTION(BlueprintCallable, Category="Player|Health")
	void SetMaxHealth(int NewMaxHealth);

	// Triggered when the player's health is updated.
	UPROPERTY(BlueprintAssignable, Category = "Player|Health")
	FIntStatUpdated OnHealthChanged;

	// Triggered when the player dies.
	UPROPERTY(BlueprintAssignable, Category = "Player|Health")
	FPlayerIsDead OnPlayerDied;

#pragma endregion

#pragma region Stamina

	// Return the player's current stamina.
	UFUNCTION(BlueprintPure, Category="Player|Stamina")
	float GetStamina();

	// Return the player's current recuperation factor.
	UFUNCTION(BlueprintPure, Category="Player|Stamina")
	float GetStaminaRecuperationFactor();

	UFUNCTION(BlueprintCallable, Category="Player|Stamina")
	void SetStaminaRecuperationFactor(float NewStaminaRecuperationFactor);

	// Triggered when the player's stamina is updated.
	UPROPERTY(BlueprintAssignable, Category = "Player|Stamina")
	FFloatStatUpdated OnStaminaChanged;

#pragma endregion

#pragma region Psi Power

	// Return the player's current psi power.
	UFUNCTION(BlueprintPure, Category="Player|PsiPower")
	float GetPsiPower();

	// Player unleashes a devastating blast of mind power!
	UFUNCTION(BlueprintCallable, Category="Player|PsiPower")
	void PsiBlast();

	// Triggered when the players psi power is updated.
	UPROPERTY(BlueprintAssignable, Category = "Player|PsiPower")
	FFloatStatUpdated OnPsiPowerChanged;

#pragma endregion

#pragma region Keys

	// Add a key to the wallet if it isn't already in there.
	// If it is already in there, dont do anything.
	UFUNCTION(BlueprintCallable, Category="Player|KeyWallet")
	void AddKey(FString KeyToAdd);

	// Remove a key (do we even need to do that in our game?)
	// If the key isn't in the wallet, we do nothing.
	UFUNCTION(BlueprintCallable, Category="Player|KeyWallet")
	void RemoveKey(FString KeyToRemove);

	// Does the player have a given key?
	// Returns true if they do, and false if they dont.
	UFUNCTION(BlueprintPure, Category="Player|KeyWallet")
	bool IsPlayerCarryingKey(FString DesiredKey);

	// Triggered when something happens with the player's key wallet.
	UPROPERTY(BlueprintAssignable, Category = "Player|KeyWallet")
	FKeyWalletAction OnKeyWalletAction;

#pragma endregion

protected:
	virtual void BeginPlay() override;

private:
	// is the character currently set to sprint?
	bool bIsRunning = false;

	// did the character sprint since the last update?
	bool bHasRan = false;

	// did the character jump since the last update?
	bool bHasJumped = false;

	// Health
	static constexpr int BaseStatValue = 100;
	int                  MaxHealth     = BaseStatValue;
	int                  CurrentHealth = BaseStatValue;

	// Stamina
	static constexpr float MaxStamina                = 100.0f;
	static constexpr float JumpStaminaCost           = 25.0f;
	static constexpr float RunStaminaCost            = 5.0f;
	static constexpr float RestStaminaRebate         = 4.0f;
	float                  CurrentStamina            = MaxStamina;
	float                  StaminaRecuperationFactor = 1.0f;

	// Psi Power
	static constexpr float MaxPsiPower     = 1000.0f;
	static constexpr float PsiRechargeRate = 1.0f;
	static constexpr float PsiBlastCost    = 150.0f;
	float                  CurrentPsiPower = MaxPsiPower;

	// Player Keys
	TArray<FString> KeyWallet;

	GENERATED_BODY()
};
