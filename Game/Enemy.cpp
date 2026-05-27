#include "pch.h"
#include "Enemy.h"

#include "CharacterMovement.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Audio/BeatManager.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Physics/Collider.h"
#include "Engine/Rendering/Renderers/GltfModelRenderer.h"
#include "Engine/Scenes/Scene.h"
#include "Engine/Scenes/SceneManager.h"

#include "Engine/Physics/BoxCollider.h"
#include "Engine/Physics/SphereCollider.h"

REGISTER_COMPONENT(Enemy, "Character")

void Enemy::Start()
{
	currentHealth = maxHealth;
	isDead = false;

	if (GameObject* characterObj = SceneManager::GetCurrentScene()->GetSceneObject("character"))
	{
		targetTransform = characterObj->transform;
	}

	if (auto* collider = GetOwner()->GetComponentInChildren<SphereCollider>())
	{
		collider->isTrigger = true;
		collider->radius = hitRange;
		collider->AddOnTriggerStayEvent([this](const TriggerInfo& info)
		{
				OnHit(info);
		});
		collider->SetEnabled(false);
	}

	hitEffectHandle = EffectManager::LoadEffectData("Assets/Effects/damage.json");
}

void Enemy::Update(float deltaTime)
{
	// 敵の行動ロジックをここに実装
	if (isDead) return;

	GltfModelRenderer* renderer = GetOwner()->GetComponent<GltfModelRenderer>();
	if (!renderer) return;

	if (isFighting)
	{
		// 戦闘中の行動
		
		// 移動アニメーション再生
		if (!isDead)
		{
			// ターゲットに向かって移動
			if (targetTransform)
			{
				Vector3 direction = targetTransform->GetWorldPosition() - GetOwner()->transform->GetWorldPosition();

				if (direction.Length() < attackRange)
				{
					// ターゲットに到達した場合の処理

					if (BeatManager::CheckBeatTiming() == BeatResult::Perfect)
					{
						//Console::Log("Beat Timing!");

						// ビートに合わせた攻撃行動
						if (renderer->GetCurrentAnimationName() != "box")
						{
							renderer->SetAnimation("box", true);
							renderer->SetLoop(false);
							Console::Log("Enemy Attack!");
							// 攻撃アニメーション中のみヒット判定を有効化
							if (auto* collider = GetOwner()->GetComponentInChildren<SphereCollider>())
							{
								collider->SetEnabled(true);
							}
						}
					}

					if (renderer->GetCurrentAnimationName() == "box" && 
						!renderer->IsAnimationCompleted())
					{
						// 攻撃アニメーション再生中のヒット判定
						XMFLOAT4X4 hitTransform = renderer->FindNode("mixamorig:LeftHandMiddle1")->globalTransform;
						XMFLOAT3 hitPoint = XMFLOAT3(hitTransform._41, hitTransform._42, hitTransform._43);
						hitPoint.x *= GetOwner()->transform->GetWorldScale().x;
						hitPoint.y *= GetOwner()->transform->GetWorldScale().y;
						hitPoint.z *= GetOwner()->transform->GetWorldScale().z;

						if (auto* collider = GetOwner()->GetComponentInChildren<SphereCollider>())
						{
							collider->isTrigger = true;
							collider->radius = hitRange;
							collider->center = Vector3(hitPoint);
						}
						
					}

					// 攻撃アニメーション終了後の処理
					if (renderer->IsAnimationCompleted())
					{
						// ヒット判定無効化
						if (auto* collider = GetOwner()->GetComponentInChildren<SphereCollider>())
						{
							collider->SetEnabled(false);
						}

						// 待機アニメーション再生
						if (renderer->GetCurrentAnimationName() != "idle")
						{
							renderer->SetAnimation("idle", true);
							renderer->SetLoop(true);
						}
						if (targetTransform)
						{
							XMVECTOR Origin = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&GetOwner()->transform->GetWorldPosition()));
							XMVECTOR Target = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&targetTransform->GetWorldPosition()));
							GetOwner()->transform->SetRotation(Transform::XMVectorToQuaternion(Transform::QuaternionLookAt(Origin, Target)));
						}
					}

					return;
				}
				else
				{
					if (renderer->GetCurrentAnimationName() != "walk")
					{
						renderer->SetAnimation("walk", true);
						renderer->SetLoop(true);
					}
				}

				XMVECTOR Origin = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&GetOwner()->transform->GetWorldPosition()));
				XMVECTOR Target = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&targetTransform->GetWorldPosition()));
				GetOwner()->transform->SetRotation(Transform::XMVectorToQuaternion(Transform::QuaternionLookAt(Origin, Target)));
				GetOwner()->transform->Translate(direction.Normalize() * moveSpeed * deltaTime);


				// 戦闘終了判定
				if (direction.Length() > fightRange)
				{
					isFighting = false;
				}
			}

		}
		
	}
	else
	{
		// 通常の行動
		if (renderer->GetCurrentAnimationName() != "idle")
		{
			renderer->SetAnimation("idle", true);
			renderer->SetLoop(true);
		}

		// ターゲットに向く
		if (targetTransform)
		{
			XMVECTOR Origin = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&GetOwner()->transform->GetWorldPosition()));
			XMVECTOR Target = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&targetTransform->GetWorldPosition()));
			GetOwner()->transform->SetRotation(Transform::XMVectorToQuaternion(Transform::QuaternionLookAt(Origin, Target)));

			// 戦闘開始判定
			float distance;
			XMStoreFloat(&distance, XMVector3Length(Target - Origin));
			if (distance < fightRange)
			{
				isFighting = true;
			}
		}
	}
}

