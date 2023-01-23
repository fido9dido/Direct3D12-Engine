#ifndef ICOMPONENT_H
#define ICOMPONENT_H

#include <vector>

class IComponent
{
private:
	int ID;
	class CEntityComponent* ParentEntity;
public:

	IComponent() = default;
	virtual ~IComponent();

	virtual IComponent* BuildComponent() = 0; 
	// used to register other components event callback
	virtual void RegisterEvents() = 0;
	void SetEntity(class CEntityComponent* parent);


	class CEntityComponent* GetEntity() { return ParentEntity; }

	//parent ref
	int GetID() { return ID; }
private:
	virtual void OnUpdate() = 0;


};

#endif