#pragma once

#include <Core/Core.h>

struct Shader {
    Shader() = default;

    GLuint id = 0;
    std::string name;

    std::string vertPath;
    std::string fragPath;
    std::string geomPath;

    bool isCompiled = true;

	void Bind() const {
		glUseProgram(id);
	}

	void SetUBool(const std::string& name, const bool value) const {
		assert(isCompiled && "Shader: Can't set uniform for not compiled shader.");
		glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
	}

	void SetUInt(const std::string& name, const int value) const {
		assert(isCompiled && "Shader: Can't set uniform for not compiled shader.");
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}

	void SetUFloat(const std::string& name, const float value) const {
		assert(isCompiled && "Shader: Can't set uniform for not compiled shader.");
		glUniform1f(glGetUniformLocation(id, name.c_str()), value);
	}

	void SetUMat4(const std::string& name, const glm::mat4& value) const {
		assert(isCompiled && "Shader: Can't set uniform for not compiled shader.");
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &value[0][0]);
	}

	void SetUVec3(const std::string& name, const glm::vec3& value) const {
		assert(isCompiled && "Shader: Can't set uniform for not compiled shader.");
		glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}

	void SetUVec2(const std::string& name, const glm::vec2& value) const {
		assert(isCompiled && "Shader: Can't set uniform for not compiled shader.");
		glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}

	void SetUMat4v(const std::string& name, const GLsizei size, const glm::mat4* values) const {
		assert(isCompiled && "Shader: Can't set uniform for not compiled shader.");
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), size, GL_FALSE, glm::value_ptr(values[0]));
	}

	void Compile(const std::string& name, const std::string& vertPath, const std::string& fragPath, const std::string& geomPath = "") {
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

		this->name = name;
		this->id = programID;
		this->isCompiled = true;
	}

private:

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