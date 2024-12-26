// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ABCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ABCharacterControlData.h"
#include "UI/ABHUDWidget.h"
#include "CharacterStat/ABCharacterStatComponent.h"
#include "Interface/ABGameInterface.h"
#include "ArenaBattle.h"
#include "Components/CapsuleComponent.h"
#include "Physics/ABCollision.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"
#include "EngineUtils.h"
#include "ABCharacterMovementComponent.h"

AABCharacterPlayer::AABCharacterPlayer(const FObjectInitializer& ObjectIjnitializer)
	: Super(ObjectIjnitializer.SetDefaultSubobjectClass<UABCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Input
	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionJumpRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Jump.IA_Jump'"));
	if (nullptr != InputActionJumpRef.Object)
	{
		JumpAction = InputActionJumpRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputChangeActionControlRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ChangeControl.IA_ChangeControl'"));
	if (nullptr != InputChangeActionControlRef.Object)
	{
		ChangeControlAction = InputChangeActionControlRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ShoulderMove.IA_ShoulderMove'"));
	if (nullptr != InputActionShoulderMoveRef.Object)
	{
		ShoulderMoveAction = InputActionShoulderMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderLookRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ShoulderLook.IA_ShoulderLook'"));
	if (nullptr != InputActionShoulderLookRef.Object)
	{
		ShoulderLookAction = InputActionShoulderLookRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionQuaterMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_QuaterMove.IA_QuaterMove'"));
	if (nullptr != InputActionQuaterMoveRef.Object)
	{
		QuaterMoveAction = InputActionQuaterMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionAttackRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Attack.IA_Attack'"));
	if (nullptr != InputActionAttackRef.Object)
	{
		AttackAction = InputActionAttackRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionTeleportRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Teleport.IA_Teleport'"));
	if (nullptr != InputActionTeleportRef.Object)
	{
		TeleportAction = InputActionTeleportRef.Object;
	}

	CurrentCharacterControlType = ECharacterControlType::Quater;
	bCanAttack = true;
}

void AABCharacterPlayer::BeginPlay()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	Super::BeginPlay(); 

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		EnableInput(PlayerController);
	}

	SetCharacterControl(CurrentCharacterControlType);
}

void AABCharacterPlayer::SetDead()
{
	Super::SetDead();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		DisableInput(PlayerController);
	}
}

void AABCharacterPlayer::PossessedBy(AController* NewController)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		AB_LOG(LogABNetwork, Log, TEXT("Owner : %s"), *OwnerActor->GetName());
	}
	else
	{
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No Owner"));
	}

	Super::PossessedBy(NewController);

	OwnerActor = GetOwner();
	if (OwnerActor)
	{
		AB_LOG(LogABNetwork, Log, TEXT("Owner : %s"), *OwnerActor->GetName());
	}
	else
	{
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No Owner"));
	}

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::OnRep_Owner()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s %S"), *GetName(), TEXT("Begin"));

	Super::OnRep_Owner();

	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		AB_LOG(LogABNetwork, Log, TEXT("Owner : %s"), *OwnerActor->GetName());
	}
	else
	{
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No Owner"));
	}

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::PostNetInit()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s %s"), TEXT("Begin"), *GetName());

	Super::PostNetInit();

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInputComponent->BindAction(ChangeControlAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ChangeCharacterControl);
	EnhancedInputComponent->BindAction(ShoulderMoveAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ShoulderMove);
	EnhancedInputComponent->BindAction(ShoulderLookAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ShoulderLook);
	EnhancedInputComponent->BindAction(QuaterMoveAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::QuaterMove);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::Attack);
	EnhancedInputComponent->BindAction(TeleportAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::Teleport);
}

void AABCharacterPlayer::ChangeCharacterControl()
{
	if (CurrentCharacterControlType == ECharacterControlType::Quater)
	{
		SetCharacterControl(ECharacterControlType::Shoulder);
	}
	else if (CurrentCharacterControlType == ECharacterControlType::Shoulder)
	{
		SetCharacterControl(ECharacterControlType::Quater);
	}
}

