// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NetPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class NETTPSGS_API UNetPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY()
	class ANetTPSGSCharacter* Me;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Net)
	bool bHasPistol = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Net)
	float Horizontal;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Net)
	float Vertical;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Net)
	float PitchAngle;

};
