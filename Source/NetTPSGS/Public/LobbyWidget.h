// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETTPSGS_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

	UPROPERTY()
	class UNetGameInstance* gi;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	class UEditableText* Edit_RoomName;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	class USlider* Slider_PlayerCount;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	class UTextBlock* Text_PlayerCount;

	// Button_CreateRoom을 누르면 GameInstance의 CreateMySession함수를 호출하고싶다.

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	class UButton* Button_CreateRoom;

	UFUNCTION()
	void OnMyClickCreateRoom();

	UFUNCTION()
	void OnMyValueChanged(float value);


};