void AABCharacterPlayer::SetCharacterControl(ECharacterControlType NewCharacterControlType)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	UABCharacterControlData* NewCharacterControl = CharacterControlManager[NewCharacterControlType];
	check(NewCharacterControl);

	SetCharacterControlData(NewCharacterControl);

	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		UInputMappingContext* NewMappingContext = NewCharacterControl->InputMappingContext;
		if (NewMappingContext)
		{
			Subsystem->AddMappingContext(NewMappingContext, 0);
		}
	}

	CurrentCharacterControlType = NewCharacterControlType;
}

void AABCharacterPlayer::SetCharacterControlData(const UABCharacterControlData* CharacterControlData)
{
	Super::SetCharacterControlData(CharacterControlData);

	CameraBoom->TargetArmLength = CharacterControlData->TargetArmLength;
	CameraBoom->SetRelativeRotation(CharacterControlData->RelativeRotation);
	CameraBoom->bUsePawnControlRotation = CharacterControlData->bUsePawnControlRotation;
	CameraBoom->bInheritPitch = CharacterControlData->bInheritPitch;
	CameraBoom->bInheritYaw = CharacterControlData->bInheritYaw;
	CameraBoom->bInheritRoll = CharacterControlData->bInheritRoll;
	CameraBoom->bDoCollisionTest = CharacterControlData->bDoCollisionTest;
}

void AABCharacterPlayer::ShoulderMove(const FInputActionValue& Value)
{
	if (!bCanAttack)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void AABCharacterPlayer::ShoulderLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AABCharacterPlayer::QuaterMove(const FInputActionValue& Value)
{
	if (!bCanAttack)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	float InputSizeSquared = MovementVector.SquaredLength();
	float MovementVectorSize = 1.0f;
	float MovementVectorSizeSquared = MovementVector.SquaredLength();
	if (MovementVectorSizeSquared > 1.0f)
	{
		MovementVector.Normalize();
		MovementVectorSizeSquared = 1.0f;
	}
	else
	{
		MovementVectorSize = FMath::Sqrt(MovementVectorSizeSquared);
	}

	FVector MoveDirection = FVector(MovementVector.X, MovementVector.Y, 0.0f);
	GetController()->SetControlRotation(FRotationMatrix::MakeFromX(MoveDirection).Rotator());
	AddMovementInput(MoveDirection, MovementVectorSize);
}

void AABCharacterPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AABCharacterPlayer, bCanAttack);
}

void AABCharacterPlayer::Attack()
{
	// ProcessComboCommand();
	if (bCanAttack)
	{
		// 클라이언트에서 공격 명령이 들어오면 곧바로 공격 및 애니메이션 실행
		if (!HasAuthority())
		{
			bCanAttack = false;
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&]
				{
					bCanAttack = true;
					GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
				}), AttackTime, false, -1.0f);

			PlayAttackAnimation();
		}

		ServerRPCAttack(GetWorld()->GetGameState()->GetServerWorldTimeSeconds());
	}

}

void AABCharacterPlayer::PlayAttackAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->StopAllMontages(0.0f);
	AnimInstance->Montage_Play(ComboActionMontage);
}

