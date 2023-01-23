#include "CEntityComponent.h"
#include "IComponent.h"
#include "CEvent.h"

CEntityComponent::CEntityComponent(CEntityComponent&& other) :
	ComponentList(std::move(other.ComponentList))
		, GeneratedComponentsCount(std::move(other.GeneratedComponentsCount))
{
}

CEntityComponent::CEntityComponent( CEntityComponent& other) :
	ComponentList(std::move(other.ComponentList))
		, GeneratedComponentsCount(std::move(other.GeneratedComponentsCount))
{
}

CEntityComponent::CEntityComponent(const CEntityComponent& other) :
	ComponentList(std::move(other.ComponentList))
	, GeneratedComponentsCount(std::move(other.GeneratedComponentsCount))
{
}
void CEntityComponent::RemoveComponent(IComponent* child)
{
}

void CEntityComponent::AddComponent(IComponent* newComponent)
{
	newComponent->SetEntity(this);
	newComponent->BuildComponent();
	newComponent->RegisterEvents();
	ComponentList.emplace_back(newComponent);
}
