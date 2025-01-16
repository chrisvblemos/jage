#pragma once

#include <Core/Core.h>

class System {
public:
	std::string name;
	std::set<Entity> mEntities{};
};