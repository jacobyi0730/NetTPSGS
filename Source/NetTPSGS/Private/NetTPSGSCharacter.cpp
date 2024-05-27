// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetTPSGSCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include <../../../../../../../Source/Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include "MainUI.h"
#include "NetPlayerAnimInstance.h"
#include <../../../../../../../Source/Runtime/UMG/Public/Components/WidgetComponent.h>
#include "HPWidget.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ANetTPSGSCharacter

ANetTPSGSCharacter::ANetTPSGSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0, 40, 60));
	CameraBoom->TargetArmLength = 150; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)


	GunComp = CreateDefaultSubobject<USceneComponent>(TEXT("GunComp"));
	GunComp->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));
	GunComp->SetRelativeLocation(FVector(-16.220860, 2.655643, 3.906385));
	GunComp->SetRelativeRotation(FRotator(17.292647, 82.407187, 7.526243));


	HPComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPComp"));
	HPComp->SetupAttachment(RootComponent);

	ConstructorHelpers::FClassFinder<UUserWidget> TempWidget(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Net/UI/WBP_HPWidget.WBP_HPWidget_C'"));

	if (TempWidget.Succeeded())
	{
		HPComp->SetWidgetClass(TempWidget.Class);
		HPComp->SetRelativeLocation(FVector(0, 0, 120));
		HPComp->SetDrawSize(FVector2D(150, 20));
	}
}

void ANetTPSGSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PrintNetInfo();
}

void ANetTPSGSCharacter::PrintNetInfo()
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

void ANetTPSGSCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// ������ ��� AActor�� �߿� Tag�� "Pistol"�� ���� ã�Ƽ� 
	//PistolList
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AActor::StaticClass(), TEXT("Pistol"), PistolList);

	InitMainUI();

	HpUI = Cast<UHPWidget>(HPComp->GetWidget());

	HP = MaxHP;
}


//////////////////////////////////////////////////////////////////////////
// Input

void ANetTPSGSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetTPSGSCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetTPSGSCharacter::Look);
		
		EnhancedInputComponent->BindAction(IA_TakePistol, ETriggerEvent::Started, this, &ANetTPSGSCharacter::OnIATakePistol);
		
		EnhancedInputComponent->BindAction(IA_Fire, ETriggerEvent::Started, this, &ANetTPSGSCharacter::OnIAFire);
		
		EnhancedInputComponent->BindAction(IA_Reload, ETriggerEvent::Started, this, &ANetTPSGSCharacter::OnIAReload);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANetTPSGSCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ANetTPSGSCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ANetTPSGSCharacter::OnIATakePistol(const FInputActionValue& value)
{
	if (bHasPistol)
	{
		// �̹� ���� ��� �ִ� ����
		// ����
		ReleasePistol();
	}
	else {
		// ����� �ִ� ����
		// ���
		TakePistol();
	}
	// ���� MainUI�� �ִٸ�
	if (MainUI)
	{
		//bool isGood = true;
		//int a = isGood == true ? 1 : 0;

		// ���� ����� �� ���ְ� �׷��������� ����ʹ�.
		MainUI->SetActiveCrosshair(bHasPistol);
	}
}

