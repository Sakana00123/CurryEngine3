#include "Parser.h"
#include <regex>
#include <fstream>
#include <iostream>
#include <sstream>

#include "../../External/tinygltf-release/json.hpp"
#include <windows.h>

static std::string ShiftJisToUtf8(const std::string& sjis)
{
    // Shift-JIS → UTF-16
    int wlen = MultiByteToWideChar(CP_ACP, 0, sjis.c_str(), -1, nullptr, 0);
    std::wstring wide(wlen, L'\0');
    MultiByteToWideChar(CP_ACP, 0, sjis.c_str(), -1, wide.data(), wlen);

    // UTF-16 → UTF-8
    int ulen = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8(ulen, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), ulen, nullptr, nullptr);

    return utf8;
}

static std::string RemoveComments(const std::string& s)
{
	std::string out = s;
	// "//" のコメントを削除
    size_t pos = out.find("//");
    while (pos != std::string::npos)
    {
        size_t end = out.find('\n', pos + 2);
        if (end == std::string::npos) break; // 行末までがコメントの場合は終了
        out.erase(pos, end - pos);
        pos = out.find("//");
	}
	// "/* */"のコメントを削除
	size_t start = out.find("/*");
    while (start != std::string::npos)
    {
        size_t end = out.find("*/", start + 2);
        if (end == std::string::npos) break; // 対応する終了がない場合は終了
        out.erase(start, end - start + 2);
        start = out.find("/*");
	}
	return out;
}

static void Trim(std::string& s)
{
	s.erase(0, s.find_first_not_of(" \t\n\r"));
	s.erase(s.find_last_not_of(" \t\n\r") + 1);
}

// C_PROPERTY(...) の (...) 内を取得する
static std::string ExtractMacroArgs(const std::string& text, size_t macroPos)
{
	int depth = 0;
    size_t open = text.find('(', macroPos);
	if (open == std::string::npos) return "";
    size_t pos = open;
    while (pos < text.size())
    {
        switch (text[pos])
        {
            case '(': depth++; break;
            case ')': depth--; break;
            default: break;
        }
        if (depth == 0) break;
        pos++;
    }
    if (depth != 0) return ""; // 括弧の対応が取れていない
	return text.substr(open + 1, pos - open - 1);
}

// "Range(0, 1), HideInInspector, Tooltip("abc")" をパース
static std::vector<AttributeInfo> ParseAttributes(const std::string& attrStr)
{
    std::vector<AttributeInfo> attrs;
    if (attrStr.empty()) return attrs;

    // カンマ区切りだが括弧内のカンマは無視する
    std::vector<std::string> tokens;
    int depth = 0;
    std::string cur;
    for (char c : attrStr)
    {
        if (c == '(') depth++;
        else if (c == ')') depth--;
        if (c == ',' && depth == 0)
        {
            Trim(cur);
            if (!cur.empty()) tokens.push_back(cur);
            cur.clear();
        }
        else cur += c;
    }
    Trim(cur);
    if (!cur.empty()) tokens.push_back(cur);

	// 各トークン (例: "Range(0, 1)", "HideInInspector", "Tooltip("abc")") をパース(名前空間を考慮)
	std::regex attrRegex(R"(([\w:]+)(?:\((.*)\))?)"); // 名前空間を含む属性名と引数をキャプチャ
    for (auto& token : tokens)
    {
        std::smatch m;
        if (!std::regex_search(token, m, attrRegex)) continue;
        AttributeInfo a;
        a.name = m[1].str();
		std::cout << "Parsing attribute: " << a.name << ", args: " << (m[2].matched ? m[2].str() : "none") << "\n";
		// 名前空間を除去 (例: CurryEngine::PropertyAttributes::Range -> Range)
        while (a.name.find("::") != std::string::npos)
        {
            a.name = a.name.substr(a.name.find("::") + 2); // 名前空間を除去
			std::cout << "warning:  Stripped namespace, now: " << a.name << "\n";
        }

        // 引数があれば分割
        if (m[2].matched)
        {
			std::string argsStr = m[2].str();
			// コメントを除去
			//argsStr = RemoveComments(argsStr);

			// カンマ区切りで引数を分割。ただし、括弧内のカンマは無視するため、再度同様の処理を行う
            std::stringstream ss(argsStr);
            std::string arg;
            while (std::getline(ss, arg, ','))
            {
                Trim(arg);
                // クォートを除去
                if (arg.size() >= 2 && arg.front() == '"' && arg.back() == '"')
                    arg = arg.substr(1, arg.size() - 2);
                a.args.push_back(arg);
            }
        }

		// デバッグ出力
        if (a.args.empty())
            std::cout << "  Attribute: " << a.name << " (no args)\n";
        else
        {
            std::cout << "  Attribute: " << a.name << ", args: ";
            for (const auto& arg : a.args)
				std::cout << arg << " ";
			std::cout << "\n";
        }
        attrs.push_back(a);
    }
    return attrs;
}


