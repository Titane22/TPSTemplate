// Fill out your copyright notice in the Description page of Project Settings.


#include "AWeapon_Handgun.h"
#include "../WeaponSystem.h"
#include "./Weapon/DA_Pistol.h"
#include "./Weapon/WeaponFireCameraShake.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "../TPSTemplateCharacter.h"
#include "./Widget/W_DynamicWeaponHUD.h"
#include "Weapon/IWeaponPickup.h"  
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AAWeapon_Handgun::AAWeapon_Handgun()
{
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> HandgunMesh(TEXT("/Game/Weapons/Pistol/Mesh/SK_Handgun"));
    if (HandgunMesh.Succeeded() && WeaponMesh) 
    {
        WeaponMesh->SetSkeletalMesh(HandgunMesh.Object);
    }

    // ���� Ÿ�� ����
    WeaponType = EAnimationState::Pistol;
    WeaponData = NewObject<UDA_Pistol>();
    bReloading = false;
    bAutoReload = true;

    WeaponSystem->MasterWeapon = this;
    WeaponSystem->AnimationState = EAnimationState::Unarmed;
    WeaponSystem->Weapon_Details = {
        FWeapon_Data{
            /* CurrentAmmo */ 13,
            /* MaxAmmo */ 90,
            /* ClipAmmo */ 13,
            /* DifferentAmmo */ 90,
            /* Ammo_Count */ 1,
            /* ShortGun_Trace */ false
        }
    };

    WeaponSystem->bIsDryAmmo = false;
    WeaponSystem->PistolData = nullptr;
    WeaponSystem->RifleData = NewObject<UDA_Pistol>();

    WeaponSystem->CrosshairWidget = nullptr;

    if (Muzzle && WeaponMesh)
    {
        Muzzle->AttachToComponent(WeaponMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("Muzzle"));
    }
    else
        UE_LOG(LogTemp, Warning, TEXT("Muzzle || WeaponMesh is Null"));
}

void AAWeapon_Handgun::BeginPlay()
{
    Super::BeginPlay();

    // ���� Ÿ�� ����
    WeaponType = EAnimationState::Pistol;
    WeaponData = NewObject<UDA_Pistol>();
    bReloading = false;
    bAutoReload = true;

    WeaponSystem->MasterWeapon = nullptr;
    WeaponSystem->AnimationState = EAnimationState::Unarmed;
    WeaponSystem->Weapon_Details = {
        FWeapon_Data{
            /* CurrentAmmo */ 13,
            /* MaxAmmo */ 90,
            /* ClipAmmo */ 13,
            /* DifferentAmmo */ 90,
            /* Ammo_Count */ 1,
            /* ShortGun_Trace */ false
        }
    };

    WeaponSystem->bIsDryAmmo = false;
    WeaponSystem->PistolData = nullptr;
    WeaponSystem->RifleData = NewObject<UDA_Pistol>();

    WeaponSystem->CrosshairWidget = nullptr;

    if (Muzzle && WeaponMesh)
    {
        Muzzle->AttachToComponent(WeaponMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("Muzzle"));
    }
    else
        UE_LOG(LogTemp, Warning, TEXT("Muzzle || WeaponMesh is Null"));

    UClass* LoadedClass = LoadClass<AIWeaponPickup>(
        nullptr,
        TEXT("/Game/ThirdPerson/Weapons/BP_IWeaponPickup_Pistol.BP_IWeaponPickup_Pistol_C")
    );

    if (LoadedClass)
    {
        WeaponPickupClass = LoadedClass;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load WeaponPickup class"));
    }
}

