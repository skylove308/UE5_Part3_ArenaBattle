// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/ABFountain.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "ArenaBattle.h"
#include "Components/PointLightComponent.h"
#include "EngineUtils.h"

// Sets default values
AABFountain::AABFountain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Water = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Water"));

	RootComponent = Body;
	Water->SetupAttachment(Body);
	Water->SetRelativeLocation(FVector(0.0f, 0.0f, 132.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Castle_Fountain_01.SM_Plains_Castle_Fountain_01'"));
	if (BodyMeshRef.Object)
	{
		Body->SetStaticMesh(BodyMeshRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> WaterMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Fountain_02.SM_Plains_Fountain_02'"));
	if (WaterMeshRef.Object)
	{
		Water->SetStaticMesh(WaterMeshRef.Object);
	}

	bReplicates = true;
	NetUpdateFrequency = 1.0f;
	NetCullDistanceSquared = 4000000.0f;
	// 최초 휴면상태
	// NetDormancy = DORM_Initial;
}

// Called when the game starts or when spawned
void AABFountain::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&]
			{
				//// float은 4바이트이기 때문에 4000바이트 데이터 전송
				//// 설정해놓은 3200바이트 대역폭을 넘어서기 때문에 Saturated로그가 찍힘
				//BigData.Init(BigDataElement, 1000);
				//// 전송 이후에 1씩 값 추가
				//BigDataElement += 1.0;
				//ServerLightColor = FLinearColor(FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), 1.0f);
				//OnRep_ServerLightColor();

				// MulticastRPC를 이용해 색상 바꾸기
				//const FLinearColor NewLightColor = FLinearColor(FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), 1.0f);
				//MulticastRPCChangeLightColor(NewLightColor);

				//const FLinearColor NewLightColor = FLinearColor(FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), 1.0f);
				//ClientRPCChangeLightColor(NewLightColor);
			}
		), 1.0f, true, 0.0f);

		//FTimerHandle Handle2;
		//GetWorld()->GetTimerManager().SetTimer(Handle2, FTimerDelegate::CreateLambda([&]
		//	{
		//		// 10초후에 휴면 상태가 해지되면서 변경된 속성이 반영
		//		// FlushNetDormancy();
		//		//for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		//		//{
		//		//	APlayerController* PlayerController = Iterator->Get();
		//		//	if (PlayerController && !PlayerController->IsLocalPlayerController())
		//		//	{
		//		//		SetOwner(PlayerController);
		//		//		break;
		//		//	}
		//		//}
		//		for (APlayerController* PlayerController : TActorRange<APlayerController>(GetWorld()))
		//		{
		//			if (PlayerController && !PlayerController->IsLocalPlayerController())
		//			{
		//				SetOwner(PlayerController);
		//				break;
		//			}
		//		}
		//	}), 10.0f, false, -1.0f);
	}
	else
	{
		// 클라이언트 입장에서는 GetFirstPlayerController()가 자신의 오너
		// 클라이언트는 서버에서의 프록시이기 때문에 별 의미가 없음
		// 네트워크를 진행할 때는 서버에 있는 액터의 오너가 설정되어야 함
		// 그래서 이 코드는 그냥 클라이언트의 로컬 머신에서만 호출되는 네트워크가 없는 로컬 함수
		// SetOwner(GetWorld()->GetFirstPlayerController());

		// ServerRPC는 클라이언트에서만 실행
		//FTimerHandle Handle;
		//GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&]
		//	{
		//		ServerRPCChangeLightColor();
		//	}
		//), 1.0f, true, 0.0f);
	}
}

// Called every frame
void AABFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		AddActorLocalRotation(FRotator(0.0f, RotationRate * DeltaTime, 0.0f));
		ServerRotationYaw = RootComponent->GetComponentRotation().Yaw;
	}
	else
	{
		// 서버로부터 패킷을 받은 이후의 시간들이 계속 누적
		ClientTimeSinceUpdate += DeltaTime;
		if (ClientTimeBetweenLastUpdate < KINDA_SMALL_NUMBER)
		{
			return;
		}

		const float EstimateRotationYaw = ServerRotationYaw + RotationRate * ClientTimeBetweenLastUpdate;
		const float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdate;

		FRotator ClientRotator = RootComponent->GetComponentRotation();
		const float ClientNewYaw = FMath::Lerp(ServerRotationYaw, EstimateRotationYaw, LerpRatio);
		ClientRotator.Yaw = ClientNewYaw;
		RootComponent->SetWorldRotation(ClientRotator);
	}

}

void AABFountain::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AABFountain, ServerRotationYaw); 
	//DOREPLIFETIME(AABFountain, BigData);
	DOREPLIFETIME(AABFountain, ServerLightColor);
}

//void AABFountain::OnActorChannelOpen(FInBunch& InBunch, UNetConnection* Connection)
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//
//	Super::OnActorChannelOpen(InBunch, Connection);
//
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
//}
//
//bool AABFountain::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
//{
//	bool NetRelevantResult = Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
//	if (!NetRelevantResult)
//	{
//		AB_LOG(LogABNetwork, Log, TEXT("Not Relevant:[%s] %s"), *RealViewer->GetName(), *SrcLocation.ToCompactString());
//	}
//	return NetRelevantResult;
//}
//
//void AABFountain::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//	Super::PreReplication(ChangedPropertyTracker);
//}

void AABFountain::OnRep_ServerRotationYaw()
{
	//AB_LOG(LogABNetwork, Log, TEXT("Yaw : %f"), ServerRotationYaw);

	FRotator NewRotator = RootComponent->GetComponentRotation();
	NewRotator.Yaw = ServerRotationYaw;
	RootComponent->SetWorldRotation(NewRotator);

	ClientTimeBetweenLastUpdate = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0.0f;
}

void AABFountain::OnRep_ServerLightColor()
{
	if (!HasAuthority())
	{
		AB_LOG(LogABNetwork, Log, TEXT("LightColor : %s"), *ServerLightColor.ToString());
	}

	UPointLightComponent* PointLight = Cast<UPointLightComponent>(GetComponentByClass(UPointLightComponent::StaticClass()));
	if (PointLight)
	{
		PointLight->SetLightColor(ServerLightColor);
	}
}

void AABFountain::ClientRPCChangeLightColor_Implementation(const FLinearColor& NewLightColor)
{
	AB_LOG(LogABNetwork, Log, TEXT("LightColor : %s"), *NewLightColor.ToString());

	UPointLightComponent* PointLight = Cast<UPointLightComponent>(GetComponentByClass(UPointLightComponent::StaticClass()));
	if (PointLight)
	{
		PointLight->SetLightColor(NewLightColor);
	}
}

// 검증이 실패한 경우 클라이언트는 즉시 차단되고 스탠드얼론 모드로 돌아가게됨
bool AABFountain::ServerRPCChangeLightColor_Validate()
{
	return true;
}

void AABFountain::ServerRPCChangeLightColor_Implementation()
{
	const FLinearColor NewLightColor = FLinearColor(FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), 1.0f);
	MulticastRPCChangeLightColor(NewLightColor);
}

void AABFountain::MulticastRPCChangeLightColor_Implementation(const FLinearColor& NewLightColor)
{
	AB_LOG(LogABNetwork, Log, TEXT("LightColor : %s"), *NewLightColor.ToString());

	UPointLightComponent* PointLight = Cast<UPointLightComponent>(GetComponentByClass(UPointLightComponent::StaticClass()));
	if (PointLight)
	{
		PointLight->SetLightColor(NewLightColor);
	}
}

