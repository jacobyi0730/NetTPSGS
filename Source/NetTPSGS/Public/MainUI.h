// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainUI.generated.h"

/**
 * 
 */
UCLASS()
class NETTPSGS_API UMainUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	class UImage* ImageCrosshair;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	class UUniformGridPanel* BulletPanel;

	UPROPERTY(EditDefaultsOnly, Category = TPS)
	TSubclassOf<class UUserWidget> BulletFactory;
	
	// ---------------------
	void SetActiveCrosshair(bool value);

	// 태어날 때 총알을 세팅하고싶다.
	void InitBulletPanel(int max);
	// 총알을 추가하고싶다.
	void AddBullet();
	// 총알을 삭제하고싶다.
	void RemoveBullet();

};
