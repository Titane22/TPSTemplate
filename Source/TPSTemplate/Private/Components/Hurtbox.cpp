// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Hurtbox.h"

float UHurtbox::GetDamageMultiplier(const FName HitBoneName) const
{
	// 기본 배율
	float Multiplier = 1.0f;

	// BoneName이 유효하고 DamageMultipliers에 등록되어 있으면 해당 배율 사용
	if (!HitBoneName.IsNone() && DamageMultipliers.Contains(HitBoneName))
	{
		Multiplier = DamageMultipliers[HitBoneName];
	}

	return Multiplier;
}
