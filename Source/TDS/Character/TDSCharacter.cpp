// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TDS/Game/TDSGameInstance.h"
#include "TDS/Weapons/Projectiles/ProjectileDefault.h"

ATDSCharacter::ATDSCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	InventoryComponent = CreateDefaultSubobject<UTDSInventoryComponent>(TEXT("InventoryComponent"));
	HealthComponent = CreateDefaultSubobject<UTDSCharacterHealthComponent>(TEXT("HealthComponent"));

	if (HealthComponent)
	{
		HealthComponent->OnDead.AddDynamic(this, &ATDSCharacter::CharacterDead);
	}

	if (InventoryComponent)
	{
		InventoryComponent->OnSwitchWeapon.AddDynamic(this, &ATDSCharacter::InitWeapon);
	}

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ATDSCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);


	if (CurrentCursor)
	{
		APlayerController* myPC = Cast<APlayerController>(GetController());
		if (myPC)
		{
			FHitResult TraceHitResult;
			myPC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();

			CurrentCursor->SetWorldLocation(TraceHitResult.Location);
			CurrentCursor->SetWorldRotation(CursorR);
		}
	}

	MovementTick(DeltaSeconds);
}

void ATDSCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CursorMaterial)
	{
		CurrentCursor = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorMaterial, CursorSize, FVector(0));
	}
}

void ATDSCharacter::SetupPlayerInputComponent(UInputComponent* NewInputComponent)
{
	Super::SetupPlayerInputComponent(NewInputComponent);

	NewInputComponent->BindAxis(TEXT("MoveForward"), this, &ATDSCharacter::InputAxisX);
	NewInputComponent->BindAxis(TEXT("MoveRight"), this, &ATDSCharacter::InputAxisY);

	NewInputComponent->BindAction(TEXT("ChangeToSprint"), EInputEvent::IE_Pressed, this, &ATDSCharacter::InputSprintPressed);
	NewInputComponent->BindAction(TEXT("ChangeToWalk"), EInputEvent::IE_Pressed, this, &ATDSCharacter::InputWalkPressed);
	NewInputComponent->BindAction(TEXT("AimEvent"), EInputEvent::IE_Pressed, this, &ATDSCharacter::InputAimPressed);
	NewInputComponent->BindAction(TEXT("ChangeToSprint"), EInputEvent::IE_Released, this, &ATDSCharacter::InputSprintReleased);
	NewInputComponent->BindAction(TEXT("ChangeToWalk"), EInputEvent::IE_Released, this, &ATDSCharacter::InputWalkReleased);
	NewInputComponent->BindAction(TEXT("AimEvent"), EInputEvent::IE_Released, this, &ATDSCharacter::InputAimReleased);


	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Pressed, this, &ATDSCharacter::InputAttackPressed);
	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Released, this, &ATDSCharacter::InputAttackReleased);
	NewInputComponent->BindAction(TEXT("ReloadEvent"), EInputEvent::IE_Released, this, &ATDSCharacter::TryReloadWeapon);

	NewInputComponent->BindAction(TEXT("SwitchNextWeapon"), EInputEvent::IE_Pressed, this, &ATDSCharacter::TrySwitchNextWeapon);
	NewInputComponent->BindAction(TEXT("SwitchPreviosWeapon"), EInputEvent::IE_Pressed, this, &ATDSCharacter::TrySwitchPreviosWeapon);

	NewInputComponent->BindAction(TEXT("AbilityAction"), EInputEvent::IE_Pressed, this, &ATDSCharacter::TryAbilityEnabled);

	NewInputComponent->BindAction(TEXT("DropCurrentWeapon"), EInputEvent::IE_Pressed, this, &ATDSCharacter::DropCurrenWeapon);

	TArray<FKey> HotKeys;
	HotKeys.Add(EKeys::One);
	HotKeys.Add(EKeys::Two);
	HotKeys.Add(EKeys::Three);
	HotKeys.Add(EKeys::Four);
	HotKeys.Add(EKeys::Five);
	HotKeys.Add(EKeys::Six);
	HotKeys.Add(EKeys::Seven);
	HotKeys.Add(EKeys::Eight);
	HotKeys.Add(EKeys::Nine);
	HotKeys.Add(EKeys::Zero);

	NewInputComponent->BindKey(HotKeys[1], IE_Pressed, this, &ATDSCharacter::TKeyPressed<1>);
	NewInputComponent->BindKey(HotKeys[1], IE_Pressed, this, &ATDSCharacter::TKeyPressed<2>);
	NewInputComponent->BindKey(HotKeys[1], IE_Pressed, this, &ATDSCharacter::TKeyPressed<3>);
	NewInputComponent->BindKey(HotKeys[1], IE_Pressed, this, &ATDSCharacter::TKeyPressed<4>);
	NewInputComponent->BindKey(HotKeys[1], IE_Pressed, this, &ATDSCharacter::TKeyPressed<5>);
	NewInputComponent->BindKey(HotKeys[1], IE_Pressed, this, &ATDSCharacter::TKeyPressed<6>);
	NewInputComponent->BindKey(HotKeys[1], IE_Pressed, this, &ATDSCharacter::TKeyPressed<7>);
	NewInputComponent->BindKey(HotKeys[1], IE_Pressed, this, &ATDSCharacter::TKeyPressed<8>);
	NewInputComponent->BindKey(HotKeys[1], IE_Pressed, this, &ATDSCharacter::TKeyPressed<9>);
	NewInputComponent->BindKey(HotKeys[1], IE_Pressed, this, &ATDSCharacter::TKeyPressed<0>);
}

