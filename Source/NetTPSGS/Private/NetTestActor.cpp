// Fill out your copyright notice in the Description page of Project Settings.


#include "NetTestActor.h"
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/SphereComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/StaticMeshComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Public/EngineUtils.h>
#include "NetTPSGSCharacter.h"
#include "Net/UnrealNetwork.h"

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

	// ���� ���� �������
	if (HasAuthority())
	{
		// Ÿ�̸Ӹ� �����ϰ�ʹ�.
		FTimerHandle handle;
		GetWorld()->GetTimerManager().SetTimer(handle, [&]() {
			// 1�ʸ��� MatColor�� �������� �����ϰ�ʹ�.
			//MatColor = FLinearColor::MakeRandomColor();
			//OnRep_MatColor();
			ServerRPC_ChangeColor(FLinearColor::MakeRandomColor());
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
	// ���� �������
	//if (GetLocalRole() == ROLE_Authority)
	if (HasAuthority())
	{
		float minDist = CheckDist;
		AActor* newOwner = nullptr;
		// �ֺ��� ANetTPSGSCharacter�� ��� �˻��ϰ�ʹ�.
		for (TActorIterator<ANetTPSGSCharacter> it(GetWorld()); it; ++it)
		{
			AActor* otherActor = *it;
			// ������ �Ÿ��� �缭 
			float tempDist = otherActor->GetDistanceTo(this);
			// ���� minDist���� �����ٸ� ����ϰ�
			if (tempDist < minDist)
			{
				// ���� ����� ANetTPSGSCharacter�� newOwner�� ����ؼ�
				minDist = tempDist;
				newOwner = otherActor;
			}
		}
		// ���� ���� �����ʰ� newOwner�� �ٸ��ٸ�
		if (GetOwner() != newOwner) {
			// ���� ���ʷ� �ϰ�ʹ�.
			SetOwner(newOwner);
		}

	}
	DrawDebugSphere(GetWorld(), GetActorLocation(), CheckDist, 30, FColor::Yellow, 0, 0, 1);

}

void ANetTestActor::DoRotationYaw()
{
	float dt = GetWorld()->GetDeltaSeconds();
	if (HasAuthority())
	{
		// ����
		AddActorLocalRotation(FRotator(0, 50 * dt, 0));
		RotYaw = GetActorRotation().Yaw;
	}
	else// Ŭ��
	{
		// �ð��� �帣�� �ʹ�.
		CurrentTime += dt;

		// ���� LastTime�� 0�̶�� ����
		if (LastTime < KINDA_SMALL_NUMBER) {
			return;
		}

		float lerpRatio = CurrentTime / LastTime;
		// ����ð�, ���ð�
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
	// Ŭ���̾�Ʈ���� ȣ��Ǵ� �Լ���.
	// ���� Mat ���ִٸ�
	if (Mat)
	{
		// Mat�� ���� MatColor�� �����ϰ�ʹ�.
		Mat->SetVectorParameterValue(TEXT("FloorColor"), MatColor);
	}
}

void ANetTestActor::ServerRPC_ChangeColor_Implementation(const FLinearColor newColor)
{
	// Mat�� ���� MatColor�� �����ϰ�ʹ�.
	if (Mat)
	{
		Mat->SetVectorParameterValue(TEXT("FloorColor"), newColor);
	}
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

