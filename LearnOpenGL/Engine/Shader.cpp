#include "Shader.h"

std::string Shader::GetVertexCode() const {
	if (!std::filesystem::exists(vertexPath)) {
		std::cerr << assetName << "(Error): File for vertex code does not exist: " << vertexPath << std::endl;
		return "";
	}

	std::string vertexCode;
	std::ifstream vShaderFile;

	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		vShaderFile.open(vertexPath);

		std::stringstream vShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		vShaderFile.close();
		vertexCode = vShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "Shader(Error): Failed to read vertex code file for shader " << assetName << " with path " << vertexPath << std::endl;
		std::cout << e.what() << std::endl;
	}

	return vertexCode;
}

std::string Shader::GetFragmentCode() const {
	if (!std::filesystem::exists(fragmentPath)) {
		std::cerr << assetName << "(Error): File for fragment code does not exist: " << fragmentPath << std::endl;
		return "";
	}

	std::string fragmentCode;
	std::ifstream fShaderFile;

	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		fShaderFile.open(fragmentPath);
		std::stringstream fShaderStream;
		fShaderStream << fShaderFile.rdbuf();
		fShaderFile.close();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "Shader(Error): Failed to read fragment code file for shader " << assetName << " with path " << fragmentPath << std::endl;
	}

	return fragmentCode;
}