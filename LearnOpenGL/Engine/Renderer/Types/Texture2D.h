#pragma once

#include <Engine/Core.h>

struct Texture2D {
	std::string name;
	GLuint id = 0;
	GLuint64 handle = 0;
	const void* data = nullptr;
	GLint level = 0;
	GLsizei width = 0;
	GLsizei height = 0;
	GLenum format = 0;
	GLint border = 0;
	GLenum type = 0;
	std::unordered_map<std::string, uint32_t> intVals;
	std::unordered_map<std::string, float> floatVals;

	Texture2D() = default;

	void Generate(
		const std::string& name,
		const void* data,
		GLint level,
		GLsizei width,
		GLsizei height,
		GLenum format,
		GLint border,
		GLenum type) {

		this->level = level;
		this->width = width;
		this->height = height;
		this->format = format;
		this->border = border;
		this->type = type;

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, level, format, width, height, border, format, type, data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void GenerateMipMap() const {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	void Bind() const {
		glBindTexture(GL_TEXTURE_2D, id);
	}

	void Unbind() const {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void SetInt(const GLenum param, const GLint v) const {
		glTexParameteri(GL_TEXTURE_2D, param, v);
	}

	void SetFloat(const GLenum param, const float v) const {
		glTexParameterf(GL_TEXTURE_2D, param, v);
	}

	void SetFloatVec(const GLenum param, const float* v) const {
		glTexParameterfv(GL_TEXTURE_2D, param, v);
	}

	void ActiveAndBind(const GLuint index) const {
		glActiveTexture(GL_TEXTURE0 + index);
		Bind();
	}

};