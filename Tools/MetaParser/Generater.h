#pragma once
#include <string>
#include <vector>
#include "ParseInfo.h"

class Generater
{
public:
	Generater(const std::string& outputDir);

	// クラス情報からコードを生成
	void Generate(const std::vector<FileInfo>& files);
private:
	std::string outputDirectory;
	std::string headerDir = "Reflection"; // ヘッダーファイルの出力サブディレクトリ
	std::string sourceDir = "Interop"; // ソースファイルの出力サブディレクトリ
	
	void GenerateHeader(const ClassInfo& info, const std::string& outPath, const std::string& includePath);
	void GenerateSource(const ClassInfo& info, const std::string& outPath, const std::string& includePath, const std::string& relativeSolutionPath = "");
};