std::vector<FileInfo> Parser::ParseDirectory(const std::string& dirPath)
{
    std::vector<FileInfo> result;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath))
    {
		if (!entry.is_regular_file()) continue;

		auto path = entry.path();

        if (path.extension() == ".h" || path.extension() == ".hpp")
        {
			result.push_back(ParseFile(path.string()));
        }
    }
    return result;
}

FileInfo Parser::ParseFile(const std::string& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open())
        return {};

    std::string content((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());

	content = ShiftJisToUtf8(content); // Shift-JIS を UTF-8 に変換

    FileInfo fileInfo;
    fileInfo.path = path;
    for (auto& c : ExtractClasses(RemoveComments(content)))
    {
        fileInfo.classes.push_back(c);
    }
    return fileInfo;
}


std::vector<ClassInfo> Parser::ExtractClasses(const std::string& text)
{
    std::vector<ClassInfo> classes;
	auto textBegin = text.begin();
	auto textEnd = text.end();
    
    // クラス定義の正規表現(class/struct 名、基底クラス)
    // コメントを考慮し、前方宣言を除外: '{' が続くもののみマッチ
    //std::regex classRegex(
    //    R"((class|struct)\s+(\w+)\s*(?:\s*:\s*public\s+(\w+))?\s*(?://[^\n]*)?\s*\{)"
    //);
    std::regex classRegex(
        R"((class|struct)\s+(\w+)\s*((?:\s*:\s*(?:public|protected|private)\s+\w+(?:\s*,\s*(?:public|protected|private)\s+\w+)*)?)?\s*(?://[^\n]*)?\s*\{)"
    );

    // 正規表現イテレータを作成
    auto begin = std::sregex_iterator(textBegin, textEnd, classRegex);
    auto end = std::sregex_iterator();

    // クラスごとに処理
    for (auto it = begin; it != end; ++it)
    {
        // クラス情報を作成(名前、基底クラス)
        ClassInfo info;
        info.name = (*it)[2].str();
        //info.base = (*it)[3].matched ? (*it)[3].str() : "";  // 継承なし対応
        // キャプチャグループ3に ": public A, public B" が入るので分割する
        std::string inheritStr = (*it)[3].str(); // 例: ": public Foo, public Bar"
        std::regex baseRegex(R"((?:public|protected|private)\s+(\w+))");
        auto bIt = std::sregex_iterator(inheritStr.begin(), inheritStr.end(), baseRegex);
        auto bEnd = std::sregex_iterator();
        for (; bIt != bEnd; ++bIt)
            info.bases.push_back((*bIt)[1].str());

        // class ブロックを探す
        size_t classPos = it->position(0);

        // class {} ブロックの範囲を取得
        auto [blockStart, blockEnd] = FindClassBlock(text, classPos);
        // C_REFLECT() マクロがあるかどうか
        info.reflect = ContainsReflectMacro(text, blockStart, blockEnd);
        if (!info.reflect)
        {
            std::cout << "Skipping class (no C_REFLECT): " << info.name << ", classPos" << classPos << ", block [" << blockStart << ", " << blockEnd << "]\n";
            continue; // C_REFLECT マクロがないならスキップ
        }

        std::cout << "Found class: " << info.name;
        for (const auto& base : info.bases)
			std::cout << ", base: " << base;
		std::cout << ", classPos: " << classPos << ", block [" << blockStart << ", " << blockEnd << "]\n";

        // フィールドを抽出
        ExtractFields(text, classPos, info);

		// メソッドを抽出
		ExtractMethods(text, classPos, info);

        // JSON 出力
        std::string outPath = outputDirectory + "/" + info.name + ".json";
        WriteJson(info, outPath);

        // クラス情報を追加
        classes.push_back(info);
    }
    return classes;
}

void Parser::ExtractFields(const std::string& text, size_t classPos, ClassInfo& info)
{
    size_t pos = classPos;

    while (true)
    {
        // C_PROPERTY() を探す
        pos = text.find("C_PROPERTY", pos);
        if (pos == std::string::npos) break;

		// マクロ引数を抽出して属性情報を解析
		std::string attrStr = ExtractMacroArgs(text, pos);
		auto attributes = ParseAttributes(attrStr);

        // 次の行を取得
        size_t lineStart = text.find('\n', pos);
        if (lineStart == std::string::npos) break;

        lineStart++;
        size_t lineEnd = text.find(';', lineStart);
        if (lineEnd == std::string::npos) break;

        std::string line = text.substr(lineStart, lineEnd - lineStart);
        Trim(line);

		// フィールドの型と名前を抽出
		std::regex fieldRegex(R"(([A-Za-z0-9_:<>]+)\s+([A-Za-z0-9_]+)\s*)"); // 型と名前の正規表現(例: int myField)
		std::smatch m;
        if (!std::regex_search(line, m, fieldRegex))
        {
			std::cout << "  Warning: Could not parse field line: " << line << "\n";

            pos = lineEnd;
            continue; // マッチしなければ次へ
		}
		// マッチした場合
		std::cout << "  Field found: " << m[1].str() << " " << m[2].str() << "\n";

		// 型と名前を取得
		FieldInfo field;
        field.type = m[1].str();
        field.name = m[2].str();
		field.attributes = attributes;
        
		// フィールド情報を追加
        info.fields.push_back(field);

		// 次の位置へ
        pos = lineEnd;
    }
}

