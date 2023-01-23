#include "CMeshComponent.h"
#include "CEntityComponent.h"
#include "MeshGeometry.h"
#include <CTransformComponent.h>

const MeshData* CMeshComponent::GetConstBufferData()
{
	return (MeshRef) ? MeshRef->GeometryPrimitive->MeshDataRef : nullptr;
}

MeshData* CMeshComponent::GetBufferData()
{
	return (MeshRef) ? MeshRef->GeometryPrimitive->MeshDataRef : nullptr;
}

void CMeshComponent::SetDescription(const SMeshDescriptor& desc)
{
	Description = desc;
}
void CMeshComponent::UpdateTransform(const DirectX::XMMATRIX& worldMatrix)
{
//TODO Remove decouple worldmatrix from render item
	if (!MeshRef) { return; }
	
	MeshRef->WorldMatrix = worldMatrix;
	MeshRef->FramesDirtyCount += 3;
}
void CMeshComponent::SetLayer(RenderLayer layer)
{
}

IComponent* CMeshComponent::BuildComponent()
{
	if (Description.Name.empty()) { return nullptr; }

	std::shared_ptr<CRenderItem> renderitem = std::make_shared<CRenderItem>();
	MeshRef = renderitem.get();
	renderitem->TextureTransform = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
	renderitem->Layer = static_cast<RenderLayer>(Description.Layer);
	renderitem->SubstanceCBIndex = Description.SubstanceCBIndex;
	OnRenderComponentCreated.Notify(renderitem, Description.Name);
	return this;
}

void CMeshComponent::OnUpdate()
{
}

void CMeshComponent::RegisterEvents()
{
	CTransformComponent* transformComponent = GetEntity()->GetComponentOfType<CTransformComponent>();
	if (!transformComponent) { return; }

	transformComponent->OnTransformComponentUpdate.Register(std::bind(&CMeshComponent::UpdateTransform, this, std::placeholders::_1));
}
