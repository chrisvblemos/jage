#pragma once

#include <Core/Core.h>

struct FrameBuffer {
	FrameBuffer() = default;

	struct ColorAttachment {
		std::string name;
		GLuint id = 0;
	};

	struct DepthAttachment {
		std::string name;
		GLuint id = 0;
	};

	std::string name;
	GLuint id;
	GLuint width = 0;
	GLuint height = 0;
	GLuint rbo = 0;
	std::vector<ColorAttachment> colorAttachments;
	DepthAttachment depthAttachment;

	void Generate(const std::string name, uint32_t width, uint32_t height) {
		glGenFramebuffers(1, &id);

		this->name = name;
		this->width = width;
		this->height = height;
	}

	void Clear() {
		colorAttachments.clear();
		glDeleteFramebuffers(1, &id);
	}

	void CreateDepthAttachment(GLenum format) {
		glGenTextures(1, &depthAttachment.id);
		glBindTexture(GL_TEXTURE_2D, depthAttachment.id);
		glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_COMPONENT, GL_TEXTURE_2D, depthAttachment.id, 0);
	}

	void Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, id);
	}

	void Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void CreateRenderBuffer() {
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void AttachColorTexture2D(const std::string& name, const GLuint texture, const GLuint i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texture, 0);
		ColorAttachment colorAttachment;
		colorAttachment.id = texture;
		colorAttachment.name = name;

		colorAttachments.push_back(colorAttachment);
	}

	void AttachDepthTexture2D(const std::string& name, const GLuint texture) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
		DepthAttachment depthAttachment;
		depthAttachment.id = texture;
		depthAttachment.name = name;

		this->depthAttachment = depthAttachment;
	}

	void AttachCubeMapTexture(const std::string& name, const GLuint cubemapTexture) {
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubemapTexture, 0);
		DepthAttachment depthAttachment;
		depthAttachment.id = cubemapTexture;
		depthAttachment.name = name;

		this->depthAttachment = depthAttachment;
	}

	void DettachDepthTexture() {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	}

	void DrawBuffer(const std::string& name) {
		for (int i = 0; i < colorAttachments.size(); i++) {
			if (name == colorAttachments[i].name) {
				glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
				return;
			}
		}
	}

	void DrawColorBuffers() {
		size_t nColorAttachments = colorAttachments.size();
		std::vector<GLuint> attachments;
		for (int i = 0; i < colorAttachments.size(); i++) {
			attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
		}
		glDrawBuffers(static_cast<GLsizei>(colorAttachments.size()), attachments.data());
	}

	void DisableColorBuffer() {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	void SetViewport() {
		glViewport(0, 0, width, height);
	}

	bool CheckComplete() {
		return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}
};


