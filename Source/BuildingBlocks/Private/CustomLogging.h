// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Logging/StructuredLog.h"

/* Custom log category so that related messages can be filtered */
DECLARE_LOG_CATEGORY_EXTERN(BBLog, Log, All);



























/* Macro to save typing the log name. (So lazy!) */
#define BBLOG(Verbosity, Format, ...) \
	UE_LOGFMT(BBLog, Verbosity, Format, ##__VA_ARGS__)
