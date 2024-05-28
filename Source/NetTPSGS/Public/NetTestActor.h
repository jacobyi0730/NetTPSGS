// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetTestActor.generated.h"

UCLASS()
class NETTPSGS_API ANetTestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANetTestActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void PrintNetInfo();

	// �޽ø� ���� ���̰�ʹ�.
	UPROPERTY(EditDefaultsOnly, Category = TPS)
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = TPS)
	class USphereComponent* SphereComp;

	float CheckDist = 200;
	void CheckOwner();

	// �������� ȸ���� �ϰ� �� ȸ������ ����ϰ�ʹ�.
	// Ŭ�󿡼� �� ȸ������ �ݿ��ϰ�ʹ�.
	UPROPERTY(ReplicatedUsing=OnRep_RotYaw)
	float RotYaw = 0;

	UFUNCTION()
	void OnRep_RotYaw();

	void DoRotationYaw();

	float CurrentTime = 0;
	float LastTime = 0;


	// ������ ���� �����ϰ�ʹ�.
	UPROPERTY()
	class UMaterialInstanceDynamic* Mat;

	UPROPERTY(ReplicatedUsing=OnRep_MatColor)
	FLinearColor MatColor;

	UFUNCTION()
	void OnRep_MatColor();

	// RPC (Remote Procedure Call)
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPC_ChangeColor(const FLinearColor newColor);

	UFUNCTION(Client, Reliable)
	void ClientRPC_ChangeColor(const FLinearColor newColor);

	UFUNCTION(NetMulticast, Reliable)
	void MultiRPC_ChangeColor(const FLinearColor newColor);


//	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