void ATDSCharacter::InputAxisX(float Value)
{
	AxisX = Value;
}

void ATDSCharacter::InputAxisY(float Value)
{
	AxisY = Value;
}

void ATDSCharacter::InputAttackPressed()
{
	if (bIsAlive)
	{
		AttackCharEvent(true);
	}
	
}

void ATDSCharacter::InputAttackReleased()
{
	AttackCharEvent(false);
}

void ATDSCharacter::InputWalkPressed()
{
	WalkEnabled = true;
	ChangeMovementState();
}

void ATDSCharacter::InputWalkReleased()
{
	WalkEnabled = false;
	ChangeMovementState();
}

void ATDSCharacter::InputSprintPressed()
{
	SprintRunEnabled = true;
	ChangeMovementState();
}

void ATDSCharacter::InputSprintReleased()
{
	SprintRunEnabled = false;
	ChangeMovementState();
}

void ATDSCharacter::InputAimPressed()
{
	AimEnabled = true;
	ChangeMovementState();
}

void ATDSCharacter::InputAimReleased()
{
	AimEnabled = false;
	ChangeMovementState();
}

void ATDSCharacter::MovementTick(float DeltaTime)
{
	if (bIsAlive)
	{
		AddMovementInput(FVector(1.0f, 0.0f, 0.0f), AxisX);
		AddMovementInput(FVector(0.0f, 1.0f, 0.0f), AxisY);

		if (MovementState == EMovementState::Sprint_State)
		{
			FVector myRotation = FVector(AxisX, AxisY, 0.0f);
			FRotator myRotator = myRotation.ToOrientationRotator();
			SetActorRotation(FQuat(myRotator));
		}
		else
		{
			APlayerController* myController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			if (myController)
			{
				FHitResult ResultHit;
				myController->GetHitResultUnderCursor(ECC_GameTraceChannel1, true, ResultHit);
				float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;
				SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f)));

				if (CurrentWeapon)
				{
					FVector Displacement = FVector(0);
					switch (MovementState)
					{
					case EMovementState::Aim_State:
						Displacement = FVector(0.0f, 0.0f, 160.0f);
						CurrentWeapon->ShouldReduceDispersion = true;
						break;
					case EMovementState::AimWalk_State:
						Displacement = FVector(0.0f, 0.0f, 160.0f);
						CurrentWeapon->ShouldReduceDispersion = true;
						break;
					case EMovementState::Walk_State:
						Displacement = FVector(0.0f, 0.0f, 120.0f);
						CurrentWeapon->ShouldReduceDispersion = false;
						break;
					case EMovementState::Run_State:
						Displacement = FVector(0.0f, 0.0f, 120.0f);
						CurrentWeapon->ShouldReduceDispersion = false;
						break;
					case EMovementState::Sprint_State:
						break;
					default:
						break;
					}

					CurrentWeapon->ShootEndLocation = ResultHit.Location + Displacement;
				}
			}
		}
	}

}


