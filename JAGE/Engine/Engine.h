#pragma once

#include <Core/Core.h>
#include <Core/Window.h>
#include <Core/Input.h>

#define LogEngine "Engine"

class Engine {
public:
	Engine() = default;

	void Init();

private:
	float frameTime;
	float frameTimePrev;
};