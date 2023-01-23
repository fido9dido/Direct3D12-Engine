#ifndef CAMERA_H
#define CAMERA_H

#include "FirstPersonMode.h"
#include <DirectXMath.h>
#include <array>
#include <memory>
#include <d3d12.h>

#define   nStages   4  // camera zoom levels x0 x4 x8 x16 

enum class CameraViews : int
{
	Perspective,
	OrthFront,
	OrthSide,
	OrthTop
};
enum class RENGINEMODE
{
	Mode_NotSelected,
	Mode_Perspective, // perspective projection
	Mode_2D, // world coordinates equals to screen coordinates
	Mode_Orthogonal // orthogonal projection
};
struct Point3D {
	float X, Y, Z;
} typedef Point;


class CCamera : public FirstPersonMode
{
public:
	CCamera();
	//TODO add controller and set view matrix using controller axis
	const RENGINEMODE GetMode() const { return Mode; };
	const DirectX::XMMATRIX& GetProjectionView()const { return ViewProjection; }
	void InitCamera(float backBufferWidth, float backBufferHeight, const DirectX::XMVECTOR& cameraPosition, const DirectX::XMVECTOR& cameraDirection);
	void InitOrthogonalCamera(float backBufferWidth, float backBufferHeight, float scale, int n);
	void UpdateOrthogonalCamera(int n);
	void UpdateCamera(int n);
	void Strafe(float dt);
	void Move(float dt);
	void Prepare2D();
	void SetClippingPlanes(float nearPlane, float farPlane);
	void SetMode(const RENGINEMODE mode, int stageNum);
	const std::array<D3D12_VIEWPORT, 4>& GetViewPort() const { return ViewPort; }
	const HRESULT SetProjectionPerspective(float fovY, float aspect, float nearZ, float farZ);
	const HRESULT UpdateProjectionPerspective(float fov, float aspectRatio, DirectX::XMMATRIX& outMatrix);
	const HRESULT SetView3D(const DirectX::XMVECTOR& cameraPosition, const DirectX::XMVECTOR& cameraDirection, const DirectX::XMVECTOR& cameraUp);
	const HRESULT SetViewLookAt(const DirectX::XMVECTOR& cameraPosition, const DirectX::XMVECTOR& point, const DirectX::XMVECTOR& worldUp);
	const HRESULT SetView3D();
	const HRESULT InitStage(float fFOV, const std::shared_ptr<D3D12_VIEWPORT>& view, int stageNum);
	const HRESULT GetFrustrumFromViewProjection(std::array<DirectX::XMVECTOR, 6>& frustumPlanes);
	const POINT Transform3Dto2DPosition(const DirectX::XMVECTOR& point);
	void Transform2Dto3D(const POINT& point, DirectX::XMVECTOR& outOrigin, DirectX::XMVECTOR& outDirection);
	void UpdateViewProjection();
	const std::array<DirectX::XMMATRIX, nStages >& GetProjectionPerspective();
	const std::array<DirectX::XMMATRIX, nStages >& GetProjectionOrthogonal();
	const DirectX::XMMATRIX& GetView3D();
private:

	float									OrthogonalDistance;
	float				                    Width;					// app width
	float				                    Height;
	float                                   NearWindowHeight;
	float									FarWindowHeight;
	float									Scale = 1;				// zooming value for orth camera
	float					                AspectRatio;			
	float									FovY;
	float									NearClippingPlane;		// near clipping plane Greater than 0 
	float									FarClippingPlane;		// far clipping plane
	int                                     Stage;					// stage (0-3) for viewport/projection zoom effect from normal to max zoom
	Point                                   OrthogonalPosition;
	RENGINEMODE                             Mode;					// persp., ortho or 2d (2d orthogonal but ignore distance)
	std::array<D3D12_VIEWPORT, nStages>	    ViewPort;				// viewports for all 4 stagesV

	DirectX::XMMATRIX						View2D;					// view matrix orthogonal
	DirectX::XMMATRIX 						Projection2D;			// orthogonal projection (whole screen)   
	DirectX::XMMATRIX 						View3D;					// perspective view matrix 
	DirectX::XMMATRIX 						ViewProjection;			// combo matrix for 3D
	std::array<DirectX::XMMATRIX, nStages>  ProjectionOrthogonal;	// orthogonal projection
	std::array<DirectX::XMMATRIX, nStages>	ProjectionPerspective;	// perspective projection

};

#endif

























