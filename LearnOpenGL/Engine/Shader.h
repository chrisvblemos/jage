#pragma once

#include "GameAsset.h"

#include <glad/glad.h>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Shader : public GameAsset {
public:
	uint32_t id;
	std::string vertexPath;
	std::string fragmentPath;
	bool isCompiled = false;

	Shader() = default;
	Shader(const std::string& assetName) : GameAsset(assetName) {};
	Shader(const std::string& assetName, const std::string& vertexPath, const std::string& fragmentPath) : GameAsset(assetName), vertexPath(vertexPath), fragmentPath(fragmentPath) {};

	std::string GetVertexCode() const;
	std::string GetFragmentCode() const;
};