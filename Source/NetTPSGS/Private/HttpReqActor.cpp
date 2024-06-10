// Fill out your copyright notice in the Description page of Project Settings.


#include "HttpReqActor.h"
#include "HttpUI.h"
#include <../../../../../../../Source/Runtime/Online/HTTP/Public/HttpModule.h>

// Sets default values
AHttpReqActor::AHttpReqActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHttpReqActor::BeginPlay()
{
	Super::BeginPlay();

	HttpUI = Cast<UHttpUI>(CreateWidget(GetWorld(), HttpUIFactory));
	HttpUI->AddToViewport();

	HttpUI->SetReqActor(this);
}

// Called every frame
void AHttpReqActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHttpReqActor::MyRequest(const FString url)
{
	FHttpModule& httpModule = FHttpModule::Get();
	TSharedPtr<IHttpRequest> req = httpModule.CreateRequest();

	req->SetURL(url);
	req->SetVerb(TEXT("GET"));
	req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	req->OnProcessRequestComplete().BindUObject(this, &AHttpReqActor::OnMyHttpRequestComplete);
	req->ProcessRequest();
}

void AHttpReqActor::OnMyHttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if ( bConnectedSuccessfully )
	{
		FString result = Response->GetContentAsString();
		HttpUI->SetLog(result);
	}
	else {
		HttpUI->SetLog(TEXT("Failed...Response"));
	}
}

void AHttpReqActor::MyRequestGetWebImage(const FString url)
{
	FHttpModule& httpModule = FHttpModule::Get();
	TSharedPtr<IHttpRequest> req = httpModule.CreateRequest();

	req->SetURL(url);
	req->SetVerb(TEXT("GET"));
	req->SetHeader(TEXT("Content-Type"), TEXT("image/jpeg"));

	req->OnProcessRequestComplete().BindUObject(this, &AHttpReqActor::OnMyHttpRequestWebImageComplete);

	req->ProcessRequest();
}

void AHttpReqActor::OnMyHttpRequestWebImageComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if ( bConnectedSuccessfully )
	{
		HttpUI->SetWebImage(Response->GetContent());
	}
	else {
		HttpUI->SetLog(TEXT("Failed...Response GetWebImage"));
	}
}

