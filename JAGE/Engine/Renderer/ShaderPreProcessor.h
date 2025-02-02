#pragma once

#include <Core/Core.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>

#define SHADERS_DIR "Assets/Shaders/"
#define SHADERS_PROC_DIR "Assets/Shaders/PreProcessed/"

struct ShaderFile {
	std::string filename;
	std::vector<std::string> includes;
	std::vector<std::string> code;
};

class ShaderPreProcessor {
public:
	ShaderPreProcessor() = default;

	void Initialize() {
		readShaderFiles();
		preProcessFiles();
		writeFiles();
	}

	std::string GetCodeStr(const std::string& fileName) const {
		const auto& it = procShaders.find(fileName);
		if (it == procShaders.end()) LOG(LogOpenGL, LOG_CRITICAL, std::format("Attempted to load missing shader file {}.", fileName));

		const std::vector<std::string> code = it->second.code;
		if (code.empty()) LOG(LogOpenGL, LOG_CRITICAL, std::format("Loaded shader file {} is missing code.", fileName));

		std::string codeStr;
		for (const auto& line : code) {
			codeStr += line + "\n";
		}

		return codeStr;
	}
	
private:
	std::unordered_map<std::string, ShaderFile> procShaders;

	void readShaderFiles() {
		bool failed = false;
		for (const auto& entry : std::filesystem::directory_iterator(SHADERS_DIR)) {
			if (entry.is_regular_file()) {
				const auto& path = entry.path();
				std::ifstream file(path);

				if (!file.is_open()) {
					LOG(LogOpenGL, LOG_ERROR, std::format("Failed to read shader file at path {}", path.string()));
				}

				std::stringstream buffer;
				std::vector<std::string> code;
				std::string fline;
				while (std::getline(file, fline)) {
					code.push_back(fline);
				}

				const std::string filename = path.filename().string();

				ShaderFile shaderFile;
				shaderFile.filename = filename;
				shaderFile.code = code;
				procShaders[filename] = shaderFile;
			}
		}

		LOG(LogOpenGL, LOG_DEBUG, std::format("Successfully loaded {} shader file(s).", procShaders.size()));
	};

	void preProcessFiles() {
		for (auto& it : procShaders) {
			ShaderFile& sf = it.second;
			if (sf.code.empty()) {
				LOG(LogOpenGL, LOG_CRITICAL, std::format("Attempted to pre process empty shader file {}.", sf.filename));
			}

			parseIncludes(sf);
			parseExtensions(sf);
		}
	}

	std::vector<std::string> parseIncludes(ShaderFile& sf) {
		const std::regex includePattern(R"(^\s*#include\s*["<]([^">]+)[">]\s*$)");
		std::smatch matches;

		std::vector<std::string> newCode;
		std::vector<std::string> injectedIncludes;
		for (int i = 0; i < sf.code.size(); i++) {
			std::string line = sf.code[i];
			if (std::regex_match(line, matches, includePattern)) {
				std::string iFilenameStr = matches[1].str();
				ShaderFile& isf = procShaders[iFilenameStr];
				const std::vector<std::string>& iCode = parseIncludes(isf);
				if (iCode[0].rfind("#version", 0) != 0) LOG(LogOpenGL, LOG_CRITICAL, "Shader file missiong #version declaration.");
				newCode.insert(newCode.end(), iCode.begin()+1, iCode.end());
				injectedIncludes.insert(injectedIncludes.end(), isf.includes.begin(), isf.includes.end());
			}
			else {
				newCode.push_back(line);
			}
		}

		sf.code = newCode;
		return newCode;
	}

	void parseExtensions(ShaderFile& sf) {
		std::vector<std::string> extensionDecs;
		std::vector<std::string> newCode;
		for (int i = 0; i < sf.code.size(); i++) {
			std::string line = sf.code[i];

			if (line.rfind("#extension", 0) == 0)
				extensionDecs.push_back(line);
			else
				newCode.push_back(line);
		}

		newCode.insert(newCode.begin() + 1, extensionDecs.begin(), extensionDecs.end());
		sf.code = newCode;
	};

	void writeFiles() {
		std::filesystem::path outDir = SHADERS_PROC_DIR;

		try {
			std::filesystem::create_directories(outDir);
		}
		catch (const std::filesystem::filesystem_error& e) {
			LOG(LogOpenGL, LOG_CRITICAL, std::format("Could not create pre processed shader files folder. Error: {}", e.what()));
		}

		for (const auto& it : procShaders) {
			const ShaderFile& sf = it.second;

			if (sf.filename.find(".glsl") != std::string::npos) continue;

			std::filesystem::path fileName = sf.filename;
			std::filesystem::path filePath = outDir / fileName;

			std::ofstream outFile(filePath);
			if (!outFile) {
				LOG(LogOpenGL, LOG_CRITICAL, std::format("Failed to write pre-processed shader file {}", sf.filename));
			}

			for (const std::string& line : sf.code) {
				outFile << line + "\n";
			}

			outFile.close();
		}
	}
};