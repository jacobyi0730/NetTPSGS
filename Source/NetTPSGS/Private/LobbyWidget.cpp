// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"
#include "NetGameInstance.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"

void ULobbyWidget::NativeConstruct()
{
	// gi을 채우고싶다.
	gi = Cast<UNetGameInstance>(GetWorld()->GetGameInstance());
	// 버튼을 연결하고싶다.
	Button_CreateRoom->OnClicked.AddDynamic(this, &ULobbyWidget::OnMyClickCreateRoom);

	Slider_PlayerCount->OnValueChanged.AddDynamic(this, &ULobbyWidget::OnMyValueChanged);

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
