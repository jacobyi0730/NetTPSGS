// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HPWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETTPSGS_API UHPWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	class UProgressBar* HPBar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = TPS)
	float HP = 1.0f;

	void UpdateHPBar(float newHP);
	
};
