// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HttpUI.generated.h"

/**
 * 
 */
UCLASS()
class NETTPSGS_API UHttpUI : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY()
	class AHttpReqActor* ReqActor;

public:
	void SetReqActor(class AHttpReqActor* actor);

	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	class UEditableText* EditRow;
	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	class UEditableText* EditPage;
	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	class UTextBlock* TextLog;
	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	class UButton* ButtonRequest;
	
	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	class UButton* ButtonGetWebImage;
	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	class UImage* ImageWeb;

	UFUNCTION()
	void OnMyClickButtonRequest();

	void SetLog(FString newLog);

	UFUNCTION()
	void OnMyClickButtonGetWebImage();

	void SetWebImage(TArray<uint8> buf);


	FString webImageUrl = TEXT("https://png.pngtree.com/png-clipart/20240117/original/pngtree-cute-cat-png-image_14126564.png");


	FString url = TEXT("http://apis.data.go.kr/4050000/libnewbk/getLibnewbk");
	FString serviceKey = TEXT("xtHbWpFpd5Q8eg13ynvmWGNTHfXjZHZlCa%2B9%2F%2FsCC9AbMIBbA2FHhw%2BElWfKfq7uFEjS%2F21nuDn%2BvWl569cMnQ%3D%3D");
};
