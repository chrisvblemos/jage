#pragma once

#include <Engine/Core.h>
#include "EntityManager.h"
#include "IComponentArray.h"

template <typename T>
class ComponentArray : public IComponentArray {
public:
	void InsertData(Entity entity, T component) {
		assert(mEntityToIndexMap.find(entity) == mEntityToIndexMap.end() && "ComponentArray: Component already added");

		size_t newIndex = mSize;
		mEntityToIndexMap[entity] = newIndex;
		mIndexToEntityMap[newIndex] = entity;
		mComponentArray[newIndex] = component;
		++mSize;
	}

	void RemoveData(Entity entity) {
		assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() && "ComponentArray: Removing non-existent component");

		size_t indexRemoved = mEntityToIndexMap[entity];
		size_t indexLast = mSize - 1;
		
		mComponentArray[indexRemoved] = mComponentArray[indexLast];	// we point the erased element index to the last element

		Entity lastEntity = mIndexToEntityMap[indexLast];			// assign the lastEntity to the erasedEntity
		mEntityToIndexMap[lastEntity] = indexRemoved;				// update lastEntity indexes to point the erasedIndex
		mIndexToEntityMap[indexRemoved] = lastEntity;

		mEntityToIndexMap.erase(entity);							// finally, erase what is not necessary anymore
		mIndexToEntityMap.erase(indexLast);

		--mSize;													// update last index available through mSize
	}

	T& GetData(Entity entity) {
		assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() && "ComponentArray: Attempting to retrieve non-existent component.");
	
		return mComponentArray[mEntityToIndexMap[entity]];
	}

	void EntityDestroyed(Entity entity) override {
		if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end()) {
			RemoveData(entity);
		}
	}

private:
	std::array<T, Constants::MAX_ENTITIES> mComponentArray;
	std::unordered_map<Entity, size_t> mEntityToIndexMap;	// unordered map does have its performance impact, but we get find, insert and delete, avoiding ifs
	std::unordered_map<size_t, Entity> mIndexToEntityMap;
	size_t mSize;


};