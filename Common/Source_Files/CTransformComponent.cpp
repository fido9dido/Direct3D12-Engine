#include "CTransformComponent.h"

CTransformComponent::CTransformComponent() 
{
}

void CTransformComponent::SetTransform(const DirectX::XMMATRIX& newMatrix)
{
    WorldMatrix = newMatrix;
    OnTransformComponentUpdate.Notify(WorldMatrix);
}

void CTransformComponent::OnUpdate()
{
}

IComponent* CTransformComponent::BuildComponent()
{
    return nullptr;
}

void CTransformComponent::RegisterEvents()
{
}
