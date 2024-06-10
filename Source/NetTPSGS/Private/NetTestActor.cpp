// Fill out your copyright notice in the Description page of Project Settings.


#include "NetTestActor.h"
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/SphereComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/StaticMeshComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Public/EngineUtils.h>
#include "NetTPSGSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "NetGameInstance.h"

// Sets default values
ANetTestActor::ANetTestActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SetRootComponent(SphereComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

	bReplicates = true;

}

void ANetTestActor::BeginPlay()
{
	Super::BeginPlay();
	//NetUpdateFrequency = 100;

	Mat = MeshComp->CreateDynamicMaterialInstance(0);

	// 만약 내가 서버라면
	if (HasAuthority())
	{
		// 타이머를 실행하고싶다.
		FTimerHandle handle;
		auto gi = GetGameInstance<UNetGameInstance>();
		GetWorld()->GetTimerManager().SetTimer(handle, [&, gi]() {
			// 1초마다 MatColor를 랜덤으로 결정하고싶다.
			//MatColor = FLinearColor::MakeRandomColor();
			//OnRep_MatColor();
			if ( gi->IsInRoom() )
			{
				ServerRPC_ChangeColor(FLinearColor::MakeRandomColor());
			}
		}, 1, true);
	}
}

// Called every frame
void ANetTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PrintNetInfo();
	CheckOwner();
	DoRotationYaw();
}

void ANetTestActor::PrintNetInfo()
{
	// localRole
	FString localRole = UEnum::GetValueAsString(GetLocalRole());
	// remoteRole
	FString remoteRole = UEnum::GetValueAsString(GetRemoteRole());
	// owner
	FString owner = GetOwner() ? GetOwner()->GetName() : "";
	// netConn
	FString netConn = GetNetConnection() ? "Valid" : "Invalid";

	FString str = FString::Printf(TEXT("localRole : %s\nremoteRole : %s\nowner : %s\nnetConn : %s"), *localRole, *remoteRole, *owner, *netConn);

	FVector loc = GetActorLocation() + FVector(0, 0, 50);
	DrawDebugString(GetWorld(), loc, str, nullptr, FColor::White, 0, true);
}

void ANetTestActor::CheckOwner()
{
	// 만약 서버라면
	//if (GetLocalRole() == ROLE_Authority)
	if (HasAuthority())
	{
		float minDist = CheckDist;
		AActor* newOwner = nullptr;
		// 주변의 ANetTPSGSCharacter을 계속 검색하고싶다.
		for (TActorIterator<ANetTPSGSCharacter> it(GetWorld()); it; ++it)
		{
			AActor* otherActor = *it;
			// 나와의 거리를 재서 
			float tempDist = otherActor->GetDistanceTo(this);
			// 만약 minDist보다 가깝다면 기억하고
			if (tempDist < minDist)
			{
				// 가장 가까운 ANetTPSGSCharacter를 newOwner로 기억해서
				minDist = tempDist;
				newOwner = otherActor;
			}
		}
		// 만약 현재 내오너가 newOwner와 다르다면
		if (GetOwner() != newOwner) {
			// 나의 오너로 하고싶다.
			SetOwner(newOwner);
		}

	}
	DrawDebugSphere(GetWorld(), GetActorLocation(), CheckDist, 10, FColor::Yellow, 0, 0, 1);

}

void ANetTestActor::DoRotationYaw()
{
	float dt = GetWorld()->GetDeltaSeconds();
	if (HasAuthority())
	{
		// 서버
		AddActorLocalRotation(FRotator(0, 50 * dt, 0));
		RotYaw = GetActorRotation().Yaw;
	}
	else// 클라
	{
		// 시간이 흐르고 싶다.
		CurrentTime += dt;

		// 만약 LastTime이 0이라면 종료
		if (LastTime < KINDA_SMALL_NUMBER) {
			return;
		}

		float lerpRatio = CurrentTime / LastTime;
		// 현재시간, 끝시간
		float newYaw = RotYaw + 50 * LastTime;
		float lerpYaw = FMath::Lerp(RotYaw, newYaw, lerpRatio);
		FRotator rot = GetActorRotation();
		rot.Yaw = lerpYaw;
		SetActorRotation(rot);
	}
}
void ANetTestActor::OnRep_RotYaw()
{
	FRotator rot = GetActorRotation();
	rot.Yaw = RotYaw;
	SetActorRotation(rot);
	LastTime = CurrentTime;
	CurrentTime = 0;
}

void ANetTestActor::OnRep_MatColor()
{
	// 클라이언트에서 호출되는 함수다.
	// 만약 Mat 이있다면
	if (Mat)
	{
		// Mat의 색을 MatColor로 적용하고싶다.
		Mat->SetVectorParameterValue(TEXT("FloorColor"), MatColor);
	}
}

void ANetTestActor::ServerRPC_ChangeColor_Implementation(const FLinearColor newColor)
{
	MultiRPC_ChangeColor(newColor);
}

void ANetTestActor::ClientRPC_ChangeColor_Implementation(const FLinearColor newColor)
{
	if (Mat)
	{
		Mat->SetVectorParameterValue(TEXT("FloorColor"), newColor);
	}
}

void ANetTestActor::MultiRPC_ChangeColor_Implementation(const FLinearColor newColor)
{
	if (Mat)
	{
		Mat->SetVectorParameterValue(TEXT("FloorColor"), newColor);
	}
}

bool ANetTestActor::ServerRPC_ChangeColor_Validate(const FLinearColor newColor)
{
	return true;
}



void ANetTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{

	
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetTestActor, RotYaw);
	DOREPLIFETIME(ANetTestActor, MatColor);

	UE_LOG(LogTemp, Warning, TEXT("GetLifetimeReplicatedProps"));
}

