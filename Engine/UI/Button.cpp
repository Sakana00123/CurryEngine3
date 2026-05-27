#include "pch.h"
#include "Button.h"
#include "Engine/Rendering/Pipeline/Graphics.h"

REGISTER_COMPONENT(Button, "UI")

void Button::Initialize()
{
	//image = gameObject->GetComponent<Image>();
}

void Button::Update(float deltaTime)
{
	// customClickFunc が設定されている場合、呼び出して true を返した場合にクリック処理を実行
    if (customClickFunc)
    {
        if (customClickFunc())
        {
            OnClick();
        }
	}
}

void Button::UpdateInfo(EventInfo& info)
{
	if (info.funcId == 1)
	{
		info.pFunc =
			[info]() {
			info.pObj->SetActive(info.flag);
			};
	}
	else {
		info.pFunc = nullptr;
	}
}

void Button::OnClick()
{
	// クリックが有効か確認
	bool canClick = true;
	if (clickFlagFunc)
    {
		canClick = clickFlagFunc();
	}
	if (canClick)
    {
        // 登録された関数を順に呼び出す
        for (auto& func : onClickFunctions) {
            if (func) {
                func();
            }
        }
        // インスペクタで設定されたイベントを順に処理
        for (auto& info : eventInfo) {
            if (info.pFunc) {
                info.pFunc();
            }
        }
    }
}

void Button::DrawProperty() {
#ifdef USE_IMGUI
    Selectable::DrawProperty();
    ImGui::Checkbox("pressing", &isPressed);
#if 0

    ImGui::NewLine();

    // イベント
    {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::Selectable("OnClick()", true);
        ImGui::PopStyleColor(3);
        //ImVec2 size = ImVec2(250.f, droppedObjects.empty() ? 30.f : 30.f * droppedObjects.size());
        if (ImGui::BeginChild("OnClickEvent", ImVec2(250, 150), true)) {

            for (size_t i = 0; i < eventInfo.size(); i++) {
                ImGui::PushID(static_cast<int>(i));
                // 要素の間に区切り線をつける
                if (i > 0) {
                    ImGui::Separator();
                }
                // ドロップ先
                ImGui::Button(eventInfo[i].pObj ? eventInfo[i].pObj->name.c_str() : "None(GameObject)");
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) {
                        IM_ASSERT(payload->DataSize == sizeof(ObjectId*));
                        ObjectId* pId = static_cast<ObjectId*>(payload->Data);
                        eventInfo[i].pObj = ObjectManager::Find(*pId);
                    }
                    ImGui::EndDragDropTarget();
                }

                //ImGui::SameLine();

                // 関数リスト
                const char* items[] = {
                    "No Function",
                    "SetActive",
                    //"No Function",
                    //"No Function",
                    //"No Function",
                };

                if (ImGui::BeginCombo("func", items[eventInfo[i].funcId])) {
                    if (eventInfo[i].pObj)
                    {
                        for (int j = 0; j < IM_ARRAYSIZE(items); j++) {
                            bool isSelected = eventInfo[i].funcId == j;
                            if (ImGui::Selectable(items[j], isSelected)) {
                                eventInfo[i].funcId = j;

                                if (eventInfo[i].funcId == 1) {
                                    UpdateInfo(eventInfo[i]);
                                }
                            }

                            if (isSelected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
                // SetActive（ユーザー側で設定可能なbool）
                if (eventInfo[i].funcId == 1) {
                    if (ImGui::Checkbox("SetActive", &eventInfo[i].flag)) {
                        UpdateInfo(eventInfo[i]);
                    }
                }
                ImGui::PopID();
            }

            if (eventInfo.empty()) {
                ImGui::Text("List is Empty");
            }
        }
        ImGui::EndChild();


        if (ImGui::Button("+", ImVec2(30, 30)) && eventInfo.size() < eventInfo.max_size()) {
            eventInfo.resize(eventInfo.size() + 1);
        }
        ImGui::SameLine();
        if (ImGui::Button("-", ImVec2(30, 30)) && eventInfo.size() > 0) {
            eventInfo.resize(eventInfo.size() - 1);
        }
    }
#endif // 0


#endif // USE_IMGUI
}