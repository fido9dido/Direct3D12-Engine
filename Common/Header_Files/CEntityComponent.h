#ifndef CENTITYCOMPONENT_H
#define CENTITYCOMPONENT_H

#include <vector>
#include <string>

#include "CPhysicsComponent.h"
#include "CMeshComponent.h"

struct SEntityDescriptor
{
	SMeshDescriptor meshDesc;
	SPhysicsDescriptor physicsDesc;
};

class CEntityComponent
{
	std::vector<class IComponent*> ComponentList; 
	int GeneratedComponentsCount = 0;

public:
	CEntityComponent() = default;
	CEntityComponent(CEntityComponent&& other);
	CEntityComponent(CEntityComponent& other);
	CEntityComponent(const CEntityComponent& other);

	void RemoveComponent(class IComponent* child);
	void AddComponent(class IComponent* child);
	int GenerateComponentID() { return GeneratedComponentsCount++; }
	
	template<typename T>
	T* GetComponentOfType()
	{
		T* result = nullptr;
		for (IComponent* component : ComponentList)
		{
			result = dynamic_cast<T*>(component);
			if (result)
			{ 
				break; 
			}
		}
		return result;
	}
};

#endif