void AAWeapon_Handgun::Fire()
{
    if (bReloading)
        return;

    if (WeaponSystem->FireCheck(WeaponSystem->Weapon_Details.Weapon_Data.Ammo_Count))
    {
        if (APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
        {
            FVector EpicenterLocation = GetActorLocation();
            PC->ClientStartCameraShake(UWeaponFireCameraShake::StaticClass(), 1.0f);

            // Get camera location and forward vector
            APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
            if (!CameraManager)
                return;

            FVector StartLocation = CameraManager->GetRootComponent()->GetComponentLocation();
            FVector ForwardVector = CameraManager->GetActorForwardVector();
            if (!WeaponData)
            {
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("WeaponData is null"));
            }
            else if (!WeaponData->MaxRange)
            {
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("WeaponData->MaxRange is null"));
                return;
            }
            FVector EndLocation = StartLocation + (ForwardVector * WeaponData->MaxRange);

            // Setup trace parameters
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(this);

            // Draw debug line
            //DrawDebugLine(
            //    GetWorld(),           // ����
            //    StartLocation,        // ������
            //    EndLocation,          // ����
            //    FColor::Yellow,       // ���� ����
            //    false,               // ���������� �׸��� ����
            //    5.0f,                // ���� �ð� (��)
            //    0,                   // �켱����
            //    1.0f                 // �β�
            //);


            // Trace #1: From camera forward
            FHitResult HitResult;
            if (GetWorld()->LineTraceSingleByChannel(
                HitResult,
                StartLocation,
                EndLocation,
                ECollisionChannel::ECC_Visibility,
                QueryParams
            ))
            {
                FVector MuzzleLocation = WeaponMesh->GetSocketLocation(FName("Muzzle"));
                FVector EndMuzzleLocation = MuzzleLocation - HitResult.Location;
                // Trace #2: From the muzzle to the camera's hit location
                FHitResult CameraHitResult;

                //DrawDebugLine(
                //    GetWorld(),           // ����
                //    MuzzleLocation,        // ������
                //    EndMuzzleLocation * -500,          // ����
                //    FColor::Yellow,       // ���� ����
                //    false,               // ���������� �׸��� ����
                //    5.0f,                // ���� �ð� (��)
                //    0,                   // �켱����
                //    1.0f                 // �β�
                //);

                bool bHit = GetWorld()->LineTraceSingleByChannel(
                    CameraHitResult,
                    MuzzleLocation,
                    EndMuzzleLocation * -500,
                    ECollisionChannel::ECC_Visibility,
                    QueryParams
                );

                FireFX();

                FireBullet(CameraHitResult, bHit);
            }
            else
            {
                FireFX();

                FireBlankTracer();
            }
        }
    }
    else
    {
        if (WeaponSystem->Weapon_Details.Weapon_Data.MaxAmmo > 0)
        {
            if (bAutoReload)
                Reload();
            else
            {
                // Empty FX
                if (USoundBase* EmptySound = Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), nullptr, TEXT("/Game/ThirdPerson/Audio/Sounds/Weapons/Pistol/Weapons_Pistol_DryFire_01"))))
                {
                    WeaponSystem->EmptyFX(EmptySound);
                }
                else
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TEXT("AWeapon_AssultRifle::Fire()::EmptySound"));
                return;
            }
        }
    }
}

