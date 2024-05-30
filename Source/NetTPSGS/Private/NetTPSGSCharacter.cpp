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
#include "Net/UnrealNetwork.h"
#include <../../../../../../../Source/Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>

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

	// UI에 반영
	if ( MainUI )	{
		MainUI->UpdateHPBar((float)_HP / MaxHP);
	}
	else if ( HpUI )	{
		HpUI->UpdateHPBar((float)_HP / MaxHP);
	}

	// HPComp를 빌보드 처리하고싶다.
	TObjectPtr<APlayerCameraManager> camMgr = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;

	// UI가 카메라를 향하는 방향으로 회전하고싶다.
	FVector direction = camMgr->GetCameraLocation() - HPComp->GetComponentLocation();
	FRotator rot = UKismetMathLibrary::MakeRotFromX(direction);
	HPComp->SetWorldRotation(rot);
	
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

	// 레벨의 모든 AActor들 중에 Tag가 "Pistol"인 것을 찾아서 
	//PistolList
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AActor::StaticClass(), TEXT("Pistol"), PistolList);

	InitMainUI();


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
		// 이미 총을 잡고 있던 상태
		// 놓기
		ReleasePistol();
	}
	else {
		// 안잡고 있던 상태
		// 잡기
		TakePistol();
	}
}

void ANetTPSGSCharacter::AttachPistol(AActor* pistol)
{
	UStaticMeshComponent* meshComp = pistol->GetComponentByClass<UStaticMeshComponent>();

	meshComp->SetSimulatePhysics(false);

	meshComp->AttachToComponent(GunComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// 만약 MainUI가 있다면
	if (IsLocallyControlled() && MainUI)
	{
		// 총을 들었을 때 켜주고 그렇지않으면 끄고싶다.
		MainUI->SetActiveCrosshair(true);
	}

}

void ANetTPSGSCharacter::TakePistol()
{
	// 만약 총을 이미 잡고있다면 종료
	if (bHasPistol)
	{
		return;
	}
	ServerRPC_TakePistol();
}

void ANetTPSGSCharacter::DetachPistol(AActor* pistol)
{
	// 구현!!
	// 메시 찾고
	UStaticMeshComponent* meshComp = pistol->GetComponentByClass<UStaticMeshComponent>();
	if (meshComp)
	{
		// 물리 켜고
		meshComp->SetSimulatePhysics(true);
		// 디테치!!
		meshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	}

}

void ANetTPSGSCharacter::ReleasePistol()
{
	// 만약 총을 안 잡고있다면 종료
	// 재장전 중이라면 종료
	if (false == bHasPistol || bReloading)
	{
		return;
	}

	ServerRPC_ReleasePistol();	// 클라가 서버에게 총을 놓아주세요. 요청
}

void ANetTPSGSCharacter::OnIAFire(const FInputActionValue& value)
{
	// 총을 집고 있지 않다면 종료
	// 만약 BulletCount가 0이하라면 종료
	// 재장전 중이라면 종료
	if (false == bHasPistol || BulletCount <= 0 || bReloading)
	{
		return;
	}
	ServerRPC_Fire();
}

void ANetTPSGSCharacter::PlayFireMontage()
{
	PlayAnimMontage(FireMongate);
}

void ANetTPSGSCharacter::InitMainUI()
{
	if (IsLocallyControlled() && MainUIFactory)
	{
		MainUI = Cast<UMainUI>(CreateWidget(GetWorld(), MainUIFactory));
		MainUI->AddToViewport();

		MainUI->SetActiveCrosshair(false);

		MainUI->InitBulletPanel(MaxBulletCount);
	}
	if ( HPComp )
	{
		HpUI = Cast<UHPWidget>(HPComp->GetWidget());
		// 만약 내가 로컬이면 HPComp를 안보이게 하고싶다.
		if ( IsLocallyControlled() )
		{
			HPComp->SetVisibility(false);
		}
	}
}

void ANetTPSGSCharacter::OnIAReload(const FInputActionValue& value)
{
	// bReloading이 true라면 종료
	if (bReloading || false == bHasPistol)
	{
		return;
	}
	bReloading = true;

	// 리로드 몽타주 애니메이션을 플레이하고싶다.
	auto* anim = Cast<UNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (anim)
	{
		anim->Montage_Play(ReloadMontage);
	}
}

void ANetTPSGSCharacter::OnMyReloadFinished()
{
	// 총을 다시 최대 갯수로 꽉 채우고싶다.
	ServerRPC_Reload();
}

int32 ANetTPSGSCharacter::GetHP()
{
	return _HP;
}

// Mulicast를 통해서 모든 클라에게 반영되고있다.
void ANetTPSGSCharacter::SetHP(int value)
{
	_HP = value;
}

void ANetTPSGSCharacter::OnMyTakeDamage()
{
	HP = HP - 1;
	if ( MainUI )
	{
		MainUI->PlayDamageAnimation();
	}
	// 만약 HP가 0이하면 죽음처리
	if (HP <= 0)
	{
		bDie = true;
	}
}



void ANetTPSGSCharacter::ServerRPC_TakePistol_Implementation()
{
	for (auto pistol : PistolList)
	{
		// 만약 pistol의 오너가 있으면 스킵
		if (pistol->GetOwner() != nullptr)
			continue;
		// 거리 바깥이면(GunSearchDist) 스킵
		float dist = pistol->GetDistanceTo(this);
		if (dist > GunSearchDist)
			continue;

		// 그 pistol을 OwnedPistol로 하고
		OwnedPistol = pistol;
		// OwnedPistol의 오너를 나로 하고
		OwnedPistol->SetOwner(this);
		// bHasPistol을 true 하고싶다.
		bHasPistol = true;

		MultiRPC_TakePistol(pistol);
		// 반복문을 종료하고싶다.
		break;
	}
}

void ANetTPSGSCharacter::MultiRPC_TakePistol_Implementation(AActor* pistolActor)
{
	// 손에 붙이고싶다.
	AttachPistol(pistolActor);
}

void ANetTPSGSCharacter::ServerRPC_ReleasePistol_Implementation()
{
	// 만약 총을 소유하고 있었다면
	if (OwnedPistol)
	{
		MultiRPC_ReleasePistol(OwnedPistol);

		bHasPistol = false;
		OwnedPistol->SetOwner(nullptr);
		OwnedPistol = nullptr;
	}
}

void ANetTPSGSCharacter::MultiRPC_ReleasePistol_Implementation(AActor* pistolActor)
{
	DetachPistol(pistolActor);

	// UI를 안보이게 하고싶다.
	if (IsLocallyControlled() && MainUI)
	{
		MainUI->SetActiveCrosshair(false);
	}
}

void ANetTPSGSCharacter::ServerRPC_Fire_Implementation()
{
	BulletCount--;

	// 카메라 위치에서 카메라 앞방향으로 LineTrace를 해서 닿은 곳에 VFX를 표현하고싶다.
	FHitResult OutHit;
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + FollowCamera->GetForwardVector() * 100000;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);

	MultiRPC_Fire(BulletCount, bHit, OutHit);
}

