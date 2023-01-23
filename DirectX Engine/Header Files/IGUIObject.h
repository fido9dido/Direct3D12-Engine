#ifndef IGUIOBJECT_H
#define IGUIOBJECT_H

#include <Windows.h>
#include <DirectXMath.h>

class IGUIObject
{
public:
	IGUIObject() = default;

	virtual bool Picked(const DirectX::XMVECTOR&, float*) = 0;
	void UnHide(void) { m_bHidden = false; }
	bool IsHidden(void) { return m_bHidden; }
	void Hide(void) {
		m_bHidden = true;
		m_bSelected = false;
	}

protected:
private:
	UINT m_NumVerts; // number of vertices
	UINT m_NumIndis; // number of indices
	UINT m_NumPolys; // number of polygons
	bool m_bSelected; // currently selected?
	bool m_bHidden; // currently hidden
	DWORD m_Type;
};

#endif // !IUIOBJECT_H
