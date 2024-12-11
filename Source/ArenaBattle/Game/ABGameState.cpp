// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ABGameState.h"
#include "ArenaBattle.h"

// 게임 상태 클래스(Game Stat)에서 게임 시작 로직을 처리하기 위해 사용되는 함수, 서버 측에서 게임이 시작될 때 호출
void AABGameState::HandleBeginPlay()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::HandleBeginPlay();

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

// 게임 플레이 시작 상태의 변경 사항이 네트워크 클라이언트에 전달될 때 호출되는 함수
// 클라이언트 측에서 Actor의 게임 플레이가 시작된 상태를 반영하기 위해 호출
void AABGameState::OnRep_ReplicatedHasBegunPlay()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::OnRep_ReplicatedHasBegunPlay();

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}
