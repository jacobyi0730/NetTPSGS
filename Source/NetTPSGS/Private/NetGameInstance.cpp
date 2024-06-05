// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

void UNetGameInstance::Init()
{
	Super::Init();

	// 서브시스템에서 세션인터페이스 가져오고싶다.
	auto subsys = IOnlineSubsystem::Get();
	if ( subsys )
	{
		sessionInterface = subsys->GetSessionInterface();

		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreateSessionComplete);

		sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UNetGameInstance::OnFindSessionsComplete);
	}

	//FTimerHandle h;
	//GetWorld()->GetTimerManager().SetTimer(h, [&](){
	//	//CreateMySession(TEXT("MyRoom"), 20);
	//	FindOtherSessions();
	//}, 2, false);
}

void UNetGameInstance::CreateMySession(FString roomName, int32 playerCount)
{
	FOnlineSessionSettings set;

	// 1. 데디케이트 서버 접속인가?
	set.bIsDedicated = false;
	// 2. 랜선으로 매칭할것인가?
	FName subsysName = IOnlineSubsystem::Get()->GetSubsystemName();
	set.bIsLANMatch = subsysName == "NULL";
	// 3. 매칭을 공개할것인가? (<ㅡ>친구초대로만 할것인가?)
	set.bShouldAdvertise = true;
	// 4. Presence를 사용할것인가?
	set.bUsesPresence = true;
	// 5. 게임도중에 참여가능여부
	set.bAllowJoinInProgress = true;
	set.bAllowJoinViaPresence = true;
	// 6. 참여할 공개 연결의 최대 갯수
	set.NumPublicConnections = playerCount;
	// 7. 커스텀 정보 : 방이름, 호스트이름
	set.Set(FName("ROOM_NAME"), roomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	set.Set(FName("HOST_NAME"), mySessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// 8. netID를 가져오고싶다.
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();

	UE_LOG(LogTemp, Warning, TEXT("CreateMySession : %s"), *roomName);
	// 방을 만들어 달라고 요청하고싶다.
	sessionInterface->CreateSession(*netID, FName(*roomName), set);
}

void UNetGameInstance::OnCreateSessionComplete(FName sessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete -> sessionName : %s, bWasSuccessful : %d"), *sessionName.ToString(), bWasSuccessful);
}

void UNetGameInstance::FindOtherSessions()
{
	// 세션인터페이스를 이용해서 방을 찾고싶다.
	sessioinSearch = MakeShareable(new FOnlineSessionSearch);

	// 검색 조건을 설정하고 싶다.
	sessioinSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	// LAN 여부
	sessioinSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";

	// 최대 검색 수 
	sessioinSearch->MaxSearchResults = 10;

	sessionInterface->FindSessions(0, sessioinSearch.ToSharedRef());
}

void UNetGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	// 만약 성공했다면
	if ( bWasSuccessful )
	{
		// sessioinSearch에서 정보를 가져오고싶다. -> UI로 표현하고싶다.
		auto results = sessioinSearch->SearchResults;
		for ( auto item : results )
		{
			if ( false == item.IsValid() ) { continue; }

			FSessionInfo info;
			info.Set(item);

			UE_LOG(LogTemp, Warning, TEXT("%s"), *info.ToString());
		}
	}
}
