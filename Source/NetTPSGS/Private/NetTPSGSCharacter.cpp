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

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ANetTPSGSCharacter

ANetTPSGSCharacter::ANetTPSGSCharacter()
{
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
	// 만약 MainUI가 있다면
	if (MainUI)
	{
		//bool isGood = true;
		//int a = isGood == true ? 1 : 0;

		// 총을 들었을 때 켜주고 그렇지않으면 끄고싶다.
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
	// 만약 총을 이미 잡고있다면 종료
	if (bHasPistol)
	{
		return;
	}

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
		// 손에 붙이고싶다.
		AttachPistol(pistol);
		// 반복문을 종료하고싶다.
		break;
	}
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
	if (false == bHasPistol)
	{
		return;
	}
	// 만약 총을 소유하고 있었다면
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
	// 총을 집고 있지 않다면 종료
	if (false == bHasPistol)
	{
		return;
	}

	// 만약 BulletCount가 0이하라면 종료
	if (BulletCount <= 0)
	{
		return;
	}
	BulletCount--;

	PlayFireMontage();

	// 만약 MainUI가 있으면 총알을 하나씩 제거하고싶다.
	if (MainUI)
	{
		MainUI->RemoveBullet();
	}

	// 카메라 위치에서 카메라 앞방향으로 LineTrace를 해서 닿은 곳에 VFX를 표현하고싶다.
	FHitResult OutHit;
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + FollowCamera->GetForwardVector() * 100000;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletImpactVFXFactory, OutHit.ImpactPoint);
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



