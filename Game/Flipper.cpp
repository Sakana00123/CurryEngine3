#include "pch.h"
#include "Flipper.h"
#include "Engine/Input/InputSystem.h"

REGISTER_COMPONENT(Flipper, "Flipper")

void Flipper::Start()
{
	// フリッパーの初期角度を保存
	startAngle = GetTransform()->GetEulerAngles().y;

}

void Flipper::Update(float deltaTime)
{
	// 入力に応じてフリッパーを回転させる
	if (InputSystem::GetInputState(flipKey, InputStateMask::Trigger))
	{
		if (flipDirection == 0) // 停止状態から回転開始
			flipDirection = 1;
	}
	// フリッパーの回転を更新
	switch (flipDirection)
	{
		case 1: // 回転中
		{
			// 回転進捗を更新
			currentFlipRate += flipForce * deltaTime / flipAngle;
			if (currentFlipRate >= 1.0f)
			{
				currentFlipRate = 1.0f;
				flipDirection = -1; // 反転開始
			}
			break;
		}
		case -1: // 反転中
		{
			// 回転進捗を更新
			currentFlipRate -= flipForce * deltaTime / flipAngle;
			if (currentFlipRate <= 0.0f)
			{
				currentFlipRate = 0.0f;
				flipDirection = 0; // 停止
			}
			break;
		}
	default:
		return; // 停止中は何もしない
	}

	// フリッパーの角度を計算して適用
	float targetAngle = startAngle + (isRightFlipper ? 1 : -1) * flipAngle * currentFlipRate;
	Vector3 eulerAngles = GetTransform()->GetEulerAngles();
	eulerAngles.y = targetAngle;
	GetTransform()->SetRotation(eulerAngles);
}