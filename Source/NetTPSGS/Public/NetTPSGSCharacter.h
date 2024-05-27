// Copyright Epic Games, Inc. All Rights Reserved.

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



	// EŰ�� ������ �����Ÿ��ȿ� ���� ã�� ������ �տ� ���̰�ʹ�.
	// �Է�ó��
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_TakePistol;

	void OnIATakePistol(const FInputActionValue& value);

	// ���� ��ġ�� ������Ʈ
	UPROPERTY(EditDefaultsOnly, Category = TSP)
	class USceneComponent* GunComp;

	UPROPERTY()
	TArray<AActor*> PistolList;

	// ���� ����ϰ�ʹ�.
	bool bHasPistol;

	UPROPERTY()
	AActor* OwnedPistol;
	
	// ���� �˻��ϰ�ʹ�.
	UPROPERTY(EditDefaultsOnly, Category = TSP)
	float GunSearchDist = 200;

	void AttachPistol(AActor* pistol);
	void DetachPistol(AActor* pistol);

	void TakePistol();
	void ReleasePistol();

	// ���� ��� �ʹ�. ���� �� ��ġ�� VFX�� ǥ���ϰ�ʹ�.
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

	// �ִ��Ѿ� ����
	UPROPERTY(EditDefaultsOnly, Category = TPS)
	int32 MaxBulletCount = 10;
	// �����Ѿ� ����
	int32 BulletCount = MaxBulletCount;


	// R ��ư�� ������ ������ �ϰ�ʹ�.
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Reload;

	void OnIAReload(const FInputActionValue& value);

	void OnMyReloadFinished();

	UPROPERTY(EditDefaultsOnly, Category = TPS)
	class UAnimMontage* ReloadMontage;


	// ü���� ����� �����ϰ�ʹ�.
	// �ִ�ü��, ����ü��, UI
	UPROPERTY(EditDefaultsOnly, Category = TPS)
	int32 MaxHP = 3;

	UPROPERTY(EditDefaultsOnly, Category = TPS)
	int32 _HP = MaxHP;

	__declspec(property(get = GetHP, put = SetHP)) int32 HP;
	int32 GetHP();
	void SetHP(int value);

	// ���� ������Ʈ ���� UI�ݿ��ؼ� ���̰� �ϰ�ʹ�.
	UPROPERTY(EditDefaultsOnly, Category = TPS)
	class UWidgetComponent* HPComp;

	// �¾ �� HPComp�� ������ �����ͼ� ����ϰ�ʹ�.
	UPROPERTY()
	class UHPWidget* HpUI;

	void OnMyTakeDamage();

	bool bDie;

	bool bReloading;

};