//void Enemy::DrawProperty()
//{
//#ifdef USE_IMGUI
//	ImGui::Text("Current Health: %d / %d", currentHealth, maxHealth);
//
//	ImGui::DragInt("maxHealth", &maxHealth);
//	ImGui::DragInt("attackPower", &attackPower);
//	ImGui::DragFloat("moveSpeed", &moveSpeed);
//	ImGui::DragFloat("attackRange", &attackRange);
//	ImGui::DragFloat("fightRange", &fightRange);
//	ImGui::DragFloat("hitRange", &hitRange);
//	ImGui::Checkbox("fighting", &isFighting);
//#endif // USE_IMGUI
//}

void Enemy::TakeDamage(int damage)
{
	if (isDead) return;
	currentHealth -= damage;

	//// ダメージエフェクト再生
	//EffectManager::Play(hitEffectHandle, gameObject->transform->GetWorldPosition());

	//// 攻撃音再生
	//Audio::PlayOneShot(L"./Assets/Sounds/SE/hit.wav");

	if (currentHealth <= 0)
	{
		OnDeath();
	}
}

void Enemy::OnHit(const TriggerInfo& info)
{
	// 攻撃がヒットしたときの処理をここに実装
	if (GameObject* characterObj = info.other)
	{
		if (CharacterMovement* character = characterObj->GetComponent<CharacterMovement>())
		{
			// ダメージエフェクト再生
			int instanceId = EffectManager::Play(hitEffectHandle, Vector3(info.otherCollider->GetTransform()->GetWorldPosition()));
			// 攻撃音再生
			Audio::PlayOneShot(L"./Assets/Sounds/SE/damage.wav");

			character->TakeDamage(attackPower);
			info.selfCollider->SetEnabled(false);
		}
	}
}

void Enemy::OnDeath()
{
	currentHealth = 0;
	isDead = true;
	// 死亡時の処理をここに実装
	if (GltfModelRenderer* renderer = GetOwner()->GetComponent<GltfModelRenderer>())
	{
		renderer->SetAnimation("death", false);
		renderer->SetLoop(false);
	}
	// ヒット判定無効化
	if (auto* collider = GetOwner()->GetComponentInChildren<SphereCollider>())
	{
		collider->SetEnabled(false);
	}
}