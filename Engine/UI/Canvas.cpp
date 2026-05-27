#include "pch.h"
#include "Canvas.h"
#include "Graphic.h"

REGISTER_COMPONENT_WITH_ATTRIBUTES(Canvas, "UI", ComponentAttributes::DisallowMultiple | ComponentAttributes::ExecuteInEditMode | ComponentAttributes::RequiredComponent, { "RectTransform" });

std::vector<Graphic*> Canvas::GetGraphics() const {
#if 0
	return this->graphics;
#else
	std::vector<std::pair<Graphic*, int>> graphicWithZ; // Graphic とその描画順をペアで保持
	int parentZ = 0;
	std::function<void(GameObject*)> findGraphic = [&](GameObject* object) {
		auto* rect = object->GetComponent<RectTransform>();
		if (!rect) {
			return;
		}
		// 親のソーティングオーダー + 自身のローカルソーティングオーダーで描画順を決定
		int myZ = parentZ + rect->localSortingOrder;
		parentZ = myZ; // 子の探索に備えて更新

		if (Graphic* graphic = object->GetComponent<Graphic>())
		{
			graphicWithZ.push_back({ graphic, myZ });
		}
		for (GameObject* child : object->children)
		{
			findGraphic(child);
		}
		};
	findGraphic(gameObject);

	// 描画順でソートして Graphic* のリストを作成
	std::vector<Graphic*> graphics;
	std::stable_sort(graphicWithZ.begin(), graphicWithZ.end(), [](const auto& a, const auto& b) {
		return a.second < b.second; // 描画順でソート
		});
	for (const auto& pair : graphicWithZ) {
		graphics.push_back(pair.first);
	}
	return graphics;
#endif
}