#ifndef CMESHCOMPONENT_H
#define CMESHCOMPONENT_H

#include "IComponent.h"
#include "CRenderItem.h"
#include "CEvent.h"
#include <memory>
#include <string>

struct SMeshDescriptor
{
	std::string Name;
	uint8_t Layer;
	int SubstanceCBIndex;
};


class  CMeshComponent : public IComponent
{
public:
	using OnRenderComponentCreatedEvent = CEvent<std::shared_ptr<CRenderItem>&, const std::string&>;
	using OnRenderComponentDeleteEvent = CEvent<int>;
	OnRenderComponentCreatedEvent OnRenderComponentCreated;
	OnRenderComponentDeleteEvent OnRenderComponentDelete;
private:
	SMeshDescriptor Description;
	CRenderItem* MeshRef;
	
public:

	CMeshComponent() =delete;
	template<typename Func, typename... Args>
	CMeshComponent(Func&& function, Args&& ...args) : MeshRef(nullptr)
	{
		OnRenderComponentCreated.Register(
			std::bind(std::move(function), std::forward<Args>(args)...)
		);
	}
	virtual ~CMeshComponent(){}

	void SetDescription(const SMeshDescriptor& desc);
	void UpdateTransform(const DirectX::XMMATRIX& worldMatrix);
	void SetLayer(RenderLayer layer);
	
	const struct MeshData* GetConstBufferData();
	struct MeshData* GetBufferData();

	virtual IComponent* BuildComponent() override;
	virtual void RegisterEvents() override;
private:

	virtual void OnUpdate() override;

};

#endif