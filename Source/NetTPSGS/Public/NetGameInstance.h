// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "NetGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString roomName;
	UPROPERTY(BlueprintReadOnly)
	FString hostName;
	UPROPERTY(BlueprintReadOnly)
	FString userName;
	UPROPERTY(BlueprintReadOnly)
	int32 maxPlayerCount;
	UPROPERTY(BlueprintReadOnly)
	int32 currentPlayerCount;
	UPROPERTY(BlueprintReadOnly)
	int32 pingMs;

	FORCEINLINE void Set(const FOnlineSessionSearchResult& item) {
		item.Session.SessionSettings.Get(FName("ROOM_NAME"), roomName);
		item.Session.SessionSettings.Get(FName("HOST_NAME"), hostName);
		// 방장의 이름
		userName = item.Session.OwningUserName;
		// 최대 플레이어 수
		maxPlayerCount = item.Session.SessionSettings.NumPublicConnections;
		// 현재 방에 들어온 플레이어 수
		currentPlayerCount = maxPlayerCount - item.Session.NumOpenPublicConnections;
		pingMs = item.PingInMs;
	}

	FORCEINLINE FString ToString() {
		return FString::Printf(TEXT("%s, %s, %s, (%d/%d), %dms"), *roomName, *hostName, *userName, currentPlayerCount, maxPlayerCount, pingMs);
	}
};



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

	// 방을 찾고 싶다.
	TSharedPtr<FOnlineSessionSearch> sessioinSearch;
	void FindOtherSessions();

	UFUNCTION()
	void OnFindSessionsComplete(bool bWasSuccessful);


};
