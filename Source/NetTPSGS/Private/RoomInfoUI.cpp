// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomInfoUI.h"
#include <../../../../../../../Source/Runtime/UMG/Public/Components/Button.h>

void URoomInfoUI::NativeConstruct()
{
	Button_JoinRoom->OnClicked.AddDynamic(this, &URoomInfoUI::OnMyClickJoinRoom);
}

void URoomInfoUI::OnMyClickJoinRoom()
{
	// GI의 JoinRoom을 호출하고싶다.
}

void URoomInfoUI::Setup(const FSessionInfo& info)
{
	// 방정보를 이용해서 UI를 구성하고싶다.
}
