#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Effects/EffectManager.h"
#include "Engine/Physics/Collider.h"

class CharacterMovement : public Component
{
	C_REFLECT(CharacterMovement)
public:
	C_PROPERTY()
	float moveSpeed = 10.0f;
	C_PROPERTY()
	int maxHealth = 100;
	
	C_PROPERTY()
	int attackPower = 20;

	C_PROPERTY()
	float attackRange = 5.0f;

	C_PROPERTY()
	float jumpPower = 100.0f;

public:

	CharacterMovement() = default;
	virtual ~CharacterMovement() = default;
	void SetMoveSpeed(float speed) { moveSpeed = speed; }
	float GetMoveSpeed() const { return moveSpeed; }
	void SetJumpPower(float power) { jumpPower = power; }
	float GetJumpPower() const { return jumpPower; }

	void Start() override;

	void Update(float deltaTime) override;

	//void DrawProperty() override;


	void UpdateAnimation(float deltaTime);

	void TakeDamage(int damage);

	/** @brief 攻撃が発生したときに呼び出される関数。*/
	void OnAttack(const TriggerInfo& info);

private:
	bool isAttacking = false;
	bool previousMoving = false; // 前フレームの移動状態をトラッキング
	bool isDead = false;
	float movingTime = 0.0f; // 移動時間のトラッキング用
	EffectHandle attackEffectHandle = -1;
	EffectHandle dodgeEffectHandle = -1;
	EffectHandle damageEffectHandle = -1;
	//int dodgeCount = 0; // 連続回避回数のトラッキング

	int currentHealth = 100;

	float gravity = -360.0f; // 重力加速度
	Vector3 velocity; // キャラクターの現在の速度

};
