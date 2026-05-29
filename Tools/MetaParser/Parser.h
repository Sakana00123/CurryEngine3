#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

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

struct FileInfo
{
	std::string path;
	std::vector<ClassInfo> classes;
};

class Parser
{
public:
	Parser(const std::string& outputDir = "./") : outputDirectory(outputDir) {}

	std::vector<FileInfo> ParseDirectory(const std::string& dirPath);
	FileInfo ParseFile(const std::string& filePath);
	std::vector<ClassInfo> ExtractClasses(const std::string& text);
	void ExtractFields(const std::string& text, size_t classPos, ClassInfo& info);
	void ExtractMethods(const std::string& text, size_t classPos, ClassInfo& info);

	std::pair<size_t, size_t> FindClassBlock(const std::string& text, size_t classPos);
	bool ContainsReflectMacro(const std::string& text, size_t begin, size_t end);

	void WriteJson(const ClassInfo& info, const std::string& outPath);

private:
	std::string outputDirectory;

	//void ParseHeader(const std::string& path);
	//void ExportJson(const std::string& outPath);
};