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
	// 총알을 전부 삭제하고싶다.
	void RemoveAllBullets();
	int32 MaxBullet;

	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	class UProgressBar* HPBar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = TPS)
	float HP = 1.0f;

	void UpdateHPBar(float newHP);

	UPROPERTY(EditDefaultsOnly, meta=(BindWidgetAnim), Transient)
	class UWidgetAnimation* DamageAnimation;

	void PlayDamageAnimation();

	// 게임오버 변수 생성 기능 추가
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	class UCanvasPanel* GameOverUI;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	class UButton* ButtonRespawn;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	class UButton* ButtonQuit;

	UFUNCTION()
	void OnMyButtonRespawn();

	UFUNCTION()
	void OnMyButtonQuit();

	void SetActiveGameOverUI(bool value);

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	class UTextBlock* Text_UserList;
};
