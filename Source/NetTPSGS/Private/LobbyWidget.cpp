// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"
#include "NetGameInstance.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include <../../../../../../../Source/Runtime/UMG/Public/Components/WidgetSwitcher.h>

void ULobbyWidget::NativeConstruct()
{
	// gi을 채우고싶다.
	gi = Cast<UNetGameInstance>(GetWorld()->GetGameInstance());
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
void ULobbyWidget::OnMyClickGoCreateRoom()
{
	SwitcherUI->SetActiveWidgetIndex(1);
}
void ULobbyWidget::OnMyClickGoFindRoom()
{
	SwitcherUI->SetActiveWidgetIndex(2);
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
}

void ULobbyWidget::AddRoomInfoUI(const FSessionInfo& info)
{
}

