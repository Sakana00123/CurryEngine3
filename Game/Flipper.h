#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"

class Flipper : public Component
{
	C_REFLECT(Flipper)
public:
	/**@brief フリッパーの回転速度。*/
	C_PROPERTY()
	float flipForce = 500.0f;

	/**@brief フリッパーの回転方向。trueで右回転、falseで左回転。*/
	C_PROPERTY()
	bool isRightFlipper = true;

	/**@brief フリッパーの回転角度。*/
	C_PROPERTY()
	float flipAngle = 45.0f;

	/**@brief フリッパーの回転を操作するキー。*/
	C_PROPERTY()
	std::string flipKey = "Space"; // フリッパーを操作するキー（例: "Space"、"LeftShift"、"RightShift"など）

public:
	Flipper() = default;
	virtual ~Flipper() = default;
	void Start() override;
	void Update(float deltaTime) override;

private:
	float startAngle = 0.0f; // フリッパーの初期角度
	float currentFlipRate = 0.0f; // 現在のフリッパーの回転進捗(0.0fから1.0fの範囲)
	int flipDirection = 0; // フリッパーの回転方向（1: 回転中、-1: 反転中、0: 停止）
};