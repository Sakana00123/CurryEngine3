#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Engine/Effects/EffectManager.h"
#include "Engine/Physics/CollisionEvent.h"

class Enemy : public Component
{
	C_REFLECT(Enemy)
public:

	/**@brief 最大体力。*/
	C_PROPERTY()
	int maxHealth = 100;

	/**@brief 攻撃力。*/
	C_PROPERTY()
	int attackPower = 10;

	/**@brief 移動速度。*/
	C_PROPERTY()
	float moveSpeed = 7.0f;

	/**@brief 攻撃範囲。これ以内に入ると攻撃を行う。 */
	C_PROPERTY()
	float attackRange = 12.0f;

	/**@brief 戦闘範囲。これを超えると戦闘終了。 */
	C_PROPERTY()
	float fightRange = 50.0f;

	/**@brief ヒット判定範囲。*/
	C_PROPERTY()
	float hitRange = 5.0f;

public:
	Enemy() = default;
	virtual ~Enemy() = default;
	
	void Start() override;
	
	void Update(float deltaTime) override;
	
	//void DrawProperty() override;

	void TakeDamage(int damage);

	/** @brief 攻撃がヒットしたときに呼び出される関数。*/
	void OnHit(const TriggerInfo& info);

	void OnDeath();

private:
	int currentHealth = 0;
	bool isDead = false;
	bool isFighting = false;
	Transform* targetTransform = nullptr;
	EffectHandle hitEffectHandle = -1;

	// ダメージを与えたかどうか（アニメーション中の一度だけ与える用）
	bool hasDealtDamage = false;
};
