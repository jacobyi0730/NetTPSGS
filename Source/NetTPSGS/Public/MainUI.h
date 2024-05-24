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

	// �¾ �� �Ѿ��� �����ϰ�ʹ�.
	void InitBulletPanel(int max);
	// �Ѿ��� �߰��ϰ�ʹ�.
	void AddBullet();
	// �Ѿ��� �����ϰ�ʹ�.
	void RemoveBullet();

};
