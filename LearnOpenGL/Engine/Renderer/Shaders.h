#pragma once

#include "../Shader.h"

namespace Shaders {
	inline Shader LitDeferred("SHADER_LIGHT_PASS", "Assets/Shaders/blit.vert.glsl", "Assets/Shaders/lit_deferred.frag.glsl");
	inline Shader UnlitDeferred("SHADER_UNLIT", "Assets/Shaders/blit.vert.glsl", "Assets/Shaders/unlit_deferred.frag.glsl");
	inline Shader Blit("SHADER_SCREEN", "Assets/Shaders/blit.vert.glsl", "Assets/Shaders/blit.frag.glsl");
	inline Shader GBuffer("SHADER_GBUFFER", "Assets/Shaders/static_geometry.vert.glsl", "Assets/Shaders/gbuffer.frag.glsl");
}