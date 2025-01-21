#pragma once

#include <Core/Core.h>
#include "IBindable.h"

class Shader {
public:
    Shader() = default;
	Shader(const std::string& vertPath, const std::string& fragPath, const std::string& geomPath = "")
		:	vertPath(vertPath), fragPath(fragPath), geomPath(geomPath) {
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
		glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
	}

	void SetUInt(const std::string& name, const int value) const {
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
	std::string vertPath;
	std::string fragPath;
	std::string geomPath;

	bool isCompiled = false;

	void Compile() {
		std::string vertCode = ReadShaderFile(vertPath);
		std::string fragCode = ReadShaderFile(fragPath);

		GLuint vertShader = CompileCode(vertCode, GL_VERTEX_SHADER);
		GLuint fragShader = CompileCode(fragCode, GL_FRAGMENT_SHADER);

		GLuint geomShader = 0;
		if (!geomPath.empty()) {
			std::string geomCode = ReadShaderFile(geomPath);
			geomShader = CompileCode(geomCode, GL_GEOMETRY_SHADER);
		}

		GLuint programID = LinkProgram(vertShader, fragShader, geomShader);
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);

		if (geomShader > 0)
			glDeleteShader(geomShader);

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
			throw std::runtime_error("Shader: Program Linking Error: " + std::string(infoLog));
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragShader);

		if (geomShader != 0)
			glDeleteShader(geomShader);

		return programID;
	}

	std::string ReadShaderFile(const std::string& path) {
		std::ifstream file(path);
		if (!file.is_open()) throw std::runtime_error("Shader: Failed to open code file: " + path);
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
			throw std::runtime_error("Shader Compilation Error: " + std::string(infoLog));
		}

		return shaderID;
	}

	
};