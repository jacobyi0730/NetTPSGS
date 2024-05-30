// Fill out your copyright notice in the Description page of Project Settings.


#include "MainUI.h"
#include <../../../../../../../Source/Runtime/UMG/Public/Components/Image.h>
#include <../../../../../../../Source/Runtime/UMG/Public/Components/UniformGridPanel.h>

void UMainUI::SetActiveCrosshair(bool value)
{
	// ImageCrosshair를 활성/비활성 처리하고싶다.
	ImageCrosshair->SetVisibility(value ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	BulletPanel->SetVisibility(value ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UMainUI::InitBulletPanel(int max)
{
	MaxBullet = max;
	// 총알 UI를 max 갯수만큼 만들어서 BulletPanel에 붙이고 싶다.
	for (int i = 0; i < max; i++)
	{
		AddBullet();
	}
}

void UMainUI::AddBullet()
{
	auto bullet = CreateWidget(this, BulletFactory);
	BulletPanel->AddChildToUniformGrid(bullet, 0, BulletPanel->GetChildrenCount());
}

void UMainUI::RemoveBullet()
{
	if (BulletPanel->GetChildrenCount() <= 0)
	{
		return;
	}

	BulletPanel->RemoveChildAt(BulletPanel->GetChildrenCount() - 1);
}

void UMainUI::RemoveAllBullets()
{
	for (int i = 0 ; i < MaxBullet && BulletPanel->GetChildrenCount() ; i++)
	{
		BulletPanel->RemoveChildAt(0);
	}
}

void UMainUI::UpdateHPBar(float newHP)
{
	HP = newHP;
}

void UMainUI::PlayDamageAnimation()
{
	PlayAnimation(DamageAnimation, 0, 1);
}
