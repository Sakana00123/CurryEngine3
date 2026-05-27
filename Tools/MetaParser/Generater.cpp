#include "Generater.h"
#include "Parser.h"
#include <filesystem>
#include <fstream>
#include <sstream>

Generater::Generater(const std::string& outputDir)
	: outputDirectory(outputDir)
{
	if (!std::filesystem::exists(outputDirectory))
	{
		std::filesystem::create_directories(outputDirectory);
	}
}

void Generater::Generate(const std::vector<FileInfo>& files)
{
	// 各ファイルごとに処理
	for (const auto& info : files)
	{
		// 元ヘッダーファイルのパス(#include 用)
		std::string includePath = std::filesystem::relative(info.path, outputDirectory).string();
		// 変換してスラッシュ区切りに
		std::replace(includePath.begin(), includePath.end(), '\\', '/');

		// 出力ディレクトリからの相対パスを計算
		for (const auto& classInfo : info.classes)
		{
			if (!classInfo.reflect) continue;
			std::string headerPath = outputDirectory + "/" + classInfo.name + ".reflect.h";
			std::string sourcePath = outputDirectory + "/" + classInfo.name + ".reflect.cpp";
			GenerateHeader(classInfo, headerPath, includePath);
			//GenerateSource(classInfo, sourcePath);
		}
	}

	// ReflectionGenerated.h を生成
	{
		std::ofstream ofs(outputDirectory + "/ReflectionGenerated.h");
		ofs << "#pragma once\n\n";

		for (const auto& info : files)
		{
			for (const auto& classInfo : info.classes)
			{
				if (!classInfo.reflect) continue;
				ofs << "#include \"" << classInfo.name << ".reflect.h\"\n";
			}
		}
	}
}

void Generater::GenerateHeader(const ClassInfo& info, const std::string& outPath, const std::string& includePath)
{
	std::ofstream ofs(outPath);
	if (!ofs.is_open())
	{
		throw std::runtime_error("Failed to open file for writing: " + outPath);
	}
	ofs << "#pragma once\n\n";
	ofs << "#include \"" << includePath << "\"\n";
	ofs << "#include \"" << "Engine/Core/Reflection/Meta.h" << "\"\n";

	ofs << "REGISTER_CLASS(" << info.name << ", ";
	for (size_t i = 0; i < info.bases.size(); i++)
	{
		ofs << info.bases[i];
		if (i < info.bases.size() - 1)
			ofs << ", ";
	}
	ofs << ")\n";
	for (const auto& field : info.fields)
	{
		if (field.attributes.empty())
		{
			// 属性がない場合は通常のマクロを使用
			ofs << "    REGISTER_PROPERTY(" 
				<< info.name << ", "
				<< field.name << ", "
				<< field.type << ")\n";
		}
		else
		{
			// 属性がある場合は属性付きマクロを使用
			ofs << "    REGISTER_PROPERTY_WITH_ATTR("
				<< info.name << ", "
				<< field.name << ", "
				<< field.type << ", ";

			// 属性リストを出力
			for (size_t i = 0; i < field.attributes.size(); i++)
			{
				const auto& attr = field.attributes[i];
				ofs << "ATTR(" << attr.name;
				for (const auto& arg : attr.args)
				{
					ofs << ", \"" << arg << "\"";
				}
				ofs << ")";
				if (i < field.attributes.size() - 1)
				{
					ofs << ", ";
				}
			}
			ofs << ")\n";
		}
	}
	ofs << "END_REGISTER(" << info.name << ")\n";
	ofs.close();
}

void Generater::GenerateSource(const ClassInfo& info, const std::string& outPath)
{
	std::ofstream ofs(outPath);
	if (!ofs.is_open())
	{
		throw std::runtime_error("Failed to open file for writing: " + outPath);
	}
	ofs << "#include \"" << info.name << ".reflect.h\"\n";
	
	ofs.close();
}