EMovementState ATDSCharacter::GetMovementState()
{
	return MovementState;
}

TArray<UTDS_StateEffect*> ATDSCharacter::GetCurrentEffectsOnChar()
{
	return Effects;
}

int32 ATDSCharacter::GetCurrentWeaponIndex()
{
	return CurrentIndexWeapon;
}

void ATDSCharacter::AttackCharEvent(bool bIsFiring)
{
	AWeaponDefault* myWeapon = nullptr;
	myWeapon = GetCurrentWeapon();
	if (myWeapon)
	{
		myWeapon->SetWeaponStateFire(bIsFiring);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ATDSCharacter::AttackCharEvent - CurrentWeapon -NULL"));
	}
}

void ATDSCharacter::CharacterUpdate()
{
	float ResSpeed = 600.0f;
	switch (MovementState)
	{
	case EMovementState::Aim_State:
		ResSpeed = MovementInfo.AimSpeed;
		break;
	case EMovementState::Walk_State:
		ResSpeed = MovementInfo.WalkSpeed;
		break;
	case EMovementState::Run_State:
		ResSpeed = MovementInfo.RunSpeed;
		break;
	case EMovementState::AimWalk_State:
		ResSpeed = MovementInfo.AimWalkSpeed;
		break;
	case EMovementState::Sprint_State:
		ResSpeed = MovementInfo.SprintSpeed;
		break;
	default:
		break;
	}

	GetCharacterMovement()->MaxWalkSpeed = ResSpeed;
}

void ATDSCharacter::ChangeMovementState()
{
	if (!SprintRunEnabled && !WalkEnabled && !AimEnabled)
	{
		MovementState = EMovementState::Run_State;
	}
	else
	{
		if (SprintRunEnabled)
		{
			MovementState = EMovementState::Sprint_State;
			WalkEnabled = false;
			AimEnabled = false;
		}
		if (!SprintRunEnabled && WalkEnabled && AimEnabled)
		{
			MovementState = EMovementState::AimWalk_State;
		}
		else
		{
			if (!SprintRunEnabled && WalkEnabled && !AimEnabled)
			{
				MovementState = EMovementState::Walk_State;
			}
			else
			{
				if (!SprintRunEnabled && !WalkEnabled && AimEnabled)
				{
					MovementState = EMovementState::Aim_State;
				}
			}
		}
	}

	CharacterUpdate();

	//weapon state
	AWeaponDefault* myWeapon = GetCurrentWeapon();
	if (myWeapon)
	{
		myWeapon->UpdateStateWeapon(MovementState);
	}
}

AWeaponDefault* ATDSCharacter::GetCurrentWeapon()
{
	return CurrentWeapon;
}

void ATDSCharacter::InitWeapon(FName IdWeaponName, FAdditionalWeaponInfo WeaponAdditionalInfo, int32 NewCurrentIndexWeapon)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}
	UTDSGameInstance* myGI = Cast<UTDSGameInstance>(GetGameInstance());
	FWeaponInfo myWeaponInfo;
	if (myGI)
	{
		if (myGI->GetWeaponInfoByName(IdWeaponName, myWeaponInfo))
		{
			if (myWeaponInfo.WeaponClass)
			{
				FVector SpawnLocation = FVector(0);
				FRotator SpawnRoratation = FRotator(0);

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = GetInstigator();

				AWeaponDefault* myWeapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(myWeaponInfo.WeaponClass, &SpawnLocation, &SpawnRoratation, SpawnParams));
				if (myWeapon)
				{
					FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					myWeapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
					CurrentWeapon = myWeapon;
					myWeapon->WeaponSetting = myWeaponInfo;

					myWeapon->ReloadTimeDebug = myWeaponInfo.ReloadTime;
					myWeapon->UpdateStateWeapon(MovementState);

					myWeapon->AdditionalWeaponInfo = WeaponAdditionalInfo;
					CurrentIndexWeapon = NewCurrentIndexWeapon;

					myWeapon->OnWeaponReloadStart.AddDynamic(this, &ATDSCharacter::WeaponReloadStart);
					myWeapon->OnWeaponReloadEnd.AddDynamic(this, &ATDSCharacter::WeaponReloadEnd);
					myWeapon->OnWeaponFire.AddDynamic(this, &ATDSCharacter::WeaponFire);

					if (CurrentWeapon->GetWeaponRound() <= 0 && CurrentWeapon->CheckCanWeaponReload())
					{
						CurrentWeapon->InitReload();
					}
					if (InventoryComponent)
					{
						InventoryComponent->OnWeaponAmmoAviable.Broadcast(myWeapon->WeaponSetting.WeaponType);
					}
				}
			}
		}
		else 
		{
			UE_LOG(LogTemp, Warning, TEXT("ATDSCharacter::InitWeapon - Weapon not found in table -NULL"));
		}
		
	}
}


