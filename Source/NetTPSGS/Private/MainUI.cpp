// Fill out your copyright notice in the Description page of Project Settings.


#include "MainUI.h"
#include <../../../../../../../Source/Runtime/UMG/Public/Components/Image.h>
#include <../../../../../../../Source/Runtime/UMG/Public/Components/UniformGridPanel.h>

void UMainUI::SetActiveCrosshair(bool value)
{
	// ImageCrosshair�� Ȱ��/��Ȱ�� ó���ϰ�ʹ�.
	ImageCrosshair->SetVisibility(value ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	BulletPanel->SetVisibility(value ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UMainUI::InitBulletPanel(int max)
{
	MaxBullet = max;
	// �Ѿ� UI�� max ������ŭ ���� BulletPanel�� ���̰� �ʹ�.
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
