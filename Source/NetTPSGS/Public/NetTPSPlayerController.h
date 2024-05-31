// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NetTPSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class NETTPSGS_API ANetTPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UPROPERTY()
	class ANetTPSGSGameMode* GM;

	UPROPERTY()
	class UMainUI* MainUI;


	UFUNCTION(Server, Reliable)
	void ServerRPC_RespawnPlayer();

	UFUNCTION(Server, Reliable)
	void ServerRPC_ChangeSpectator();

};
