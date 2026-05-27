#include "pch.h"
#include "UIEasing.h"
#include "Engine/Core/GameObject.h"
#include "Engine/Scenes/Scene.h"

// いずれかのマクロを使用してコンポーネントを登録します。必要に応じて属性も指定できます。
REGISTER_COMPONENT(UIEasing, "UserScripts")
//REGISTER_COMPONENT_WITH_ATTRIBUTES(UIEasing, "UserScripts", ComponentAttributes::None, {})


void UIEasing::Start()
{
	// コンポーネントが開始されたときの処理をここに実装します。
}

void UIEasing::Update(float deltaTime)
{
	// 毎フレームの更新処理をここに実装します。
	//if (t >= 0.0f) // t が 0 以上のときにイージングを更新
	{
		float t = 0.0f;
		easingHandler.Update(t, deltaTime);
		if (easingHandler.IsCompleted())
		{
			// ブレンドが完了したときの処理をここに実装します。
		}
		else if (easingHandler.GetSequenceCount() > 0)
		{
			// ブレンド中の処理をここに実装します。
			Vector2 currentPosition = easingHandler.Lerp<Vector2>(startPosition, targetPosition, t);
			// 例えば、currentPosition をオブジェクトの位置に適用するなどの処理を行うことができます。
			RectTransform* rect = GetRectTransform();
			if (rect)
			{
				rect->SetAnchoredPosition(currentPosition);
			}
		}
	}
}

void UIEasing::DrawProperty()
{
#ifdef USE_IMGUI
	// エディタでプロパティを描画する処理をここに実装します。 
	Component::DrawProperty(); // 基底クラスのプロパティ描画を呼び出す（必要に応じて）

	const char* easingTypes[] = {
		"InQuad", "OutQuad", "InOutQuad",
		"InCubic", "OutCubic", "InOutCubic",
		"InQuart", "OutQuart", "InOutQuart",
		"InQuint", "OutQuint", "InOutQuint",
		"InSine", "OutSine", "InOutSine",
		"InExpo", "OutExpo", "InOutExpo",
		"InCirc", "OutCirc", "InOutCirc",
		"InBounce", "OutBounce", "InOutBounce",
		"InBack", "OutBack", "InOutBack",
		"Linear"
	};
	ImGui::Combo("EasingType", &easingType, easingTypes, IM_ARRAYSIZE(easingTypes));

	if (ImGui::Button("Start Easing"))
	{
		StartEasing();
	}
	if (ImGui::Button("Revert Easing"))
	{
		StartEasing(0.0f); // 目標位置を開始位置に設定してイージングを開始
	}

#endif
}

void UIEasing::StartEasing(float t, std::function<void()> onComplete)
{
	// イージングを開始する処理をここに実装します。
	// 例えば、easingHandler にイージング要素を追加して開始するなどの処理を行うことができます。
	this->t =t;
	this->onCompleteCallback = onComplete;
	easingHandler.Clear(); // 既存のシーケンスをクリア
	easingHandler.AddEasing(static_cast<EaseType>(easingType), (1.0f - t), t, easingDuration);
	easingHandler.SetCompletedFunction([this]() {
		// ブレンド中の処理をここに実装します。
		Vector2 currentPosition = easingHandler.Lerp<Vector2>(startPosition, targetPosition, this->t);
		// 例えば、currentPosition をオブジェクトの位置に適用するなどの処理を行うことができます。
		RectTransform* rect = GetRectTransform();
		if (rect)
		{
			rect->SetAnchoredPosition(currentPosition);
		}
		if (onCompleteCallback)
		{
			onCompleteCallback();
		}
		});
}