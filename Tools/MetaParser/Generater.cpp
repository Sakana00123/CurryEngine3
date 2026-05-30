#include "Generater.h"
#include "Parser.h"
#include <filesystem>
#include <fstream>
#include <sstream>
static const std::string generatedExtension = ".generated";

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
		// 元ヘッダーファイルの相対パス
		std::string relativePathFromHeader = std::filesystem::relative(info.path, outputDirectory + "/" + headerDir).string();
		std::string relativePathFromSource = std::filesystem::relative(info.path, outputDirectory + "/" + sourceDir).string();
		// 変換してスラッシュ区切りに
		std::replace(relativePathFromHeader.begin(), relativePathFromHeader.end(), '\\', '/');
		std::replace(relativePathFromSource.begin(), relativePathFromSource.end(), '\\', '/');

		// 生成ソースからソリューションルートへの相対パス (例: "../../")
		//std::string relativeSolutionPath = std::filesystem::relative(std::filesystem::current_path(), std::filesystem::absolute(relativePathFromSource)).string();
		//std::replace(relativeSolutionPath.begin(), relativeSolutionPath.end(), '\\', '/');

		// 出力ディレクトリからの相対パスを計算
		for (const auto& classInfo : info.classes)
		{
			if (!classInfo.reflect) continue;
			std::filesystem::path headerOutputDir = std::filesystem::path(outputDirectory) / headerDir;
			std::filesystem::path sourceOutputDir = std::filesystem::path(outputDirectory) / sourceDir;
			if (!std::filesystem::exists(headerOutputDir))
			{
				std::filesystem::create_directories(headerOutputDir);
			}
			if (!std::filesystem::exists(sourceOutputDir))
			{
				std::filesystem::create_directories(sourceOutputDir);
			}
			std::string headerPath = (headerOutputDir / std::filesystem::path(classInfo.name + generatedExtension + ".h")).string();
			std::string sourcePath = (sourceOutputDir / std::filesystem::path(classInfo.name + generatedExtension + ".cpp")).string();
			
			std::filesystem::path relativeSolutionPath = std::filesystem::relative(std::filesystem::current_path(), std::filesystem::absolute(sourcePath)).parent_path();
			std::string relativeSolutionPathStr = relativeSolutionPath.string();
			std::replace(relativeSolutionPathStr.begin(), relativeSolutionPathStr.end(), '\\', '/');

			GenerateHeader(classInfo, headerPath, relativePathFromHeader);
			GenerateSource(classInfo, sourcePath, relativePathFromSource, relativeSolutionPathStr);
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
				ofs << "#include \"" << headerDir << "/" << classInfo.name << generatedExtension + ".h\"\n";
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
	for (const auto& method : info.methods)
	{
		ofs << "    REGISTER_METHOD(" 
			<< info.name << ", "
			<< method.name << ", "
			<< method.returnType;
		
		if (!method.parameters.empty())
		{
			ofs << ", ";
			for (size_t i = 0; i < method.parameters.size(); i++)
			{
				const auto& param = method.parameters[i];
				ofs << param.first << " " << param.second;
				if (i < method.parameters.size() - 1)
					ofs << ", ";
			}
		}
		else
		{
			ofs << ", void"; // 引数なしは void として扱う
		}
		
		ofs << ")\n";
	}
	ofs << "END_REGISTER(" << info.name << ")\n";
	ofs.close();
}