void ANetTPSGSCharacter::MultiRPC_Fire_Implementation(int32 newBulletCount, bool bHit, FHitResult OutHit)
{
	BulletCount = newBulletCount;

	PlayFireMontage();

	// 만약 MainUI가 있으면 총알을 하나씩 제거하고싶다.
	if (IsLocallyControlled() && MainUI)
	{
		MainUI->RemoveBullet();
	}
	if (bHit)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletImpactVFXFactory, OutHit.ImpactPoint);

		// 맞은상대가 플레이어라면
		auto otherPlayer = Cast<ANetTPSGSCharacter>(OutHit.GetActor());
		if (otherPlayer)
		{
			otherPlayer->OnMyTakeDamage();
		}
	}
}

// 여기는 서버에서 호출됨
void ANetTPSGSCharacter::ServerRPC_Reload_Implementation()
{
	// 기능적으로 총알을 꽉 채우고싶다.
	RefillAllRounds();
	ClientRPC_Reload();
}

// 여기는 클라이언트에서 호출됨
void ANetTPSGSCharacter::ClientRPC_Reload_Implementation()
{
	bReloading = false;
	RefillAllRounds();
	if ( MainUI )
	{
		MainUI->RemoveAllBullets();
		MainUI->InitBulletPanel(MaxBulletCount);
	}
}

void ANetTPSGSCharacter::RefillAllRounds()
{
	BulletCount = MaxBulletCount;
}

void ANetTPSGSCharacter::PrepareDie()
{
	if ( IsLocallyControlled() )
	{
		FollowCamera->PostProcessSettings.ColorSaturation = FVector4(0, 0, 0, 1);
	}
}

void ANetTPSGSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetTPSGSCharacter, bHasPistol);
	DOREPLIFETIME(ANetTPSGSCharacter, _HP);
}