void ATDSCharacter::TryReloadWeapon()
{
	if (bIsAlive && CurrentWeapon && !CurrentWeapon->WeaponReloading)
	{
		if (CurrentWeapon->GetWeaponRound() <= CurrentWeapon->WeaponSetting.MaxRound && CurrentWeapon->CheckCanWeaponReload())
		{
			CurrentWeapon->InitReload();
		}
	}
}

void ATDSCharacter::WeaponReloadStart(UAnimMontage* Anim)
{
	WeaponReloadStart_BP(Anim);
}

void ATDSCharacter::WeaponReloadEnd(bool bIsSuccess, int32 AmmoTake)
{
	if (InventoryComponent && CurrentWeapon)
	{
		InventoryComponent->AmmoSlotChangeValue(CurrentWeapon->WeaponSetting.WeaponType, AmmoTake);
		InventoryComponent->SetAdditionalInfoWeapon(CurrentIndexWeapon, CurrentWeapon->AdditionalWeaponInfo);
	}

	WeaponReloadEnd_BP(bIsSuccess);
}

bool ATDSCharacter::TrySwitchWeaponToIndexByKeyInput(int32 ToIndex)
{
	bool bIsSucces = true;
	if (CurrentWeapon && !CurrentWeapon->WeaponReloading && InventoryComponent->WeaponSlots.IsValidIndex(ToIndex))
	{
		if (CurrentIndexWeapon != ToIndex && InventoryComponent)
		{
			int32 OldIndex = CurrentIndexWeapon;
			FAdditionalWeaponInfo OldInfo;

			if (CurrentWeapon)
			{
				OldInfo = CurrentWeapon->AdditionalWeaponInfo;
				if (CurrentWeapon->WeaponReloading)
				{
					CurrentWeapon->CancelReload();
				}
			}
			
			bIsSucces = InventoryComponent->SwitchWeaponByIndex(ToIndex, OldIndex, OldInfo);
		}
	}
	return bIsSucces;
}

void ATDSCharacter::DropCurrenWeapon()
{
	if (InventoryComponent)
	{
		FDropItem ItemInfo;
		InventoryComponent->DropWeaponByIndex(CurrentIndexWeapon, ItemInfo);
	}
}

void ATDSCharacter::WeaponFire(UAnimMontage* Anim)
{
	if (InventoryComponent && CurrentWeapon)
	{
		InventoryComponent->SetAdditionalInfoWeapon(CurrentIndexWeapon, CurrentWeapon->AdditionalWeaponInfo);
	}

	WeaponFire_BP(Anim);
}

void ATDSCharacter::WeaponReloadStart_BP_Implementation(UAnimMontage* Anim)
{
	//BP
}

void ATDSCharacter::WeaponReloadEnd_BP_Implementation(bool bIsSuccess)
{
	//BP
}

void ATDSCharacter::WeaponFire_BP_Implementation(UAnimMontage* Anim)
{
	//BP
}

UDecalComponent* ATDSCharacter::GetCursorToWorld()
{
	return CurrentCursor;
}


