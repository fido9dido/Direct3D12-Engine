#include "IComponent.h"
#include "CEntityComponent.h"

IComponent::~IComponent()
{
	ParentEntity->RemoveComponent(this);
}

void IComponent::SetEntity(CEntityComponent* parent)
{
	ParentEntity = parent;
	ID = parent->GenerateComponentID();
}