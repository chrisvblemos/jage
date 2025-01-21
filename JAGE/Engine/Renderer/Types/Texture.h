#pragma once

#include <Core/Core.h>
#include "IBindable.h"

template <typename T>
struct TexParameter {
	GLenum name;
	T value;
};

class TextureBase {
public:
	TextureBase() = default;
	TextureBase(
		const std::string name,
		const GLenum internalFormat,
		const GLuint width,
		const GLuint height)
		: name(name),
		type(type),
		internalFormat(internalFormat),
		width(width),
		height(height) {
	};

	void BindToUnit(const GLuint unit) {
		glBindTextureUnit(unit, id);
		isBound = true;
		this->unit = unit;
	}

	void UnbindFromUnit(const GLuint unit) {
		glBindTextureUnit(unit, 0);
		isBound = false;
		this->unit = -1;
	}

	virtual void Bind() = 0;
	virtual void Unbind() = 0;

	template <typename T>
	void SetParam(const GLenum pname, const T pvalue) const {};

	template <>
	void SetParam<GLfloat>(GLenum pname, const GLfloat pvalue) const {
		glTextureParameterf(id, pname, pvalue);
	}

	template <>
	void SetParam<GLint>(GLenum pname, const GLint pvalue) const {
		glTextureParameteri(id, pname, pvalue);
	}

	template <typename T>
	void SetParams(GLenum pname, const T* pvalues) const {};

	template <>
	void SetParams<GLfloat>(GLenum pname, const GLfloat* pvalues) const {
		glTextureParameterfv(id, pname, pvalues);
	}

	template <>
	void SetParams<GLint>(GLenum pname, const GLint* pvalues) const {
		glTextureParameteriv(id, pname, pvalues);
	}

	void MakeResident() {
		if(handle == 0) {
			handle = glGetTextureHandleARB(id);
		}

		if (!isResident && handle != 0)
			glMakeTextureHandleResidentARB(handle);
	}

	void MakeNonResident() const {
		if (isResident && handle != 0)
			glMakeTextureHandleNonResidentARB(handle);
	}

	void Clear() {
		if (id != 0) {
			glDeleteTextures(1, &id);
			id = -1;
		}
	}

	void GenerateMipMap() const {
		if (handle != 0) {
			LOG(LogOpenGL, LOG_WARNING, std::format("Attempting to set param in immutable texture {}.", name));
			return;
		}

		glGenerateTextureMipmap(id);
	}

	std::string GetName() const { return name; }
	GLuint GetID() const { return id; }
	GLuint64 GetHandle() const { return handle; }
	uint32_t GetHandleIndex() const { return handleIdx; }
	void SetHandleIndex(const int32_t index) { handleIdx = index; }

protected:
	std::string name;
	GLuint id = 0;
	GLenum type = 0;
	GLenum format = 0;
	GLenum internalFormat = 0;
	GLuint width = 0;
	GLuint height = 0;
	bool isBound = false;
	bool isResident = false;
	GLuint64 handle = 0;
	int32_t unit = -1;
	int32_t handleIdx = -1;
};

class TextureCubeMapArray : public TextureBase {
public:
	TextureCubeMapArray() = default;
	TextureCubeMapArray(
		const std::string name,
		const GLenum internalFormat,
		const GLuint width,
		const GLuint height,
		const GLuint depth,
		const GLint levels)
		: TextureBase(
			name,
			internalFormat,
			width,
			height), depth(depth), levels(levels) {

		if (depth % 6 != 0) {
			LOG(LogOpenGL, LOG_CRITICAL, "Failed to generate texture cubemap array. Depth isn't a multiple of 6.");
			return;
		}

		glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &id);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		Allocate(internalFormat, width, height, depth);
	};

	void Bind() override {
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, id);
		isBound = true;
	}

	void Unbind() override {
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
		isBound = false;
	}

	void Allocate(const GLenum internalFormat, const GLuint width, const GLuint height, const GLuint depth) {
		glTextureStorage3D(id, 1, internalFormat, width, height, depth);

		this->internalFormat = internalFormat;
		this->width = width;
		this->height = height;
		this->depth = depth;
	}

	void SetSubImage(const GLint level, const GLsizei width, const GLsizei height, 
		const GLsizei selDepth, const GLenum format, const GLenum type, const void* data) {
		if (handle != 0) {
			LOG(LogOpenGL, LOG_WARNING, std::format("Attempting to set param in immutable texture {}.", name));
			return;
		}

		this->type = type;
		glTextureSubImage3D(id, level, 0, 0, 0, width, height, selDepth, format, type, data);
	}

private:
	GLuint depth = 0;
	GLint levels = 0;
};

class Texture2D : public TextureBase {
public:
	Texture2D() = default;

	Texture2D(
		const std::string name,
		const GLenum internalFormat,
		const GLuint width,
		const GLuint height)
		: TextureBase(
			name,
			internalFormat,
			width,
			height) {

		glCreateTextures(GL_TEXTURE_2D, 1, &id);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTextureStorage2D(id, 1, internalFormat, width, height);
	}

	void Bind() override {
		glBindTexture(GL_TEXTURE_2D, id);
	}

	void Unbind() override {
		glBindTexture(GL_TEXTURE_2D, id);
	}

	void SetSubImage2D(const GLenum format, const GLint level, const GLint xoffset, 
		const GLint yoffset, const GLenum type, const void* data) {
		if (handle != 0)
			LOG(LogOpenGL, LOG_WARNING, std::format("Attempting to set param in immutable texture {}.", name));

		if (data == nullptr)
			LOG(LogOpenGL, LOG_CRITICAL, std::format("Failed to set sub image 2D for texture {}. Data is null.", name));

		this->type = type;
		this->format = format;
	
		glTextureSubImage2D(id, level, xoffset, yoffset, width, height, format, type, data);
	}

};