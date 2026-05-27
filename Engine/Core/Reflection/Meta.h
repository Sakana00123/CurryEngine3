#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include "Engine/EditorSupport/AutoRegisterComponent.h"

// C_REFLECT マクロをクラスに付けると、そのクラスのメタ情報が自動的に登録されるようになります。
#define C_REFLECT(ClassName) \
	friend struct ClassName##_AutoRegister;


/**
 * @brief C_PROPERTY マクロをプロパティ宣言の前に付けると、そのプロパティがリフレクションシステムに登録されます。
 * @details C_PROPERTY() の引数に属性を指定できます。例: `C_PROPERTY(Range(0, 100), HideInInspector)`
 * 		属性は `AttributeInfo` としてメタ情報に保存され、エディタなどで利用できます。
 * 		属性の引数は文字列として保存されるため、必要に応じてパースして利用してください。
 * 		属性の例: `Range(0, 100)`, `HideInInspector`, `Tooltip("説明")` など。
 */
#define C_PROPERTY(...) \
public:

// ----------------- C_PROPERTYマクロ内に書ける属性 -----------------
namespace CurryEngine
{
	namespace PropertyAttributes
	{
		// プロパティをインスペクタに表示しない
		struct HideInInspector {};

		// プロパティをシリアライズ対象から除外する
		struct NonSerialized {};

		// プロパティをインスペクタで編集できないようにする
		struct ReadOnly {};

		// プロパティにツールチップを表示する
		struct Tooltip
		{
			const char* text;
			constexpr Tooltip(const char* t) : text(t) {}
		};

		// プロパティの値を指定した範囲内に制限する
		struct Range
		{
			float _min;
			float _max;
			constexpr Range(float min, float max) : _min(min), _max(max) {}
		};
		
		// プロパティの編集速度を指定する（例: ImGui の DragInt/DragFloat で使用）
		struct Speed
		{
			float value;
			constexpr Speed(float v) : value(v) {}
		};

		// プロパティが参照するオブジェクトの型を指定する（例: Component/GameObject など）。エディタでオブジェクト参照のドロップ操作をサポートするために使用します。
		struct ObjectReference
		{
			const char* targetType; // 参照先の型名 (例: "Component", "GameObject")
			constexpr ObjectReference(const char* target) : targetType(target) {}
		};
	}
}

 // ---- リフレクションシステムのメタ情報構造体と登録システム ----

// ---- メタ情報構造体 ----

// 属性情報
struct AttributeInfo
{
	std::string name;					// 属性名 (例: "HideInInspector", "Range", "Tooltip")
	std::vector<std::string> args{};	// 属性引数 (例: Range(0, 100) なら args = {"0", "100"}, Tooltip("説明") なら args = {"説明"})
};

// プロパティのメタ情報
struct PropertyInfo
{
	std::string type;
	std::string name;
	size_t offset;
	std::vector<AttributeInfo> attributes{};


	// 指定した属性情報を取得する関数(無かったらnullptrを返す)
	const AttributeInfo* GetAttribute(const std::string& attrName) const {
		for (auto& attr : attributes) {
			if (attr.name == attrName) {
				return &attr;
			}
		}
		return nullptr;
	}
};

// クラスのメタ情報
struct ClassMeta
{
	std::string name;
	std::vector<std::string> bases;
	std::vector<PropertyInfo> properties;
};

// ---- リフレクション登録システム ----
class ReflectionRegistry
{
public:
	// クラス登録
	static void Register(const ClassMeta& meta);
	// クラス検索
	static const ClassMeta* FindClass(const std::string& name);

private:
	static std::unordered_map<std::string, ClassMeta>& GetRegistry();
};

// ---- マクロ ----
#if 0
#define REGISTER_CLASS(ClassName, BaseName) \
    namespace { struct ClassName##_AutoRegister { \
        static size_t GetOffset(auto ClassName::* member) \
        { \
            return reinterpret_cast<size_t>( \
                &(reinterpret_cast<ClassName*>(0)->*member) \
            ); \
        } \
        ClassName##_AutoRegister() { \
            ClassMeta meta; \
            meta.name = #ClassName; \
            meta.base = #BaseName;  

#endif // 0


#define REGISTER_CLASS(ClassName, ...) \
    namespace { struct ClassName##_AutoRegister { \
        static size_t GetOffset(auto ClassName::* member) \
        { \
            return reinterpret_cast<size_t>( \
                &(reinterpret_cast<ClassName*>(0)->*member) \
            ); \
        } \
        ClassName##_AutoRegister() { \
            ClassMeta meta; \
            meta.name = #ClassName; \
            meta.bases = []{ \
                std::vector<std::string> v; \
                std::string s = #__VA_ARGS__; \
                /* カンマ区切りで分割 */ \
                std::istringstream ss(s); std::string t; \
                while (std::getline(ss, t, ',')) { \
                    auto b = t.find_first_not_of(" \t"); \
                    auto e = t.find_last_not_of(" \t"); \
                    if (b != std::string::npos) v.push_back(t.substr(b, e-b+1)); \
                } \
                return v; \
            }();


#define REGISTER_PROPERTY(ClassName, propName, propType) \
			meta.properties.push_back({ #propType, #propName, GetOffset(&ClassName::propName), {} });

#define ATTR(name, ...) AttributeInfo{ #name, { __VA_ARGS__ } }

#define REGISTER_PROPERTY_WITH_ATTR(ClassName, propName, propType, ...) \
		{ \
            PropertyInfo p; \
			p.type = #propType; \
			p.name = #propName; \
			p.offset = GetOffset(&ClassName::propName); \
			p.attributes = std::vector<AttributeInfo>{ __VA_ARGS__ }; \
			meta.properties.push_back(p); \
		}

#define END_REGISTER(ClassName) \
            ReflectionRegistry::Register(meta); \
        } \
    } ClassName##_AutoRegisterInstance; };