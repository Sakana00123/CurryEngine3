#pragma once
#include <string>
#include <vector>
struct FileInfo;
struct ClassInfo;

class Generater
{
public:
	Generater(const std::string& outputDir);

	// クラス情報からコードを生成
	void Generate(const std::vector<FileInfo>& files);
private:
	std::string outputDirectory;
	
	void GenerateHeader(const ClassInfo& info, const std::string& outPath, const std::string& includePath);
	void GenerateSource(const ClassInfo& info, const std::string& outPath);
};