void AABCharacterPlayer::AttackHitCheck()
{
	if (IsLocallyControlled())
	{
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

		FHitResult OutHitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

		const float AttackRange = Stat->GetTotalStat().AttackRange;
		const float AttackRadius = Stat->GetAttackRadius();
		const float AttackDamage = Stat->GetTotalStat().Attack;
		const FVector Forward = GetActorForwardVector();
		const FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector End = Start + GetActorForwardVector() * AttackRange;

		bool HitDetected = GetWorld()->SweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, CCHANNEL_ABACTION, FCollisionShape::MakeSphere(AttackRadius), Params);
		
		float HitCheckTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
		// 클라이언트
		if (!HasAuthority())
		{
			if (HitDetected)
			{
				// 서버로 검증을 보냄
				ServerRPCNotifyHit(OutHitResult, HitCheckTime);
			}
			else
			{
				ServerRPCNotifyMiss(Start, End, Forward, HitCheckTime);
			}
		}
		// 서버
		else
		{
			FColor DebugColor = HitDetected ? FColor::Green : FColor::Red;
			if (HitDetected)
			{
				AttackHitConfirm(OutHitResult.GetActor());
			}
		}


	}

}
void AABCharacterPlayer::AttackHitConfirm(AActor* HitActor)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	if (HasAuthority())
	{
		const float AttackDamage = Stat->GetTotalStat().Attack;
		FDamageEvent DamageEvent;
		HitActor->TakeDamage(AttackDamage, DamageEvent, GetController(), this);

	}
}

void AABCharacterPlayer::DrawDebugAttackRange(const FColor& DrawColor, FVector TraceStart, FVector TraceEnd, FVector Forward)
{
#if ENABLE_DRAW_DEBUG

	const float AttackRange = Stat->GetTotalStat().AttackRange;
	const float AttackRadius = Stat->GetAttackRadius();
	FVector CapsuleOrigin = TraceStart + (TraceEnd - TraceStart) * 0.5f;
	float CapsuleHalfHeight = AttackRange * 0.5f;

	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, AttackRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 5.0f);

#endif
}

void AABCharacterPlayer::ClientRPCPlayAnimation_Implementation(AABCharacterPlayer* CharacterToPlay)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	if (CharacterToPlay)
	{
		CharacterToPlay->PlayAttackAnimation();
	}
}

bool AABCharacterPlayer::ServerRPCNotifyHit_Validate(const FHitResult& HitResult, float HitCheckTime)
{
	return (HitCheckTime - LastAttackStartTime) > AcceptMinCheckTime;
}

void AABCharacterPlayer::ServerRPCNotifyHit_Implementation(const FHitResult& HitResult, float HitCheckTime)
{
	AActor* HitActor = HitResult.GetActor();
	if (::IsValid(HitActor))
	{
		const FVector HitLocation = HitResult.Location;
		const FBox HitBox = HitActor->GetComponentsBoundingBox();
		// Min, Max는 경계 상자의 각각 대각선 최소 꼭지점(3차원 좌표)
		// 즉, 절반이 중심점
		const FVector ActorBoxCenter = (HitBox.Min + HitBox.Max) * 0.5f;
		// 캐릭터의 중심점과 HitLocation의 거리가 지정해 준 거리보다 작다면 Hit판정
		if (FVector::DistSquared(HitLocation, ActorBoxCenter) <= AcceptCheckDistance * AcceptCheckDistance)
		{
			AttackHitConfirm(HitActor);
		}
		else
		{
			AB_LOG(LogABNetwork, Warning, TEXT("%s"), TEXT("HitTest Rejected!"));
		}

#if ENABLE_DRAW_DEBUG
		DrawDebugPoint(GetWorld(), ActorBoxCenter, 50.0f, FColor::Cyan, false, 5.0f);
		DrawDebugPoint(GetWorld(), HitLocation, 50.0f, FColor::Magenta, false, 5.0f);
#endif
		DrawDebugAttackRange(FColor::Green, HitResult.TraceStart, HitResult.TraceEnd, HitActor->GetActorForwardVector());
	}
}
bool AABCharacterPlayer::ServerRPCNotifyMiss_Validate(FVector_NetQuantize TraceStart, FVector_NetQuantize TraceEnd, FVector_NetQuantizeNormal TraceDir, float HitCheckTime)
{
	return (HitCheckTime - LastAttackStartTime) > AcceptMinCheckTime;
}
void AABCharacterPlayer::ServerRPCNotifyMiss_Implementation(FVector_NetQuantize TraceStart, FVector_NetQuantize TraceEnd, FVector_NetQuantizeNormal TraceDir, float HitCheckTime)
{
	DrawDebugAttackRange(FColor::Red, TraceStart, TraceEnd, TraceDir);
}

