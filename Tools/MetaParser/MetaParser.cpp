// MetaParser.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "CSharpGenerater.h"
#include "Parser.h"
#include "Generater.h"
#include <filesystem>
#include <fstream>
#include <iostream>

int main(int argc, char** argv)
{
	std::cout << "argc = " << argc << std::endl;
	for (int i = 0; i < argc; i++)
		std::cout << "arg[" << i << "] = " << argv[i] << std::endl;

	
	if (argc < 3)
    {
        std::cout << "Usage: MetaParser <outputDir> <inputDir...>\n";
        return 1;
	}

	std::string outputJson = argv[1];
	std::vector<std::string> inputDirs;
	for (int i = 2; i < argc; i++)
	{
		inputDirs.push_back(argv[i]);
	}

	// パーサーを作成してディレクトリを解析し、JSON 出力
	Parser parser(outputJson);
	std::vector<FileInfo> allFiles;
	for (const auto& dir : inputDirs)
	{
		auto files = parser.ParseDirectory(dir);
		allFiles.insert(allFiles.end(), files.begin(), files.end());
	}
	
	// コード生成器を作成してコード生成
	Generater generater(outputJson);
	generater.Generate(allFiles);

	// C# コード生成器を作成してコード生成
	std::filesystem::path typeMapPath = std::filesystem::current_path() / "type_map.json";
	std::string typeMapPathStr = std::filesystem::exists(typeMapPath) ? typeMapPath.string() : "";
	if (typeMapPathStr.empty())
	{
		std::cout << "Warning: type_map.json not found in current directory. C# generation will use empty type map.\n";
	}
	else
	{
		CSharpGenerater csGenerater(outputJson, typeMapPathStr);
		csGenerater.Generate(allFiles);
	}

	return 0;
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
