#include "pch.h"
#include "CharacterMovement.h"

#include "Enemy.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Audio/BeatManager.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Core/Transform.h"
#include "Engine/Effects/ParticleComponent.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Physics/Rigidbody.h"
#include "Engine/Rendering/Renderers/GltfModelRenderer.h"

#include "Engine/Physics/BoxCollider.h"
#include "Engine/Physics/SphereCollider.h"


REGISTER_COMPONENT(CharacterMovement, "Character")

void CharacterMovement::Start()
{
	// 初期化処理
	//attackEffectHandle = EffectManager::LoadEffectData("Assets/Effects/box.json");
	dodgeEffectHandle = EffectManager::LoadEffectData("Assets/Effects/dodge.json");
	damageEffectHandle = EffectManager::LoadEffectData("Assets/Effects/damage.json");

	// 体力初期化
	currentHealth = maxHealth;

	// 攻撃イベント登録
	if (auto* collider = GetOwner()->GetComponentInChildren<SphereCollider>())
	{
		collider->isTrigger = true;
		collider->radius = attackRange;
		collider->AddOnTriggerEnterEvent([this](const TriggerInfo& info)
			{
				OnAttack(info);
			});
		collider->SetEnabled(false);
	}
}

void CharacterMovement::Update(float deltaTime)
{
	// キャラクターの移動ロジックをここに実装
	DirectX::XMFLOAT3 forward = GetOwner()->transform->GetForward();
	DirectX::XMFLOAT3 right = GetOwner()->transform->GetRight();
	Vector3 movement{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 outRot = GetOwner()->transform->GetWorldRotation();
	bool isMoving = false;

	// 移動時間の更新
	if (previousMoving)
	{
		movingTime += deltaTime;
	}
	else
	{
		movingTime = 0.0f;
	}

	// カメラのベクトルを取得
	if (GameObject* camera = ObjectManager::Find("Camera"))
	{
		// カメラの前方向ベクトルを取得
		DirectX::XMFLOAT3 cameraForward = camera->transform->GetForward();
		{
			// Y成分を0にして水平面上のベクトルに変換
			cameraForward.y = 0.0f;
			DirectX::XMVECTOR camForwardVec = DirectX::XMLoadFloat3(&cameraForward);
			camForwardVec = DirectX::XMVector3Normalize(camForwardVec);
			DirectX::XMStoreFloat3(&cameraForward, camForwardVec);
		}

		// カメラの右方向ベクトルを取得
		DirectX::XMFLOAT3 cameraRight = camera->transform->GetRight();
		{
			// Y成分を0にして水平面上のベクトルに変換
			cameraRight.y = 0.0f;
			DirectX::XMVECTOR camRightVec = DirectX::XMLoadFloat3(&cameraRight);
			camRightVec = DirectX::XMVector3Normalize(camRightVec);
			DirectX::XMStoreFloat3(&cameraRight, camRightVec);
		}

		//if (movingTime > 0.5f)
		{
			forward = cameraForward;
			right = cameraRight;
		}

	}

	if (GltfModelRenderer* renderer = GetOwner()->GetComponent<GltfModelRenderer>())
	{
		// 死亡アニメーションの優先処理
		if (isDead)
		{
			if (renderer->GetCurrentAnimationName() != "death")
			{
				// 死亡アニメーションを再生
				renderer->SetAnimation("death", false);
				renderer->SetAnimationTimeRate(1.0f);
				renderer->animationBlendTime = 0.1f;
				renderer->SetLoop(false);
			}
			return; // 死亡時は他のアニメーションを更新しない
		}

		if (renderer->GetCurrentAnimationName() == "dodge")
		{
			movement = Vector3(GetOwner()->transform->GetForward());
			movement.x *= moveSpeed * 2.5f * deltaTime;
			movement.y = 0.0f;
			movement.z *= moveSpeed * 2.5f * deltaTime;
		}
	}

	// 入力に基づく移動処理
	if (!isAttacking)
	{
		if (InputSystem::GetInputState("Up"))
		{
			movement.x += forward.x * moveSpeed * deltaTime;
			movement.y += forward.y * moveSpeed * deltaTime;
			movement.z += forward.z * moveSpeed * deltaTime;
			isMoving = true;
		}
		if (InputSystem::GetInputState("Down"))
		{
			movement.x -= forward.x * moveSpeed * deltaTime;
			movement.y -= forward.y * moveSpeed * deltaTime;
			movement.z -= forward.z * moveSpeed * deltaTime;
			isMoving = true;
		}
		if (InputSystem::GetInputState("Left"))
		{
			movement.x -= right.x * moveSpeed * deltaTime;
			movement.y -= right.y * moveSpeed * deltaTime;
			movement.z -= right.z * moveSpeed * deltaTime;
			isMoving = true;
		}
		if (InputSystem::GetInputState("Right"))
		{
			movement.x += right.x * moveSpeed * deltaTime;
			movement.y += right.y * moveSpeed * deltaTime;
			movement.z += right.z * moveSpeed * deltaTime;
			isMoving = true;
		}

		// キャラクターの向きを移動方向に合わせる
		if (isMoving)
		{
			// 移動方向（Yaw限定）
			XMFLOAT3 dirF = movement;
			dirF.y = 0.0f;

			XMVECTOR dir = XMLoadFloat3(&dirF);

			if (XMVector3LengthSq(dir).m128_f32[0] < 0.0001f)
			{
				// 移動方向がほぼゼロベクトルの場合は回転しない
			}
			else
			{
				dir = XMVector3Normalize(dir);

				// 目標Yawを計算（Z+ forward想定）
				float targetYaw = atan2f(
					XMVectorGetX(dir),
					XMVectorGetZ(dir)
				);

				// 目標回転
				XMVECTOR targetRot = XMQuaternionRotationRollPitchYaw(
					0.0f,
					targetYaw,
					0.0f
				);

				// 現在回転
				XMFLOAT4 currentRotF = outRot;
				XMVECTOR currentRot = XMLoadFloat4(&currentRotF);

				// Quaternion符号対策
				if (XMVectorGetX(XMQuaternionDot(currentRot, targetRot)) < 0.0f)
				{
					targetRot = XMVectorNegate(targetRot);
				}

				// なめらかに補間
				float rotationSpeed = 10.0f; // 回転速度
				float t = 1.0f - expf(-rotationSpeed * deltaTime);

				XMVECTOR newRot = XMQuaternionSlerp(
					currentRot,
					targetRot,
					t
				);
				// 回転を適用
				XMStoreFloat4(&outRot, newRot);
			}
		}
	}
	

	if (Rigidbody* rb = GetOwner()->GetComponent<Rigidbody>())
	{
#if 1 // Kinematicターゲット方式で移動と重力を適用
		
		// ジャンプ処理（スペースキーが押された場合）
		if (InputSystem::GetInputState("Space", InputStateMask::Trigger))
		{
			// ジャンプのロジックをここに実装
			velocity = Vector3{ 0.0f, jumpPower, 0.0f };
		}
		Vector3 currentPos = GetOwner()->transform->GetWorldPosition();

		if (rb->useGravity)
		{
			// 重力の適用
			velocity.y += gravity * deltaTime;
		}
		else
		{
			velocity.y = 0.0f; // 重力が無効な場合はY軸の速度をリセット
		}

		// 位置の更新
		movement += velocity * deltaTime;

		// 移動を適用
		currentPos += movement;

		// キネマティックターゲットを更新
		rb->SetKinematicTarget(currentPos, outRot);

#else // Rigidbodyの物理挙動に任せる方式
		// 位置と回転を直接設定
		Vector3 pos;
		Quaternion rot;
		rb->GetGlobalPose(pos, rot); // 現在の速度を取得

		pos += movement; // 入力に基づく移動を加算

		rb->SetGlobalPose(pos, outRot);
		
		// ジャンプ処理（スペースキーが押された場合）
		if (InputSystem::GetInputState("Space", InputStateMask::Trigger))
		{
			// ジャンプのロジックをここに実装
			rb->AddForce(Vector3{ 0.0f, jumpPower, 0.0f }, ForceMode::Impulse);
		}

#endif // 0

	}
	

	previousMoving = isMoving;

	// アニメーションの更新
	UpdateAnimation(deltaTime);

	// ビートタイミングでの回避エフェクト再生(テスト用)
	{
		if (BeatManager::IsJustBeat())
		{
			EffectManager::Play(dodgeEffectHandle, GetOwner()->transform->GetPosition(), GetOwner()->transform->GetEulerAngles());
			//Audio::PlayOneShot(L"./Data/Sounds/SE/kick.wav");

		}
	}
}

//void CharacterMovement::DrawProperty()
//{
//#ifdef USE_IMGUI
//
//	ImGui::DragFloat("Move Speed", &moveSpeed, 0.1f);
//	ImGui::DragFloat("Jump Power", &jumpPower, 0.1f);
//	ImGui::DragInt("Max Health", &maxHealth, 1);
//	ImGui::DragFloat("Attack Range", &attackRange, 0.1f);
//
//#endif // USE_IMGUI
//
//}


void CharacterMovement::UpdateAnimation(float deltaTime)
{
	// アニメーション更新ロジックをここに実装
	if (GltfModelRenderer* renderer = GetOwner()->GetComponent<GltfModelRenderer>())
	{
		// 攻撃中の場合、攻撃アニメーションの完了をチェック
		if (isAttacking)
		{
			// 攻撃アニメーションが完了したかチェック
			if (renderer->IsAnimationCompleted())
			{
				isAttacking = false;
				//dodgeCount = 0; // 回避カウントをリセット

				//if (auto* collider = GetOwner()->GetComponentInChildren<SphereCollider>())
				//{
				//	// 攻撃判定を無効化
				//	collider->SetEnable(false);
				//}

			}
			else if (auto* collider = GetOwner()->GetComponentInChildren<SphereCollider>())
			{
				// 攻撃判定の位置調整
				{
					if (auto* node = renderer->FindNode("mixamorig:LeftHandMiddle1"))
					{
						// 攻撃アニメーション中
						DirectX::XMFLOAT4X4 attackTransform = node->globalTransform;
						DirectX::XMFLOAT3 attackPosition = DirectX::XMFLOAT3(attackTransform._41, attackTransform._42, attackTransform._43);
						attackPosition.x *= GetOwner()->transform->GetWorldScale().x;
						attackPosition.y *= GetOwner()->transform->GetWorldScale().y;
						attackPosition.z *= GetOwner()->transform->GetWorldScale().z;
						collider->center = Vector3(attackPosition);
					}
				}
			}
		}
		else
		{
			if (previousMoving)
			{
				if (renderer->GetCurrentAnimationName() != "walk")
				{
					// 移動中のアニメーションを再生
					renderer->SetAnimation("walk", true);
					renderer->SetAnimationTimeRate(1.4105f);
					renderer->animationBlendTime = 0.1f;
					renderer->SetLoop(true);

					// ビートに同期させる
					float timeInCurrentBeat = BeatManager::GetTimeInCurrentBeat();
					renderer->time = timeInCurrentBeat + (24.0f / 60.0f); // オフセット調整
					//(60.0f - 24.0f)
				}
			}
			else
			{
				if (renderer->GetCurrentAnimationName() != "idle")
				{
					// 静止中のアニメーションを再生
					renderer->SetAnimation("idle", true);
					renderer->SetAnimationTimeRate(1.0f);
					renderer->animationBlendTime = 0.1f;
					renderer->SetLoop(true);
				}
			}
		}

		if (InputSystem::GetInputState("ok", InputStateMask::Trigger))
		{
			if (renderer->GetCurrentAnimationName() != "box")
			{
				// 攻撃アニメーションを再生
				renderer->SetAnimation("box", false);
				renderer->SetAnimationTimeRate(1.0f);
				renderer->animationBlendTime = 0.05f;
				renderer->SetLoop(false);
				Audio::PlayOneShot(L"./Data/Sounds/SE/box.wav");
				// 攻撃エフェクト再生
				//EffectManager::Play(attackEffectHandle, gameObject->transform->GetPosition(), gameObject->transform->GetEulerAngles());
				if (ParticleComponent* effect = GetOwner()->GetComponentInChildren<ParticleComponent>())
				{
					effect->Play();
				}
				isAttacking = true;

				if (BeatManager::CheckBeatTiming() == BeatResult::Perfect)
				{
					Audio::PlayOneShot(L"./Data/Sounds/SE/punch.wav");
				}

				// 攻撃判定を有効化
				if (auto* collider = GetOwner()->GetComponentInChildren<SphereCollider>())
				{
					collider->SetEnabled(true);
				}
			}
		}
		if (InputSystem::GetInputState("Shift", InputStateMask::Trigger))
		{
			if (renderer->GetCurrentAnimationName() != "dodge")
			{

				//bool canDodge = dodgeCount < 1;
				bool isPerfect = false;

				if (BeatManager::CheckBeatTiming() == BeatResult::Perfect)
				{
					//if (dodgeCount < 3)
					{
						//canDodge = true;
						isPerfect = true;
					}
				}

				// 回避回数をカウント
				//if (canDodge)
				{
					// 回避アニメーションを再生
					renderer->SetAnimation("dodge", true);
					renderer->SetAnimationTimeRate(3.0f);
					renderer->animationBlendTime = 0.25f;
					renderer->SetLoop(false);
					isAttacking = true;
				
				
					// 回避エフェクト再生
					EffectManager::Play(dodgeEffectHandle, GetOwner()->transform->GetPosition(), GetOwner()->transform->GetEulerAngles());
				
				
					if (isPerfect)
					{
						Audio::PlayOneShot(L"./Data/Sounds/SE/dodge.wav");
					}
				}
				
			}
		}
		
	}
}

void CharacterMovement::TakeDamage(int damage)
{
	// ダメージ処理ロジックをここに実装
	// 例: 体力を減少させる、ダメージエフェクトを再生するなど
	currentHealth -= damage;

	if (currentHealth <= 0)
	{
		currentHealth = 0;
		isDead = true;
	}
	Console::Log("Character took " + std::to_string(damage) + " damage. Current Health: " + std::to_string(currentHealth));
}

void CharacterMovement::OnAttack(const TriggerInfo& info)
{
	// 攻撃が発生したときの処理をここに実装
	// 例: 敵にダメージを与える、攻撃エフェクトを再生するなど

	if (GameObject* enemyObj = info.other)
	{
		if (Enemy* enemy = enemyObj->GetComponent<Enemy>())
		{
			// 攻撃エフェクト再生
			EffectManager::Play(damageEffectHandle, info.otherCollider->GetTransform()->GetWorldPosition());

			// 攻撃音再生
			Audio::PlayOneShot(L"./Data/Sounds/SE/hit.wav");

			// 敵にダメージを与える処理など
			enemy->TakeDamage(attackPower); // 例: 20のダメージを与える
			Console::Log("Attacked an enemy for " + std::to_string(attackPower) + " damage.");
			info.selfCollider->SetEnabled(false);
		}
	}
}