void Generater::GenerateSource(const ClassInfo& info, const std::string& outPath, const std::string& includePath, const std::string& relativeSolutionPath)
{
	std::ofstream ofs(outPath);
	if (!ofs.is_open())
	{
		throw std::runtime_error("Failed to open file for writing: " + outPath);
	}
	// クラス名
	std::string className = info.name;
	
	//ofs << "#include \"" << relativeSolutionPath << "/pch.h" << "\"\n";
	ofs << "#include \"" << includePath << "\"\n";
	ofs << "#include \"" << relativeSolutionPath << "/Engine/Scenes/Scene.h" << "\"\n";
	ofs << "#include \"" << relativeSolutionPath << "/Engine/Scenes/SceneManager.h" << "\"\n";
	// API 関数の実装はここに書く（例: オブジェクトIDからコンポーネントを取得する関数など）

	ofs << "// 例: " << className << " のオブジェクトIDからコンポーネントを取得する関数\n";
	ofs << "static " << className << "* Get" << className << "ById(uint64_t objectId)\n{\n";
	ofs << "    Scene* scene = SceneManager::GetLoadingSceneOrCurrentScene();\n";
	ofs << "    if (!scene) return nullptr;\n";
	ofs << "    const auto& compMap = scene->objectManager->GetComponentCacheMap();\n";
	ofs << "    if (compMap.contains(ObjectId::FromValue(objectId))) {\n";
	ofs << "        if (auto compPtr = compMap.at(ObjectId::FromValue(objectId)).lock()) {\n";
	ofs << "            if (auto ptr = dynamic_cast<" << className << "*>(compPtr.get())) {\n";
	ofs << "                return ptr;\n";
	ofs << "            }\n";
	ofs << "        }\n";
	ofs << "    }\n";
	ofs << "    return nullptr;\n";
	ofs << "}\n";

	// フィールドの Get/Set 関数も同様に実装していく
	for (const auto& field : info.fields)
	{
		// 先頭大文字のフィールド名
		std::string capName = field.name;
		std::string fieldName = field.name;
		if (!capName.empty()) capName[0] = static_cast<char>(std::toupper(capName[0]));
		// フィールドの型
		std::string type = field.type;

		// Get 関数
		ofs << "// 例: " << className << " の " << fieldName << " フィールドの Get 関数\n";
		ofs << "extern \"""C\"" << " __declspec(dllexport) " << type << " " << className << "_Get" << capName << "(uint64_t objectId)\n{\n";
		ofs << "    if (auto comp = Get" << className << "ById(objectId)) {\n";
		ofs << "        return comp->" << fieldName << ";\n";
		ofs << "    }\n";
		ofs << "    return " << type << "{}; // オブジェクトが見つからない場合の戻り値\n";
		ofs << "}\n";
		// Set 関数
		ofs << "// 例: " << className << " の " << fieldName << " フィールドの Set 関数\n";
		ofs << "extern \"""C\"" << " __declspec(dllexport) void " << className << "_Set" << capName << "(uint64_t objectId, "
			<< type << " value)\n{\n";
		ofs << "    if (auto comp = Get" << className << "ById(objectId)) {\n";
		ofs << "        comp->" << fieldName << " = value;\n";
		ofs << "    }\n";
		ofs << "}\n";
	}

	// 他の API 関数も同様に実装していく
	for (const auto& method : info.methods)
	{
		ofs << "// 例: " << method.name << " API 関数の実装\n";
		ofs << "extern \"""C\"" << " __declspec(dllexport) " << method.returnType << " " << className << "_" << method.name << "(uint64_t objectId";
		for (const auto& param : method.parameters)
		{
			ofs << ", " << param.first << " " << param.second;
		}
		ofs << ")\n{\n";
		ofs << "    if (auto comp = Get" << className << "ById(objectId)) {\n";
		ofs << "        // comp を使って API の処理を実装\n";
		// メソッドの呼び出し
		if (method.returnType != "void")
		{
			ofs << "        return comp->" << method.name << "(";
		}
		else
		{
			ofs << "        comp->" << method.name << "(";
		}
		// 引数を渡す
		for (size_t i = 0; i < method.parameters.size(); i++)
		{
			const auto& param = method.parameters[i];
			ofs << param.second;
			if (i < method.parameters.size() - 1)
				ofs << ", ";
		}
		ofs << ");\n";
		ofs << "    }\n";
		if (method.returnType != "void")
		{
			ofs << "    return " << method.returnType << "{}; // オブジェクトが見つからない場合の戻り値\n";
		}
		ofs << "}\n";
	}


	
	ofs.close();
}