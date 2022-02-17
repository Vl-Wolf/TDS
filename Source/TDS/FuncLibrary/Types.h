// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "Types.generated.h"

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Aim_State UMETA(DisplayName = "Aim State"),
	Walk_State UMETA(DisplayName = "Walk State"),
	Run_State UMETA(DisplayName = "Run State"),
	AimWalk_State UMETA(DisplayName = "AimWalk State"),
	Sprint_State UMETA(DisplayName = "Sprint State"),
};

USTRUCT(BlueprintType)
struct FCharacterSpeed
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float AimSpeed = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float WalkSpeed = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float RunSpeed = 600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float AimWalkSpeed = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float SprintSpeed = 800.0f;
};

USTRUCT(BlueprintType)
struct FProjectileInfo
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		TSubclassOf<class AProjectileDefault> Projectile = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		UStaticMesh* ProjectileStaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		FTransform ProjectileStaticMeshOffset = FTransform();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		UParticleSystem* ProjectileTrailFX = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		FTransform ProjectileTrailFXOffset = FTransform();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		float ProjectileDamage = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		float ProjectileLifeTime = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		float ProjectileInitSpeed = 2000.0f;
	//DecalHit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
		TMap<TEnumAsByte<EPhysicalSurface>, UMaterialInterface*> HitDecals;
	//SoundHit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
		USoundBase* HitSound = nullptr;
	//FXHit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
		TMap<TEnumAsByte<EPhysicalSurface>, UParticleSystem*> HitFXs;
	//Grenade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
		UParticleSystem* ExploseFX = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
		USoundBase* ExploseSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
		float ProjectileMaxRadiusDamage = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
		float ProjectileMinRadiusDamage = 400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
		float ExploseMaxDamage = 40.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
		float ExplodeFallCoef = 1.0f;


};

USTRUCT(BlueprintType)
struct FWeaponDispersion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float Aim_StateDispersionAimMax = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float Aim_StateDispersionAimMin = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float Aim_StateDispersionAimRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float Aim_StateDispersionReduction = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float AimWalk_StateDispersionAimMax = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float AimWalk_StateDispersionAimMin = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float AimWalk_StateDispersionAimRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float AimWalk_StateDispersionReduction = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float Walk_StateDispersionAimMax = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float Walk_StateDispersionAimMin = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float Walk_StateDispersionAimRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float Walk_StateDispersionReduction = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float Run_StateDispersionAimMax = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float Run_StateDispersionAimMin = 4.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float Run_StateDispersionAimRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		float Run_StateDispersionReduction = 0.1f;

};

USTRUCT(BlueprintType)
struct FAnimationWeaponInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Character")
		UAnimMontage* AnimCharacterFire = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Character")
		UAnimMontage* AnimCharacterFireAim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Character")
		UAnimMontage* AnimCharacterReload = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Character")
		UAnimMontage* AnimCharacterReloadAim = nullptr;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Weapon")
		UAnimMontage* AnimWeaponFire = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Weapon")
		UAnimMontage* AnimWeaponReload = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Weapon")
		UAnimMontage* AnimWeaponReloadAim = nullptr;

};

USTRUCT(BlueprintType)
struct FDropMeshInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Mesh")
		UStaticMesh* DropMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Mesh")
		float DropTime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Mesh")
		float DropMeshLifeTime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Mesh")
		FTransform DropMeshOffset = FTransform();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Mesh")
		FVector DropMeshImpulseDirection = FVector(0.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Mesh")
		float MassMesh = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Mesh")
		float PowerImpulse = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Mesh")
		float ImpulseRandomDispersion = 0.0f;
};

USTRUCT(BlueprintType)
struct FWeaponInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
		TSubclassOf<class AWeaponDefault> WeaponClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		float RateOfFire = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		float ReloadTime = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		int32 MaxRound = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		int32 NumberProjectileByShot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
		FWeaponDispersion DispesionWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
		USoundBase* SoundFireWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
		USoundBase* SoundReloadWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
		UParticleSystem* EffectFireWeapon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
		FProjectileInfo ProjectileSetting;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
		float WeaponDamage = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
		float DistanceTrace = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitEffect")
		UDecalComponent* DecalOnHit = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		FAnimationWeaponInfo AnimationWeaponInfo;
;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Mesh")
		FDropMeshInfo MagasinDrop;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Mesh")
		FDropMeshInfo ShellBullets;
	
};

USTRUCT(BlueprintType)
struct FAddicionalWeaponInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
		int32 Round = 10;
};

UCLASS()
class TDS_API UTypes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};