void ANetTPSGSCharacter::AttachPistol(AActor* pistol)
{
	UStaticMeshComponent* meshComp = pistol->GetComponentByClass<UStaticMeshComponent>();

	meshComp->SetSimulatePhysics(false);

	meshComp->AttachToComponent(GunComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void ANetTPSGSCharacter::TakePistol()
{
	// ���� ���� �̹� ����ִٸ� ����
	if (bHasPistol)
	{
		return;
	}

	for (auto pistol : PistolList)
	{
		// ���� pistol�� ���ʰ� ������ ��ŵ
		if (pistol->GetOwner() != nullptr)
			continue;
		// �Ÿ� �ٱ��̸�(GunSearchDist) ��ŵ
		float dist = pistol->GetDistanceTo(this);
		if (dist > GunSearchDist)
			continue;

		// �� pistol�� OwnedPistol�� �ϰ�
		OwnedPistol = pistol;
		// OwnedPistol�� ���ʸ� ���� �ϰ�
		OwnedPistol->SetOwner(this);
		// bHasPistol�� true �ϰ�ʹ�.
		bHasPistol = true;
		// �տ� ���̰�ʹ�.
		AttachPistol(pistol);
		// �ݺ����� �����ϰ�ʹ�.
		break;
	}
}

void ANetTPSGSCharacter::DetachPistol(AActor* pistol)
{
	// ����!!
	// �޽� ã��
	UStaticMeshComponent* meshComp = pistol->GetComponentByClass<UStaticMeshComponent>();
	if (meshComp)
	{
		// ���� �Ѱ�
		meshComp->SetSimulatePhysics(true);
		// ����ġ!!
		meshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	}

}

void ANetTPSGSCharacter::ReleasePistol()
{
	// ���� ���� �� ����ִٸ� ����
	// ������ ���̶�� ����
	if (false == bHasPistol || bReloading)
	{
		return;
	}
	// ���� ���� �����ϰ� �־��ٸ�
	if (OwnedPistol)
	{
		bHasPistol = false;
		OwnedPistol->SetOwner(nullptr);
		
		DetachPistol(OwnedPistol);
		
		OwnedPistol = nullptr;
	}
}

void ANetTPSGSCharacter::OnIAFire(const FInputActionValue& value)
{
	// ���� ���� ���� �ʴٸ� ����
	// ���� BulletCount�� 0���϶�� ����
	// ������ ���̶�� ����
	if (false == bHasPistol || BulletCount <= 0 || bReloading)
	{
		return;
	}

	BulletCount--;

	PlayFireMontage();

	// ���� MainUI�� ������ �Ѿ��� �ϳ��� �����ϰ�ʹ�.
	if (MainUI)
	{
		MainUI->RemoveBullet();
	}

	// ī�޶� ��ġ���� ī�޶� �չ������� LineTrace�� �ؼ� ���� ���� VFX�� ǥ���ϰ�ʹ�.
	FHitResult OutHit;
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + FollowCamera->GetForwardVector() * 100000;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletImpactVFXFactory, OutHit.ImpactPoint);

		// ������밡 �÷��̾���
		auto otherPlayer = Cast<ANetTPSGSCharacter>(OutHit.GetActor());
		if (otherPlayer)
		{
			otherPlayer->OnMyTakeDamage();
		}
	}
}

void ANetTPSGSCharacter::PlayFireMontage()
{
	PlayAnimMontage(FireMongate);
}

void ANetTPSGSCharacter::InitMainUI()
{
	if (MainUIFactory)
	{
		MainUI = Cast<UMainUI>(CreateWidget(GetWorld(), MainUIFactory));
		MainUI->AddToViewport();

		MainUI->SetActiveCrosshair(false);

		MainUI->InitBulletPanel(MaxBulletCount);
	}
}

void ANetTPSGSCharacter::OnIAReload(const FInputActionValue& value)
{
	// bReloading�� true��� ����
	if (bReloading || false == bHasPistol)
	{
		return;
	}
	bReloading = true;

	// ���ε� ��Ÿ�� �ִϸ��̼��� �÷����ϰ�ʹ�.
	auto* anim = Cast<UNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (anim)
	{
		anim->Montage_Play(ReloadMontage);
	}
}

void ANetTPSGSCharacter::OnMyReloadFinished()
{
	// ���� �ٽ� �ִ� ������ �� ä���ʹ�.
	bReloading = false;
	if (MainUI)
	{
		MainUI->RemoveAllBullets();
		MainUI->InitBulletPanel(MaxBulletCount);
		BulletCount = MaxBulletCount;
	}
}

int32 ANetTPSGSCharacter::GetHP()
{
	return _HP;
}

void ANetTPSGSCharacter::SetHP(int value)
{
	_HP = value;
	// UI�� �ݿ�
	HpUI->UpdateHPBar((float)_HP / MaxHP);
}

void ANetTPSGSCharacter::OnMyTakeDamage()
{
	HP = HP - 1;
	// ���� HP�� 0���ϸ� ����ó��
	if (HP <= 0)
	{
		bDie = true;
	}
}