void Parser::ExtractMethods(const std::string& text, size_t classPos, ClassInfo& info)
{
	size_t pos = classPos;

    while (true)
    {
        // C_FUNCTION() を探す
        pos = text.find("C_FUNCTION", pos);
        if (pos == std::string::npos) break;
        // 次の行を取得
        size_t lineStart = text.find('\n', pos);
        if (lineStart == std::string::npos) break;
        lineStart++;
        size_t lineEnd = text.find(';', lineStart);
        if (lineEnd == std::string::npos) break;
        std::string line = text.substr(lineStart, lineEnd - lineStart);
        Trim(line);

        // メソッドの戻り値、名前、引数を抽出
        std::regex methodRegex(R"(([A-Za-z0-9_:<>]+)\s+([A-Za-z0-9_]+)\s*\((.*)\))"); // 戻り値、名前、引数の正規表現(例: void myMethod(int a, float b))
        std::smatch m;
        if (!std::regex_search(line, m, methodRegex))
        {
            std::cout << "  Warning: Could not parse method line: " << line << "\n";
            pos = lineEnd;
            continue; // マッチしなければ次へ
        }

        // マッチした場合
        std::cout << "  Method found: " << m[1].str() << " " << m[2].str() << ", args: " << m[3].str() << "\n";
        // 戻り値、名前、引数を取得
        MethodInfo method;
        method.returnType = m[1].str();
        method.name = m[2].str();
        // 引数を分割して型と名前を抽出
        std::string argsStr = m[3].str();
        std::regex argRegex(R"(([A-Za-z0-9_:<>]+)\s+([A-Za-z0-9_]+))"); // 引数の型と名前の正規表現(例: int a)
        auto aIt = std::sregex_iterator(argsStr.begin(), argsStr.end(), argRegex);
        auto aEnd = std::sregex_iterator();
        for (; aIt != aEnd; ++aIt)
        {
            std::string argType = (*aIt)[1].str();
            std::string argName = (*aIt)[2].str();
            method.parameters.emplace_back(argType, argName);
            std::cout << "    Parameter: " << argType << " " << argName << "\n";
        }
        // メソッド情報を追加
        info.methods.push_back(method);
        // 次の位置へ
        pos = lineEnd;

    }


}

std::pair<size_t, size_t> Parser::FindClassBlock(const std::string& text, size_t classPos)
{
    size_t braceOpenPos = text.find('{', classPos);
    if (braceOpenPos == std::string::npos)
    {
		std::cout << "  Warning: Could not find opening brace for class at position " << classPos << "\n";
        return { std::string::npos, std::string::npos };
    }
    size_t braceClosePos = braceOpenPos;
    int braceCount = 1;
    while (braceCount > 0)
    {
        braceClosePos++;
        if (braceClosePos >= text.size())
        {
            return { braceOpenPos, std::string::npos };
        }
        if (text[braceClosePos] == '{')
            braceCount++;
        else if (text[braceClosePos] == '}')
            braceCount--;
    }
	return { braceOpenPos, braceClosePos };
}

bool Parser::ContainsReflectMacro(const std::string& text, size_t begin, size_t end)
{
    size_t pos = text.find("C_REFLECT", begin);
    return (pos != std::string::npos && pos < end);
}

void Parser::WriteJson(const ClassInfo& info, const std::string& outPath)
{
    nlohmann::json j;
	j["name"] = info.name;
    j["bases"] = info.bases;
    for (const auto& f : info.fields)
    {
		auto& fieldJson = j["fields"].emplace_back();
		fieldJson["type"] = f.type;
		fieldJson["name"] = f.name;
		// 属性も追加
		nlohmann::json attrArray = nlohmann::json::array();
        for (const auto& attr : f.attributes)
        {
			auto& attrJson = attrArray.emplace_back();
			attrJson["name"] = attr.name;
			attrJson["args"] = attr.args;
        }
		fieldJson["attributes"] = attrArray;
    }
    for (const auto& m : info.methods)
	{
        auto& methodJson = j["methods"].emplace_back();
        methodJson["returnType"] = m.returnType;
        methodJson["name"] = m.name;
        nlohmann::json paramArray = nlohmann::json::array();
        for (const auto& param : m.parameters)
        {
            auto& paramJson = paramArray.emplace_back();
            paramJson["type"] = param.first;
            paramJson["name"] = param.second;
        }
        methodJson["parameters"] = paramArray;
	}
	std::cout << "Writing JSON to: " << outPath << std::endl;
    std::ofstream(outPath) << j.dump(2);
}