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

		sessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnJoinSessionComplete);

		sessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnMyExitRoomComplete);
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

	set.Set(FName("ROOM_NAME"), StringBase64Encode(roomName), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	set.Set(FName("HOST_NAME"), StringBase64Encode(mySessionName), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// 8. netID를 가져오고싶다.
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();

	UE_LOG(LogTemp, Warning, TEXT("CreateMySession : %s"), *roomName);
	// 방을 만들어 달라고 요청하고싶다.
	sessionInterface->CreateSession(*netID, FName(*roomName), set);
}

void UNetGameInstance::OnCreateSessionComplete(FName sessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete -> sessionName : %s, bWasSuccessful : %d"), *sessionName.ToString(), bWasSuccessful);

	// 만약 방생성을 성공했다면 ServerTravel 하고싶다.
	if ( bWasSuccessful )
	{
		GetWorld()->ServerTravel(TEXT("/Game/Net/Maps/GameMap?listen"));
	}
}

void UNetGameInstance::FindOtherSessions()
{
	// UI의 Finding...을 활성화 해달라...
	if ( OnMySessioinSearchFinishedDelegate.IsBound() ) 
	{
		OnMySessioinSearchFinishedDelegate.Broadcast(true);
	}

	// 세션인터페이스를 이용해서 방을 찾고싶다.
	sessioinSearch = MakeShareable(new FOnlineSessionSearch);

	// 검색 조건을 설정하고 싶다.
	sessioinSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Near);

	// LAN 여부
	sessioinSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";

	// 최대 검색 수 
	sessioinSearch->MaxSearchResults = 30;

	sessionInterface->FindSessions(0, sessioinSearch.ToSharedRef());
}

void UNetGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	// 만약 성공했다면
	if ( bWasSuccessful )
	{
		// sessioinSearch에서 정보를 가져오고싶다. -> UI로 표현하고싶다.
		auto results = sessioinSearch->SearchResults;
		if ( results.Num() == 0 ) {
			UE_LOG(LogTemp, Warning, TEXT("OnFindSessionsComplete results.Num() == 0"));
		}
		for ( int i = 0; i < results.Num(); i++ )
		{
			auto item = results[i];
			if ( false == item.IsValid() ) { continue; }

			FSessionInfo info;
			info.Set(i, item);

			FString roomName;
			FString hostName;
			item.Session.SessionSettings.Get(FName("ROOM_NAME"), roomName);
			item.Session.SessionSettings.Get(FName("HOST_NAME"), hostName);

			info.roomName = StringBase64Decode(roomName);
			info.hostName = StringBase64Decode(hostName);


			OnMySessionSearchCompleteDelegate.Broadcast(info);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *info.ToString());
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("OnFindSessionsComplete bWasSuccessful is false"));
		// UI의 Finding...을 활성화 해달라...
	}

	if ( OnMySessioinSearchFinishedDelegate.IsBound() )
	{
		OnMySessioinSearchFinishedDelegate.Broadcast(false);
	}
}

void UNetGameInstance::JoinMySession(int32 index)
{
	sessionInterface->JoinSession(0, FName(*mySessionName), sessioinSearch->SearchResults[index]);
}

void UNetGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// 호스트의 IP, Port번호를 가져와서 ClientTravel 하고싶다.
	if ( Result == EOnJoinSessionCompleteResult::Success )
	{
		auto* pc = GetWorld()->GetFirstPlayerController();
		FString url;
		sessionInterface->GetResolvedConnectString(SessionName, url);
		UE_LOG(LogTemp, Warning, TEXT("ClientTravel URL : %s"), *url);

		if ( false == url.IsEmpty() )
		{
			pc->ClientTravel(url, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UNetGameInstance::ExitRoom()
{
	ServerExitRoom();
}

void UNetGameInstance::ServerExitRoom_Implementation()
{
	MultiExitRoom();
}

void UNetGameInstance::MultiExitRoom_Implementation()
{
	sessionInterface->DestroySession(FName(*mySessionName));
}

void UNetGameInstance::OnMyExitRoomComplete(FName sessionName, bool bWasSuccessful)
{
	// 플레이어는 LobbyMap으로 여행을 떠나고싶다.
	auto* pc = GetWorld()->GetFirstPlayerController();
	FString url = TEXT("/Game/Net/Maps/LobbyMap");
	pc->ClientTravel(url, TRAVEL_Absolute);
}

bool UNetGameInstance::IsInRoom()
{
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();

	return sessionInterface->IsPlayerInSession(FName(*mySessionName), *netID);
}


FString UNetGameInstance::StringBase64Encode(const FString& str)
{
	// Set 할 때 : FString -> UTF8 (std::string) -> TArray<uint8> -> base64 로 Encode
	std::string utf8String = TCHAR_TO_UTF8(*str);
	TArray<uint8> arrayData = TArray<uint8>((uint8*)(utf8String.c_str()), utf8String.length());
	return FBase64::Encode(arrayData);
}

FString UNetGameInstance::StringBase64Decode(const FString& str)
{
	// Get 할 때 : base64 로 Decode -> TArray<uint8> -> TCHAR
	TArray<uint8> arrayData;
	FBase64::Decode(str, arrayData);
	std::string ut8String((char*)(arrayData.GetData()), arrayData.Num());
	return UTF8_TO_TCHAR(ut8String.c_str());
}