void AAWeapon_Handgun::FireBullet(FHitResult Hit, bool bReturnHit)
{
    for (int32 curBurst = 0; curBurst < WeaponData->BurstAmount; curBurst++)
    {
        float PointX, PointY;
        RandPointInCircle(FMath::Tan(WeaponData->BulletSpread) * 10.0f, PointX, PointY);

        // Get the player camera manager for the bullet direction
        APlayerCameraManager* CameraManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
        if (!CameraManager)
        {
            return;
        }
        FVector SpreadAdjustedHitLocation = Hit.Location + CameraManager->GetActorRightVector() * PointX + CameraManager->GetActorUpVector() * PointY;
        FVector MuzzleLocation = WeaponMesh->GetSocketLocation(FName("Muzzle"));
        // BulletDirection�� �ѱ����� ��ǥ ���������� ������ ��Ÿ���ϴ�.
        // �ѱ� ��ġ���� ������ ��ġ�� ���� ź���� ���� ���͸� ����մϴ�.
        FVector BulletDirection = MuzzleLocation - SpreadAdjustedHitLocation;

        // Setup trace parameters
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);

        // Create array of actors to ignore
        if (!WeaponSystem || !WeaponSystem->CharacterRef)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("WeaponSystem or WeaponSystem->CharacterRef is NULL"));
            return;
        }
        //QueryParams.AddIgnoredActor(Cast<AActor>(WeaponSystem->CharacterRef));

        //DrawDebugLine(
        //    GetWorld(),           // ����
        //    MuzzleLocation,        // ������
        //    MuzzleLocation + (BulletDirection * -5.0f),          // ����
        //    FColor::Yellow,       // ���� ����
        //    false,               // ���������� �׸��� ����
        //    5.0f,                // ���� �ð� (��)
        //    0,                   // �켱����
        //    1.0f                 // �β�
        //);
        // Perform line trace
        FHitResult HitResult;
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            MuzzleLocation,
            MuzzleLocation + (BulletDirection * -5.0f),
            ECollisionChannel::ECC_Visibility,
            QueryParams
        );
        if (!bHit)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Anyone not Hit!!!!!!"));
            return;
        }

        // Process hit result
        if (HitResult.GetActor())
        {
            // Check if hit component is simulating physics
            UPrimitiveComponent* HitComponent = HitResult.GetComponent();
            if (HitComponent && HitComponent->IsSimulatingPhysics())
            {
                // Apply physics impulse at impact point
                FVector ImpulseDir = -BulletDirection.GetSafeNormal();
                HitComponent->AddImpulseAtLocation(ImpulseDir * -1000.0f, HitResult.Location);
            }

            // Apply damage to hit actor
            bool bValidHit;
            bool bKilledPlayer = ApplyHit(HitResult, bValidHit);

            if (bValidHit)
            {
                // HitMarker
                UUserWidget* UIHitMarker = CreateWidget<UUserWidget>(GetWorld()->GetFirstPlayerController(),
                    LoadClass<UUserWidget>(nullptr, TEXT("/Game/ThirdPerson/Blueprints/UI/HitMarker.HitMarker_C")));

                UIHitMarker->AddToViewport();
            }
            if (bKilledPlayer)
            {
                // PlaySound2D
                if (USoundBase* SoundBase = Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), nullptr, TEXT("/Game/ThirdPerson/Audio/Sounds/Weapons/kill-sound-effect_Cue"))))
                {
                    UGameplayStatics::PlaySound2D(
                        this,
                        SoundBase,
                        1.0f,
                        1.0f,
                        0.0f,
                        nullptr,
                        nullptr,
                        true
                    );
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("AAWeapon_Handgun::FireBullet::bKilledPlayer::SoundBase Is NULL"));
                }
            }

            // Spawn bullet trace effect
            if (UClass* BulletTraceClass = LoadClass<AActor>(nullptr, TEXT("/Game/Weapons/BulletTrace/Bullet_Trace.Bullet_Trace_C")))
            {
                FVector BulletEndLocation = HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd;
                BulletEndLocation = BulletEndLocation - MuzzleLocation;

                FTransform SpawnTransform;
                SpawnTransform.SetLocation(WeaponMesh->GetSocketLocation(FName("Muzzle")));
                SpawnTransform.SetRotation(BulletEndLocation.Rotation().Quaternion());
                SpawnTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = this;
                SpawnParams.Instigator = GetInstigator();

                GetWorld()->SpawnActor<AActor>(BulletTraceClass, SpawnTransform, SpawnParams);
            }
            else
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("BulletTraceClass is Null"));
        }

        Hit = HitResult;
        bReturnHit = true;
    }
}

