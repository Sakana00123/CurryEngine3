#pragma once
#include <string>
#include <vector>

struct AttributeInfo
{
	std::string name;
	std::vector<std::string> args;
};

struct FieldInfo
{
	std::string type;
	std::string name;
	std::vector<AttributeInfo> attributes;
};

struct MethodInfo
{
	std::string returnType;
	std::string name;
	std::vector<std::pair<std::string, std::string>> parameters; // type, name
	std::vector<AttributeInfo> attributes;
};

struct ClassInfo
{
	std::string name;
	std::vector<std::string> bases;
	std::vector<FieldInfo> fields;
	std::vector<MethodInfo> methods;
	bool reflect = false;
};

struct EnumValueInfo
{
	std::string name;
	int         value = 0;
	bool        hasExplicitValue = false; // ’l‚ª–¾Ž¦‚³‚ê‚Ä‚¢‚é‚©
};

struct EnumInfo
{
	std::string name;
	std::string underlyingType = "int"; // enum class : uint ‚È‚Ç
	bool        isClass = false; // enum class ‚©‚Ç‚¤‚©
	std::vector<EnumValueInfo> values;
};

struct StructInfo
{
	std::string name;
	std::vector<FieldInfo> fields;
};

struct FileInfo
{
	std::string path;
	std::vector<ClassInfo> classes;
	std::vector<EnumInfo> enums;
	std::vector<StructInfo> structs;
};