void ATDSCharacter::TrySwitchNextWeapon()
{
	if (CurrentWeapon && !CurrentWeapon->WeaponReloading && InventoryComponent->WeaponSlots.Num() > 1)
	{
		int8 OldIndex = CurrentIndexWeapon;
		FAdditionalWeaponInfo OldInfo;
		if(CurrentWeapon)
		{
			OldInfo = CurrentWeapon->AdditionalWeaponInfo;
			if (CurrentWeapon->WeaponReloading)
			{
				CurrentWeapon->CancelReload();
			}
		}
		if (InventoryComponent)
		{
			if (InventoryComponent->SwitchWeaponToIndexByNextPreviosIndex(CurrentIndexWeapon + 1, OldIndex, OldInfo, true))
			{

			}
		}
	}
}

void ATDSCharacter::TrySwitchPreviosWeapon()
{
	if (CurrentWeapon && !CurrentWeapon->WeaponReloading && InventoryComponent->WeaponSlots.Num() > 1)
	{
		int8 OldIndex = CurrentIndexWeapon;
		FAdditionalWeaponInfo OldInfo;
		if (CurrentWeapon)
		{
			OldInfo = CurrentWeapon->AdditionalWeaponInfo;
			if (CurrentWeapon->WeaponReloading)
			{
				CurrentWeapon->CancelReload();
			}
		}
		if (InventoryComponent)
		{
			if (InventoryComponent->SwitchWeaponToIndexByNextPreviosIndex(CurrentIndexWeapon - 1, OldIndex, OldInfo, false))
			{

			}
		}
	}
}

void ATDSCharacter::TryAbilityEnabled()
{
	if (AbilityEffect)
	{
		UTDS_StateEffect* newEffect = NewObject<UTDS_StateEffect>(this, AbilityEffect);
		if (newEffect)
		{
			newEffect->InitObject(this, NAME_None);
		}
	}
}

EPhysicalSurface ATDSCharacter::GetSurfaceType()
{
	EPhysicalSurface Result = EPhysicalSurface::SurfaceType_Default;
	if (HealthComponent)
	{
		if (HealthComponent->GetCurrentShield() <= 0)
		{
			if (GetMesh())
			{
				UMaterialInterface* myMaterial = GetMesh()->GetMaterial(0);
				if (myMaterial)
				{
					Result = myMaterial->GetPhysicalMaterial()->SurfaceType;
				}
			}
		}
	}

	return Result;
}

TArray<UTDS_StateEffect*> ATDSCharacter::GetAllCurrentEffects()
{
	return Effects;
}

void ATDSCharacter::RemoveEffect(UTDS_StateEffect* RemoveEffect)
{
	Effects.Remove(RemoveEffect);
}

void ATDSCharacter::AddEffect(UTDS_StateEffect* newEffect)
{
	Effects.Add(newEffect);
}

void ATDSCharacter::CharacterDead_BP_Implementation()
{
	//BP
}

void ATDSCharacter::CharacterDead()
{
	float TimeAnim = 0.0f;
	int32 rnd = FMath::RandHelper(DeadsAnim.Num());
	if (DeadsAnim.IsValidIndex(rnd) && DeadsAnim[rnd] && GetMesh() && GetMesh()->GetAnimInstance())
	{
		TimeAnim = DeadsAnim[rnd]->GetPlayLength();
		GetMesh()->GetAnimInstance()->Montage_Play(DeadsAnim[rnd]);
	}

	bIsAlive = false;

	if (GetController())
	{
		GetController()->UnPossess();
	}

	UnPossessed();

	GetWorldTimerManager().SetTimer(TimerHandle_RagDollTimer, this, &ATDSCharacter::EnableRagdoll, TimeAnim, false);

	GetCursorToWorld()->SetVisibility(false);

	AttackCharEvent(false);

	CharacterDead_BP();
}

void ATDSCharacter::EnableRagdoll()
{
	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		GetMesh()->SetSimulatePhysics(true);
	}
}

float ATDSCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (bIsAlive)
	{
		HealthComponent->ChangeHealthValue(-DamageAmount);
	}

	if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		AProjectileDefault* myProjectile = Cast<AProjectileDefault>(DamageCauser);
		if (myProjectile)
		{
			UTypes::AddEffectBySurfaceType(this, NAME_None,myProjectile->ProjectileSetting.Effect, GetSurfaceType());
		}
	}

	return ActualDamage;
}
