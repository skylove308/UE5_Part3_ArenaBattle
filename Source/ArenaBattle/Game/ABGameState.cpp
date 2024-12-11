// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ABGameState.h"
#include "ArenaBattle.h"

// ���� ���� Ŭ����(Game Stat)���� ���� ���� ������ ó���ϱ� ���� ���Ǵ� �Լ�, ���� ������ ������ ���۵� �� ȣ��
void AABGameState::HandleBeginPlay()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::HandleBeginPlay();

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

// ���� �÷��� ���� ������ ���� ������ ��Ʈ��ũ Ŭ���̾�Ʈ�� ���޵� �� ȣ��Ǵ� �Լ�
// Ŭ���̾�Ʈ ������ Actor�� ���� �÷��̰� ���۵� ���¸� �ݿ��ϱ� ���� ȣ��
void AABGameState::OnRep_ReplicatedHasBegunPlay()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::OnRep_ReplicatedHasBegunPlay();

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}
