// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomInfoUI.h"
#include <../../../../../../../Source/Runtime/UMG/Public/Components/Button.h>
#include <../../../../../../../Source/Runtime/UMG/Public/Components/TextBlock.h>
#include "NetGameInstance.h"

void URoomInfoUI::NativeConstruct()
{
	Button_JoinRoom->OnClicked.AddDynamic(this, &URoomInfoUI::OnMyClickJoinRoom);
}

void URoomInfoUI::OnMyClickJoinRoom()
{
	// GI의 JoinMyRoom을 호출하고싶다.
	Cast<UNetGameInstance>(GetWorld()->GetGameInstance())->JoinMySession(index);	
}

void URoomInfoUI::Setup(const FSessionInfo& info)
{
	// 방정보를 이용해서 UI를 구성하고싶다.
	index = info.index;
	Text_RoomName->SetText(FText::FromString(info.roomName));
	Text_HostName->SetText(FText::FromString(info.hostName));
	//Text_UserName->SetText(FText::FromString(info.userName));
	Text_UserName->SetText(FText::GetEmpty());
	
	FString count = FString::Printf(TEXT("(%d/%d)"), info.currentPlayerCount, info.maxPlayerCount);
	Text_PlayerCount->SetText(FText::FromString(count));
	
	FString ping = FString::Printf(TEXT("%dms"), info.pingMs);
	Text_PingMs->SetText(FText::FromString(ping));
}
