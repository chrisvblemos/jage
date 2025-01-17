#pragma once

#include <Core/Core.h>

class ComponentWidget {
public:
	virtual ~ComponentWidget() = default;
	virtual void Render(const Entity entity) = 0;
};