bool AABCharacterPlayer::ServerRPCAttack_Validate(float AttackStartTime)
{
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}
	return (AttackStartTime - LastAttackStartTime) > AttackTime;
}

void AABCharacterPlayer::ServerRPCAttack_Implementation(float AttackStartTime)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	bCanAttack = false;
	OnRep_CanAttack();
	
	// 서버에서 공격 시작 시간 - 클라이언트에서 공격 시작 시간
	AttackTimeDifference = GetWorld()->GetTimeSeconds() - AttackStartTime;
	AB_LOG(LogABNetwork, Log, TEXT("LagTime : %f"), TEXT("Begin"));
	// AttackTime을 모두 빼면 타이머가 동작할 수 없을 수도 있으니 0.01f만큼은 빼줌
	AttackTimeDifference = FMath::Clamp(AttackTimeDifference, 0.0f, AttackTime - 0.01f);

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&]
		{
			bCanAttack = true;
			OnRep_CanAttack();
		}), AttackTime - AttackTimeDifference, false, -1.0f);

	LastAttackStartTime = AttackStartTime;
	PlayAttackAnimation();

	//MulticastRPCAttack();
	//서버에 보낼 필요없기 때문에 multicast 대신 clientRPC를 사용해준다
	for (APlayerController* PlayerController : TActorRange<APlayerController>(GetWorld()))
	{
		// 현재 제어하는 플레이어 컨트롤러가 아닐때(서버 인경우)
		if (PlayerController && GetController() != PlayerController)
		{
			// 해당 컨트롤러가 로컬 플레이어의 컨트롤러가 아닐 때(네트워크 플레이어만 해당)
			if (!PlayerController->IsLocalController())
			{
				AABCharacterPlayer* OtherPlayer = Cast<AABCharacterPlayer>(PlayerController->GetPawn());
				if (OtherPlayer)
				{
					OtherPlayer->ClientRPCPlayAnimation(this);
				}
			}
		}
	}
}

void AABCharacterPlayer::MulticastRPCAttack_Implementation()
{
	//AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin")); 
	//// 공격 가능 여부는 서버에서만 판정
	//if (HasAuthority())
	//{
	//	bCanAttack = false;
	//	// 서버 로직의 경우 OnRep 함수는 명시적으로 호출 (클라이언트에서만 자동으로 호출됨)
	//	OnRep_CanAttack();

	//	FTimerHandle Handle;
	//	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&]
	//		{
	//			bCanAttack = true;
	//			OnRep_CanAttack();
	//		}), AttackTime, false, -1.0f);

	//}

	//// 애니매이션 재생은 서버 및 클라 모두에게서
	//UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	//AnimInstance->Montage_Play(ComboActionMontage);

	// 다른 클라이언트에서는 unreliable하게 애니메이션이 동작해도 괜찮음
	if (!IsLocallyControlled())
	{
		PlayAttackAnimation();
	}
}

void AABCharacterPlayer::OnRep_CanAttack()
{
	if (!bCanAttack)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
}

void AABCharacterPlayer::SetupHUDWidget(UABHUDWidget* InHUDWidget)
{
	if (InHUDWidget)
	{
		InHUDWidget->UpdateStat(Stat->GetBaseStat(), Stat->GetModifierStat());
		InHUDWidget->UpdateHpBar(Stat->GetCurrentHp());

		Stat->OnStatChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateStat);
		Stat->OnHpChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateHpBar);
	}
}

void AABCharacterPlayer::Teleport()
{
	AB_LOG(LogABTeleport, Log, TEXT("%s"), TEXT("Begin"));

	UABCharacterMovementComponent* ABMovement = Cast<UABCharacterMovementComponent>(GetCharacterMovement());
	if (ABMovement)
	{
		ABMovement->SetTeleportCommand();
	}
}
