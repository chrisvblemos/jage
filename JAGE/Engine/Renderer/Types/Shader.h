#pragma once

#include <Core/Core.h>
#include "IBindable.h"

class Shader {
public:
    Shader() = default;
	Shader(const std::string& vertCode, const std::string& fragCode, const std::string& geomCode = "")
		: vertCode(vertCode), fragCode(fragCode), geomCode(geomCode) {
		Compile();
	}

	bool IsCompiled() const { return isCompiled; }

	void Bind() const {
		glUseProgram(id);
	}

	void Unbind() const {
		glUseProgram(0);
	}

	void SetUBool(const std::string& name, const bool value) const {
		glUniform1i(glGetUniformLocation(id, name.c_str()), static_cast<uint32_t>(value));
	}

	void SetUInt(const std::string& name, const uint32_t value) const {
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}

	void SetUFloat(const std::string& name, const float value) const {
		glUniform1f(glGetUniformLocation(id, name.c_str()), value);
	}

	void SetUMat4(const std::string& name, const glm::mat4& value) const {
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &value[0][0]);
	}

	void SetUVec3(const std::string& name, const glm::vec3& value) const {
		glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}

	void SetUVec2(const std::string& name, const glm::vec2& value) const {
		glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}

	void SetUMat4v(const std::string& name, const GLsizei size, const glm::mat4* values) const {
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), size, GL_FALSE, glm::value_ptr(values[0]));
	}

private:
	GLuint id;
	GLuint unit;
	std::string vertCode;
	std::string fragCode;
	std::string geomCode;

	bool isCompiled = false;

	void Compile() {
		GLuint vertShader = CompileCode(vertCode, GL_VERTEX_SHADER);
		GLuint fragShader = CompileCode(fragCode, GL_FRAGMENT_SHADER);

		GLuint geomShader = 0;
		if (!geomCode.empty()) {
			geomShader = CompileCode(geomCode, GL_GEOMETRY_SHADER);
		}

		GLuint programID = LinkProgram(vertShader, fragShader, geomShader);

		this->id = programID;
		this->isCompiled = true;
	}

	GLuint LinkProgram(const GLuint vertexShader, const GLuint fragShader, const GLuint geomShader) {
		GLuint programID = glCreateProgram();

		glAttachShader(programID, vertexShader);
		glAttachShader(programID, fragShader);

		if (geomShader != 0)
			glAttachShader(programID, geomShader);

		glLinkProgram(programID);

		int success;
		glGetProgramiv(programID, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(programID, 512, nullptr, infoLog);
			LOG(LogOpenGL, LOG_CRITICAL, std::format("Shader program link failed. Code: {} \n Info: {}", vertCode + fragCode + geomCode, infoLog));
			//throw std::runtime_error("Shader: Program Linking Error: " + std::string(infoLog));
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragShader);

		if (geomShader != 0)
			glDeleteShader(geomShader);

		return programID;
	}

	std::string ReadShaderFile(const std::string& path) {
		std::ifstream file(path);
		if (!file.is_open()) LOG(LogOpenGL, LOG_CRITICAL, std::format("Failed to open shader file at {}", path));
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	GLuint CompileCode(const std::string& source, GLenum type) {
		GLuint shaderID = glCreateShader(type);
		const char* src = source.c_str();

		glShaderSource(shaderID, 1, &src, nullptr);
		glCompileShader(shaderID);

		int success;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(shaderID, 512, nullptr, infoLog);
			std::regex lineRegex("\\((\\d+)\\)");
			std::string infoLogStr = infoLog;
			std::smatch match;
			std::string line;
			std::string errorLine;
			if (std::regex_search(infoLogStr, match, lineRegex)) {
				size_t lineNum = std::stoi(match[1]);
				std::string lineStr;
				size_t currLine = 0;

				std::istringstream stream(source);
				while(std::getline(stream, line)) {
					++currLine;
					if (currLine == lineNum) {
						errorLine = line;
						break;
					}
				}

				LOG(LogOpenGL, LOG_CRITICAL, std::format("Failed to compile shader. {} at {}", infoLog, errorLine));
			}

			LOG(LogOpenGL, LOG_CRITICAL, std::format("Failed to compile shader. InfoLog: {}", type, infoLog));
		}

		return shaderID;
	}

	
};