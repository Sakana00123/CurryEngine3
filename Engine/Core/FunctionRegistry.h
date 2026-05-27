#pragma once

#include <string>
#include <functional>

#include <unordered_map>
#include <vector>

class FunctionRegistry;

struct FunctionMeta
{
	std::string name;
	std::function<void()> callback;

	//FunctionMeta(const std::string& name, std::function<void()> callback)
	//	: name(name), callback(callback) {
	//	FunctionRegistry::Register(*this);
	//}
};

class FunctionRegistry
{
public:

	static void Register(const FunctionMeta& meta) {
		registry[meta.name] = meta;
	}

	static const std::unordered_map<std::string, FunctionMeta>& GetAll() {
		return registry;
	}

	static void Invoke(const std::string& name) {
		auto it = registry.find(name);
		if (it != registry.end()) {
			it->second.callback();
		}
	}
	
private:
	static inline std::unordered_map<std::string, FunctionMeta> registry;
};