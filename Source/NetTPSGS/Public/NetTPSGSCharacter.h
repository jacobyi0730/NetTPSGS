﻿// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NetTPSGSCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ANetTPSGSCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:
	ANetTPSGSCharacter();
	
	virtual void Tick( float DeltaSeconds ) override;

	void PrintNetInfo();

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }



	// E키를 누르면 일정거리안에 총을 찾고 있으면 손에 붙이고싶다.
	// 입력처리
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_TakePistol;

	void OnIATakePistol(const FInputActionValue& value);

	// 총이 배치될 컴포넌트
	UPROPERTY(EditDefaultsOnly, Category = TSP)
	class USceneComponent* GunComp;

	UPROPERTY()
	TArray<AActor*> PistolList;

	// 총을 기억하고싶다.
	UPROPERTY(Replicated)
	bool bHasPistol;

	UPROPERTY()
	AActor* OwnedPistol;
	
	// 총을 검색하고싶다.
	UPROPERTY(EditDefaultsOnly, Category = TSP)
	float GunSearchDist = 200;

	void AttachPistol(AActor* pistol);
	void DetachPistol(AActor* pistol);

	void TakePistol();
	void ReleasePistol();

	// 총을 쏘고 싶다. 총을 쏜 위치에 VFX를 표현하고싶다.
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Fire;

	void OnIAFire(const FInputActionValue& value);

	UPROPERTY(EditDefaultsOnly, Category = TPS)
	class UParticleSystem* BulletImpactVFXFactory;

	UPROPERTY(EditDefaultsOnly, Category = TPS)
	class UAnimMontage* FireMongate;

	void PlayFireMontage();

	UPROPERTY(EditDefaultsOnly, Category = TPS)
	TSubclassOf<class UUserWidget> MainUIFactory;

	class UMainUI* MainUI;

	void InitMainUI();

	// 최대총알 갯수
	UPROPERTY(EditDefaultsOnly, Category = TPS)
	int32 MaxBulletCount = 10;
	// 현재총알 갯수
	int32 BulletCount = MaxBulletCount;


	// R 버튼을 누르면 재장전 하고싶다.
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Reload;

	void OnIAReload(const FInputActionValue& value);

	void OnMyReloadFinished();

	UPROPERTY(EditDefaultsOnly, Category = TPS)
	class UAnimMontage* ReloadMontage;


	// 체력을 만들고 관리하고싶다.
	// 최대체력, 현재체력, UI
	UPROPERTY(EditDefaultsOnly, Category = TPS)
	int32 MaxHP = 3;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = TPS)
	int32 _HP = MaxHP;

	__declspec(property(get = GetHP, put = SetHP)) int32 HP;
	int32 GetHP();
	void SetHP(int value);

	// 위젯 컴포넌트 만들어서 UI반영해서 보이게 하고싶다.
	UPROPERTY(EditDefaultsOnly, Category = TPS)
	class UWidgetComponent* HPComp;

	// 태어날 때 HPComp의 위젯을 가져와서 기억하고싶다.
	UPROPERTY()
	class UHPWidget* HpUI;

	void OnMyTakeDamage();

	bool bDie;

	bool bReloading;


	// 총잡기 s==============================================
	// E 버튼을 눌러 총을 잡을 때...
	// 1. 클라가 서버RPC를 호출
	// 2.1 서버의 RPC_함수가 호출
	// 2.2 멀티캐스트RPC를 호출해서 모든 클라가 총을 잡게 하고싶다.
	UFUNCTION(Server, Reliable)
	void ServerRPC_TakePistol();

	UFUNCTION(NetMulticast, Reliable)
	void MultiRPC_TakePistol(AActor* pistolActor);
	// 총잡기 e==============================================

	// 총놓기 s==============================================
	UFUNCTION(Server, Reliable)
	void ServerRPC_ReleasePistol();

	UFUNCTION(NetMulticast, Reliable)
	void MultiRPC_ReleasePistol(AActor* pistolActor);
	// 총잡기 e==============================================

	// 총쏘기 s==============================================
	UFUNCTION(Server, Reliable)
	void ServerRPC_Fire();

	UFUNCTION(NetMulticast, Reliable)
	void MultiRPC_Fire(int32 newBulletCount, bool bHit, FHitResult OutHit);
	// 총쏘기 e==============================================

	// 재장전 s==============================================
	UFUNCTION(Server, Reliable)
	void ServerRPC_Reload();

	UFUNCTION(Client, Reliable)
	void ClientRPC_Reload();

	void RefillAllRounds();
	// 재장전 e==============================================

	// 죽음처리
	void PrepareDie();

	// 죽었다.
	// 충돌처리 Off
	// 이동 Off
	// UI -> 리스폰




	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


};

