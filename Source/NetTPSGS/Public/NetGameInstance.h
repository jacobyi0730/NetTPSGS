// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetGameInstance.generated.h"

UCLASS()
class NETTPSGS_API UNetGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

	// 세션 인터페이스를 만들고싶다.
	IOnlineSessionPtr sessionInterface;

	// 방생성 요청 기능
	void CreateMySession(FString roomName, int32 playerCount);
	
	// 방생성 응답
	UFUNCTION()
	void OnCreateSessionComplete(FName sessionName, bool bWasSuccessful);

	FString mySessionName = TEXT("Jacobyi");

};
