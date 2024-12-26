// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ABCharacterBase.h"
#include "InputActionValue.h"
#include "Interface/ABCharacterHUDInterface.h"
#include "ABCharacterPlayer.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABCharacterPlayer : public AABCharacterBase, public IABCharacterHUDInterface
{
	GENERATED_BODY()
	
public:
	AABCharacterPlayer(const FObjectInitializer& ObjectIjnitializer);

protected:
	virtual void BeginPlay() override;
	virtual void SetDead() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Owner() override;
	virtual void PostNetInit() override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

// Character Control Section
protected:
	void ChangeCharacterControl();
	void SetCharacterControl(ECharacterControlType NewCharacterControlType);
	virtual void SetCharacterControlData(const class UABCharacterControlData* CharacterControlData) override;

// Camera Section
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

// Input Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ChangeControlAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ShoulderMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ShoulderLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> QuaterMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> TeleportAction;

	void ShoulderMove(const FInputActionValue& Value);
	void ShoulderLook(const FInputActionValue& Value);

	void QuaterMove(const FInputActionValue& Value);

	ECharacterControlType CurrentCharacterControlType;

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void Attack();
	void PlayAttackAnimation();
	virtual void AttackHitCheck() override;
	void AttackHitConfirm(AActor* HitActor);
	void DrawDebugAttackRange(const FColor& DrawColor, FVector TraceStart, FVector TraceEnd, FVector Forward);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCAttack(float AttackStartTime);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCAttack();

	UFUNCTION(Client, Unreliable)
	void ClientRPCPlayAnimation(AABCharacterPlayer* CharacterToPlay);

	// 클라이언트에서 무언가 액터에 맞았을때 서버로 판정을 보내는 함수
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCNotifyHit(const FHitResult& HitResult, float HitCheckTime);

	// 미스시에 최적화를 위해 정보를 보냄
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCNotifyMiss(FVector_NetQuantize TraceStart, FVector_NetQuantize TraceEnd, FVector_NetQuantizeNormal TraceDir, float HitCheckTime);

	// 현재 공격 중인지 아닌지 판단해서 플래그를 걸어주는 용도
	UPROPERTY(ReplicatedUsing = OnRep_CanAttack)
	uint8 bCanAttack : 1;

	UFUNCTION()
	void OnRep_CanAttack();

	// 공격과 공격 사이 시간 간격
	float AttackTime = 1.4667;
	// 마지막에 공격한 시간
	float LastAttackStartTime = 0.0f;
	// 서버와 클라이언트 공격의 시간 차
	float AttackTimeDifference = 0.0f;
	// 둘 사이의 거리가 3m이하이면 문제 없도록 처리
	float AcceptCheckDistance = 300.0f;
	// 이 시간만큼은 지나고 판정 시작
	float AcceptMinCheckTime = 0.15f;

// UI Section
protected:
	virtual void SetupHUDWidget(class UABHUDWidget* InHUDWidget) override;

// Teleport Section
protected:
	void Teleport();
};
