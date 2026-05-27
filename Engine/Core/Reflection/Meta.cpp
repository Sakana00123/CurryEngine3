#include "pch.h"
#include "Meta.h"
#include "Engine/Editor/Console.h"

void ReflectionRegistry::Register(const ClassMeta& meta)
{
	GetRegistry()[meta.name] = meta;
	Console::Log("class: " + meta.name + ", fields: " + std::to_string(meta.properties.size()));
}

const ClassMeta* ReflectionRegistry::FindClass(const std::string& name)
{
	auto it = GetRegistry().find(name);
	if (it != GetRegistry().end())
	{
		return &it->second;
	}
	return nullptr;
}

std::unordered_map<std::string, ClassMeta>& ReflectionRegistry::GetRegistry()
{
	static std::unordered_map<std::string, ClassMeta> registry;
	return registry;
}