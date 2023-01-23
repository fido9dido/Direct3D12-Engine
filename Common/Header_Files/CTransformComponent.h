#ifndef CTRANSFORMCOMPONENT_H
#define CTRANSFORMCOMPONENT_H

#include "IComponent.h"
#include <DirectXMath.h>
#include <CEvent.h>

class CTransformComponent : public IComponent
{
	DirectX::XMMATRIX WorldMatrix;
	using OnTransformComponentUpdateEvent = CEvent<const DirectX::XMMATRIX&>;

public:
	OnTransformComponentUpdateEvent OnTransformComponentUpdate;
	CTransformComponent();
	virtual ~CTransformComponent() {}
	void SetTransform(const DirectX::XMMATRIX& newMatrix);
	const DirectX::XMMATRIX& GetWorldMatrix() { return WorldMatrix; }
	const DirectX::XMVECTOR& GetTranslation() { return WorldMatrix.r[3]; }
	virtual void OnUpdate() override;
	virtual IComponent* BuildComponent() override;
	virtual void RegisterEvents() override;

};

#endif