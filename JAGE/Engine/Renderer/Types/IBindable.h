#pragma once

#include <Core/Core.h>

#define BIND(T, bffer) \
	ScopedBind<T> CONCAT(bind, __COUNTER__)(bffer)

#define BIND_TEX(T, tex, unit) \
	ScopedBindTex<T> CONCAT(bindTex, __COUNTER__)(tex, unit)

#define CONCAT_INNER(x, y) x##y
#define CONCAT(x, y) CONCAT_INNER(x, y)

template <typename T>
class ScopedBind {
public:
	T& bindable;

	ScopedBind(T& bindable) : bindable(bindable) {
		bindable.Bind();
	};

	~ScopedBind() {
		bindable.Unbind();
	}
};

template <typename T>
class ScopedBindTex {
public:
	T& tex;
	GLuint unit = 0;

	ScopedBindTex(T& tex, const GLuint unit) : tex(tex), unit(unit) {
		tex.BindToUnit(unit);
	}

	~ScopedBindTex() {
		tex.UnbindFromUnit(unit);
	}
};