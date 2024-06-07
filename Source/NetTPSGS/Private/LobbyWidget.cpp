// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"
#include "NetGameInstance.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include <../../../../../../../Source/Runtime/UMG/Public/Components/WidgetSwitcher.h>
#include "RoomInfoUI.h"
#include <../../../../../../../Source/Runtime/UMG/Public/Components/ScrollBox.h>

void ULobbyWidget::NativeConstruct()
{
	// gi을 채우고싶다.
	gi = Cast<UNetGameInstance>(GetWorld()->GetGameInstance());

	// gi의 OnMySessionSearchCompleteDelegate에 AddRoomInfoUI를 연결하고싶다.
	gi->OnMySessionSearchCompleteDelegate.AddDynamic(this, &ULobbyWidget::OnMyAddRoomInfoUI);
	gi->OnMySessioinSearchFinishedDelegate.AddDynamic(this, &ULobbyWidget::OnMySetActiveFindingText);


	// 버튼을 연결하고싶다.
	Button_CreateRoom->OnClicked.AddDynamic(this, &ULobbyWidget::OnMyClickCreateRoom);

	Slider_PlayerCount->SetValue(FCString::Atof(*Text_PlayerCount->GetText().ToString()));

	Slider_PlayerCount->OnValueChanged.AddDynamic(this, &ULobbyWidget::OnMyValueChanged);

	Button_GoCreateRoom->OnClicked.AddDynamic(this, &ULobbyWidget::OnMyClickGoCreateRoom);
	Button_GoFindRoom->OnClicked.AddDynamic(this, &ULobbyWidget::OnMyClickGoFindRoom);
	Button_Menu->OnClicked.AddDynamic(this, &ULobbyWidget::OnMyClickGoMenu);
	Button_Menu_1->OnClicked.AddDynamic(this, &ULobbyWidget::OnMyClickGoMenu);

	Button_FindRoom->OnClicked.AddDynamic(this, &ULobbyWidget::OnMyClickFindRoom);

	// 최초 시작시 메뉴 위젯을 활성화 하고싶다.
	SwitcherUI->SetActiveWidgetIndex(0);
}
void ULobbyWidget::OnMyClickGoMenu()
{
	SwitcherUI->SetActiveWidgetIndex(0);
}
bool ULobbyWidget::SetSessionName()
{
	// 해야할일!
	// badword 체크!
	FString str = Edit_SessionName->GetText().ToString();
	//str = str.TrimStartAndEnd();
	//str = str.Replace(TEXT("|"), TEXT(""));
	//str.Split()
	//str.Replace()
	// 입력한 것이 없다면
	if ( str.IsEmpty() ) {
		return false;
	}

#pragma region BadWords
	//TArray<FString> badWords;
	//badWords.Add("쓰|레|기");
	//badWords.Add("양말");

	//if ( badWords.Contains(str) )
	//{
	//	return;
	//}
#pragma endregion

	if ( gi )
	{
		gi->mySessionName = str;
		return true;
	}
	return false;
}
void ULobbyWidget::OnMyClickGoCreateRoom()
{
	if ( false == SetSessionName() )
		return;

	SwitcherUI->SetActiveWidgetIndex(1);
}

void ULobbyWidget::OnMyClickGoFindRoom()
{
	if ( false == SetSessionName() )
		return;

	SwitcherUI->SetActiveWidgetIndex(2);
	// 방찾기 메뉴 진입시 찾기를 시도하고싶다.
	if ( gi )
	{
		gi->FindOtherSessions();
	}
}
void ULobbyWidget::OnMyClickCreateRoom()
{
	if ( gi )
	{
		int32 count = Slider_PlayerCount->GetValue();
		gi->CreateMySession(Edit_RoomName->GetText().ToString(), count);
	}
}
void ULobbyWidget::OnMyValueChanged(float value)
{
	Text_PlayerCount->SetText(FText::AsNumber(value));
}

void ULobbyWidget::OnMyClickFindRoom()
{
	// 기존의 목록을 삭제하고싶다.
	ScrollBox_RoomList->ClearChildren();
	// gi의 FindOtherSessions를 호출하고싶다.
	gi->FindOtherSessions();
}

void ULobbyWidget::OnMyAddRoomInfoUI(const FSessionInfo& info)
{
	// RoomInfoUIFactory를 이용해서 위젯을 만들고
	auto ui = CreateWidget<URoomInfoUI>(this, RoomInfoUIFactory);
	// info를 Setup함수를 통해 전달하고싶다.
	ui->Setup(info);
	// 생성한 위젯을 ScrollBox_RoomList에 붙이고싶다.
	ScrollBox_RoomList->AddChild(ui);
}

void ULobbyWidget::OnMySetActiveFindingText(bool bSearching)
{
	TEXT_Finding->SetVisibility(bSearching ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	Button_FindRoom->SetIsEnabled(!bSearching);
}

