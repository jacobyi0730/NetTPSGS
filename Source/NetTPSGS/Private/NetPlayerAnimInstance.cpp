// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerAnimInstance.h"
#include "NetTPSGSCharacter.h"

void UNetPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 오너를 가져와서 Me에 채우고싶다.
	Me = Cast<ANetTPSGSCharacter>(TryGetPawnOwner());
}

void UNetPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (nullptr == Me)
		return;

	// Me의 이동속도를 Horizontal, Vertical로 분리해서 각 각 변수에 채우고싶다.

	// Me의 이동 속도를 가져오고싶다.
	auto vel = Me->GetVelocity();
	// Me의 이동 속도와 Me의 앞방향을 Dot 해서 Vertical에 대입하고싶다.
	Vertical = FVector::DotProduct(vel, Me->GetActorForwardVector());

	// Me의 이동 속도와 Me의 오른쪽방향을 Dot 해서 Horizontal에 대입하고싶다.
	Horizontal = FVector::DotProduct(vel, Me->GetActorRightVector());
}