void AAWeapon_Handgun::FireFX()
{
    USoundAttenuation* SoundAttenuation = LoadObject<USoundAttenuation>(nullptr, TEXT("/Game/ThirdPerson/Audio/AttenuationPresets/ATT_Pistol"));
    if (!SoundAttenuation)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("SoundAttenuation is NULL"));
        return;
    }

    USoundConcurrency* SoundConcurrency = LoadObject<USoundConcurrency>(nullptr, TEXT("/Game/ThirdPerson/Audio/Concurrency/SCON_Guns_LimitToOwner"));
    if (!SoundConcurrency)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("SoundConcurrency is NULL"));
        return;
    }
    WeaponSystem->FireFX(WeaponData->FireSound, WeaponMesh->GetSocketLocation("Muzzle"), SoundAttenuation, SoundConcurrency);

    UNiagaraSystem* SystemTemplate = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Weapons/Effects/Particles/Weapons/NS_WeaponFire"));
    if (!SystemTemplate)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Failed to load Niagara System"));
        return;
    }

    WeaponSystem->MuzzleVFX(SystemTemplate, Muzzle);

    WeaponSystem->FireMontage(WeaponData->BodyFireMontage, nullptr);

    WeaponMesh->PlayAnimation(WeaponData->WeaponFireMontage, false);
}

void AAWeapon_Handgun::FireBlankTracer()
{
    if (APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        FVector SocketLocation = WeaponMesh->GetSocketLocation(FName("Muzzle"));
        PC->ClientStartCameraShake(UWeaponFireCameraShake::StaticClass(), 1.0f);

        // Get camera location and forward vector
        APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
        if (!CameraManager)
            return;
        if (UClass* BulletTraceClass = LoadClass<AActor>(nullptr, TEXT("/Game/Weapons/BulletTrace/Bullet_Trace.Bullet_Trace_C")))
        {
            FVector TraceEndLocation = CameraManager->GetRootComponent()->GetComponentLocation() + CameraManager->GetActorForwardVector() * 20000.0f;
            FVector DirectionVector = TraceEndLocation - SocketLocation;
            FRotator Rotation = UKismetMathLibrary::MakeRotFromX(DirectionVector);

            FTransform NewTransform(Rotation, SocketLocation, FVector(1.0f));
            GetWorld()->SpawnActor<AActor>(BulletTraceClass, NewTransform);
        }
    }
}

void AAWeapon_Handgun::Reload()
{
    if (!WeaponSystem || !WeaponData)
        return;

    if (!WeaponSystem->CheckAmmo())
        return;
    bReloading = true;
    float ReloadDelay = 0.0f;
    if (WeaponSystem)
    {
        ReloadDelay = WeaponSystem->ReloadMontage(WeaponData->BodyReloadMontage, nullptr);
    }

    if (WeaponMesh)
    {
        WeaponMesh->PlayAnimation(WeaponData->WeaponReloadMontage, false);
    }

    WeaponSystem->ReloadCheck();

    FTimerHandle ReloadTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        ReloadTimerHandle,
        [this]()
        {
            if (WeaponSystem && WeaponSystem->CharacterRef &&
                WeaponSystem->CharacterRef->CurrentWeaponUI)
            {
                FWeapon_Details WeaponDetails = WeaponSystem->Weapon_Details;
                WeaponSystem->CharacterRef->CurrentWeaponUI->UpdateAmmoCount(
                    WeaponDetails.Weapon_Data.MaxAmmo,
                    WeaponDetails.Weapon_Data.CurrentAmmo
                );
            }
            bReloading = false;
        },
        ReloadDelay,
        false
    );
}



void AAWeapon_Handgun::RandPointInCircle(float Radius, float& PointX, float& PointY)
{
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float DistanceFromCenter = FMath::RandRange(0.0f, Radius);

    // Convert polar coordinates to cartesian coordinates
    PointX = DistanceFromCenter * FMath::Cos(FMath::DegreesToRadians(Angle));
    PointY = DistanceFromCenter * FMath::Sin(FMath::DegreesToRadians(Angle));
}
