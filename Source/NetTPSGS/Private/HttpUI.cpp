// Fill out your copyright notice in the Description page of Project Settings.


#include "HttpUI.h"
#include <../../../../../../../Source/Runtime/UMG/Public/Components/Button.h>
#include "HttpReqActor.h"
#include <../../../../../../../Source/Runtime/UMG/Public/Components/EditableText.h>
#include <../../../../../../../Source/Runtime/UMG/Public/Components/TextBlock.h>
#include <../../../../../../../Source/Runtime/Engine/Public/ImageUtils.h>
#include <../../../../../../../Source/Runtime/UMG/Public/Components/Image.h>

void UHttpUI::SetReqActor(AHttpReqActor* actor)
{
	ReqActor = actor;
}

void UHttpUI::NativeConstruct()
{
	Super::NativeConstruct();

	ButtonRequest->OnClicked.AddDynamic(this, &UHttpUI::OnMyClickButtonRequest);
	ButtonGetWebImage->OnClicked.AddDynamic(this, &UHttpUI::OnMyClickButtonGetWebImage);
}

void UHttpUI::OnMyClickButtonRequest()
{
	if ( ReqActor )
	{
		int32 pageNo = FCString::Atof(*EditPage->GetText().ToString());;
		int32 numOfRows = FCString::Atof(*EditRow->GetText().ToString());

		FString fullURL = FString::Printf(TEXT("%s?serviceKey=%s&pageNo=%d&numOfRows=%d"), *url, *serviceKey, pageNo, numOfRows);

		ReqActor->MyRequest(fullURL);
	}
}

void UHttpUI::SetLog(FString newLog)
{
	TextLog->SetText(FText::FromString(newLog));
}

void UHttpUI::OnMyClickButtonGetWebImage()
{	
	if ( ReqActor )
	{
		ReqActor->MyRequestGetWebImage(webImageUrl);
	}
}

void UHttpUI::SetWebImage(TArray<uint8> buf)
{
	FString imagePath = FPaths::ProjectPersistentDownloadDir() + "/MyGetImage.jpg";
	FFileHelper::SaveArrayToFile(buf, *imagePath);
	UTexture2D* realTex = FImageUtils::ImportBufferAsTexture2D(buf);

	ImageWeb->SetBrushFromTexture(realTex);
}
