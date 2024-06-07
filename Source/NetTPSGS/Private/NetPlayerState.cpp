// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerState.h"
#include "NetGameInstance.h"

void ANetPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// 만약 플레이어컨트롤러고 로컬플레이어라면
	auto* pc = GetPlayerController();
	if (pc && pc->IsLocalController())
	{
		// ServerRPC_SetPlayerName를 호출하고 싶다.
		auto* gi = Cast<UNetGameInstance>(GetWorld()->GetGameInstance());
		if ( gi )
		{
			ServerRPC_SetPlayerName(gi->mySessionName);
		}
	}
}

void ANetPlayerState::ServerRPC_SetPlayerName_Implementation(const FString& name)
{
	// 이름을 교체 하고싶다.
	SetPlayerName(name);
}

