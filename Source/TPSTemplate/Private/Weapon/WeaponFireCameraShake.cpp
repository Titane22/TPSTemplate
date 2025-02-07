// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponFireCameraShake.h"

UWeaponFireCameraShake::UWeaponFireCameraShake()
{
	// 진동 관련 설정
	OscillationDuration = 0.15f;
	OscillationBlendInTime = 0.1f;
	OscillationBlendOutTime = 0.1f;

	// 회전 진동은 Random으로 설정
	RotOscillation.Pitch.Amplitude = 0.5f;
	RotOscillation.Pitch.Frequency = 1.0f;
	RotOscillation.Pitch.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;
	RotOscillation.Pitch.Waveform = EOscillatorWaveform::SineWave;

	RotOscillation.Yaw.Amplitude = 0.0f;
	RotOscillation.Yaw.Frequency = 0.0f;
	RotOscillation.Yaw.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;
	RotOscillation.Yaw.Waveform = EOscillatorWaveform::SineWave;

	RotOscillation.Roll.Amplitude = 0.25f;
	RotOscillation.Roll.Frequency = 0.65f;
	RotOscillation.Roll.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;
	RotOscillation.Roll.Waveform = EOscillatorWaveform::SineWave;

	// 위치 진동 설정
	LocOscillation.X.Amplitude = -14.0f;
	LocOscillation.X.Frequency = 2.0f;
	LocOscillation.X.InitialOffset = EInitialOscillatorOffset::EOO_OffsetZero;
	LocOscillation.X.Waveform = EOscillatorWaveform::SineWave;

	LocOscillation.Y.Amplitude = 0.0f;
	LocOscillation.Y.Frequency = 0.0f;
	LocOscillation.Y.InitialOffset = EInitialOscillatorOffset::EOO_OffsetZero;
	LocOscillation.Y.Waveform = EOscillatorWaveform::PerlinNoise;

	LocOscillation.Z.Amplitude = -2.0f;
	LocOscillation.Z.Frequency = 1.0f;
	LocOscillation.Z.InitialOffset = EInitialOscillatorOffset::EOO_OffsetZero;
	LocOscillation.Z.Waveform = EOscillatorWaveform::PerlinNoise;

	// FOV 진동 설정
	FOVOscillation.Amplitude = 1.0f;
	FOVOscillation.Frequency = 0.5f;
	FOVOscillation.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;
	FOVOscillation.Waveform = EOscillatorWaveform::SineWave;

	AnimPlayRate = 1.0f;
	AnimScale = 1.0f;
	AnimBlendInTime = 0.1f;
	AnimBlendOutTime = 0.1f;
}

