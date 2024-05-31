// Fill out your copyright notice in the Description page of Project Settings.


#include "NetTPSPlayerController.h"
#include "NetTPSGSGameMode.h"
#include <../../../../../../../Source/Runtime/Engine/Classes/GameFramework/SpectatorPawn.h>

void ANetTPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GM = Cast<ANetTPSGSGameMode>(GetWorld()->GetAuthGameMode());
	}
}

void ANetTPSPlayerController::ServerRPC_ChangeSpectator_Implementation()
{
	// 현재 주인공을 기억하고싶다.
	auto* oldPawn = GetPawn();
	// 만약 oldPawn이 있다면
	if (oldPawn)
	{
		// 관전자를 만들어서 
		FTransform t = oldPawn->GetActorTransform();
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		auto* newPawn = GetWorld()->SpawnActor<ASpectatorPawn>(GM->SpectatorClass, t, params);

		// 빙의 하고싶다.
		Possess(newPawn);
		// oldPawn은 파괴하고싶다.
		oldPawn->Destroy();
		// 5초후에 ServerRPC_RespawnPlayer_Implementation를 호출하고싶다.
		FTimerHandle handle;
		GetWorld()->GetTimerManager().SetTimer(handle, this, &ANetTPSPlayerController::ServerRPC_RespawnPlayer_Implementation, 5, false);
	}
}

void ANetTPSPlayerController::ServerRPC_RespawnPlayer_Implementation()
{
	// 현재 주인공을 기억하고싶다.
	auto* oldPawn = GetPawn();
	
	// 빙의를 해제 하고싶다.
	UnPossess();

	// 기억했던 주인공을 파괴 하고싶다.
	if ( oldPawn )
	{
		oldPawn->Destroy();
	}
	
	// GameMode의 재시작 함수를 호출하고싶다.
	GM->RestartPlayer(this);
}
