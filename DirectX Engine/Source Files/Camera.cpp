#include "Camera.h"
#include "CustomReturnValues.h"

CCamera::CCamera() : Mode(RENGINEMODE::Mode_Perspective), Stage(-1), View3D(DirectX::XMMatrixIdentity())
{
	ProjectionPerspective[0] = DirectX::XMMatrixIdentity();
}

/**
 * Set camera as viewmatrix.
 * -> IN: float* - 4 float building camera's up vector
 *        float* - 4 float building camera's direction vector
 *        float* - 4 float building camera's pos vector aka eye
 */
const HRESULT CCamera::SetView3D(const DirectX::XMVECTOR& CamPos,
	const DirectX::XMVECTOR& CamDir,
	const DirectX::XMVECTOR& CamUp
)
{
	//XMMatrixLookAtLH
	View3D = DirectX::XMMatrixLookToLH(CamPos, CamDir, CamUp);

	UpdateViewProjection();

	return DXR_OK;
}

void CCamera::InitCamera(float  backBufferWidth, float backBufferHeight, const DirectX::XMVECTOR& cameraPosition, const DirectX::XMVECTOR& cameraDirection)
{
	Width = backBufferWidth;
	Height = backBufferHeight;
	Position = cameraPosition;
	Mode = RENGINEMODE::Mode_Perspective;
	//set far near
	SetClippingPlanes(0.1f, 1000.f);

	D3D12_VIEWPORT viewPort = { 0, 0, Width, Height };

	InitStage(0.8f, std::make_shared<D3D12_VIEWPORT>(viewPort), 0);

	SetProjectionPerspective(DirectX::XM_PI / 4.f, float(backBufferWidth) / float(backBufferHeight), 0.1f, 1000);

	View3D = DirectX::XMMatrixLookToLH(cameraPosition, cameraDirection, DirectX::XMVectorSet(0, 1, 0, 0));

	//init combo matrices 
	UpdateViewProjection();
}

//TODO  revise
void CCamera::InitOrthogonalCamera(
	float backBufferWidth
	, float backBufferHeight
	, float scale
	, int n //window number (refers to plane) 
)
{
	Scale = scale;
	OrthogonalDistance = 10;
	OrthogonalPosition = { 0, -2 ,0 };
	DirectX::XMVECTOR cameraPosition;
	Width = backBufferWidth;
	Height = backBufferHeight;

	SetMode(RENGINEMODE::Mode_Orthogonal, 0);;
	SetClippingPlanes(0.1f, 2495.f);

	D3D12_VIEWPORT viewPortView = { 0, 0, Width, Height };

	InitStage(0.8f, std::make_shared<D3D12_VIEWPORT>(viewPortView), 0);

	float x = 2.f / ((Width / Height) * Scale);
	float y = 2.f / Scale;
	float z = 1.0f / (FarClippingPlane - NearClippingPlane);

	ProjectionOrthogonal[Stage] += DirectX::XMMatrixScaling(x, y, z);

	if (n == 1) // XY plane: front
	{
		UpVector = DirectX::XMVectorSet(0, 1, 0, 0);
		DirectionVector = DirectX::XMVectorSet(0, 0, 1, 0);
		cameraPosition = DirectX::XMVectorSet(OrthogonalPosition.X, OrthogonalPosition.Y, -OrthogonalDistance, 0); //vcR, U, D, P
		View3D = DirectX::XMMatrixLookToLH(cameraPosition, DirectionVector, UpVector);
	}
	else if (n == 2)// ZY plane: side
	{
		UpVector = DirectX::XMVectorSet(0, 1, 0, 0);
		DirectionVector = DirectX::XMVectorSet(-1, 0, 0, 0);
		cameraPosition = DirectX::XMVectorSet(OrthogonalDistance, OrthogonalPosition.Y, OrthogonalPosition.Z, 0); //vcR, U, D, P
		View3D = DirectX::XMMatrixLookToLH(cameraPosition, DirectionVector, UpVector);
	}
	else if (n == 3) // XZ plane: top
	{
		UpVector = DirectX::XMVectorSet(0, 0, 1, 0);
		DirectionVector = DirectX::XMVectorSet(0, -1, 0, 0);
		cameraPosition = DirectX::XMVectorSet(OrthogonalPosition.X, OrthogonalDistance, OrthogonalPosition.Z, 0); //vcR, U, D, P
		View3D = DirectX::XMMatrixLookToLH(cameraPosition, DirectionVector, UpVector);
	}
	else {
		throw 0;
	}
	//init combo matrices 
	UpdateViewProjection();
	bViewDirty = true;
}

//TODO  revise
void CCamera::UpdateOrthogonalCamera(int n)
{
	DirectX::XMVECTOR cameraPosition;

	SetMode(RENGINEMODE::Mode_Orthogonal, 0);;
	//SetClippingPlanes(0.1f, 2495);
	D3D12_VIEWPORT viewPortView = { 0, 0, Width, Height };
	InitStage(0.8f, std::make_shared<D3D12_VIEWPORT>(viewPortView), 0);
	float x = 2 / ((Width / Height) * Scale);
	float y = 2 / Scale;
	float z = 1.0f / (FarClippingPlane - NearClippingPlane);
	ProjectionOrthogonal[Stage] += DirectX::XMMatrixScaling(x, y, z);

	if (n == 1)
	{ // XY plane: ront
		cameraPosition = DirectX::XMVectorSet(OrthogonalPosition.X, OrthogonalPosition.Y, -OrthogonalDistance, 0); //vcR, U, D, P
		View3D = DirectX::XMMatrixLookToLH(cameraPosition, DirectionVector, UpVector);
	}
	else if (n == 2)// ZY plane: side
	{
		cameraPosition = DirectX::XMVectorSet(OrthogonalDistance, OrthogonalPosition.Y, OrthogonalPosition.Z, 0); //vcR, U, D, P
		View3D = DirectX::XMMatrixLookToLH(cameraPosition, DirectionVector, UpVector);
	}
	else if (n == 3) // XZ plane: top
	{
		cameraPosition = DirectX::XMVectorSet(OrthogonalPosition.X, OrthogonalDistance, OrthogonalPosition.Z, 0); //vcR, U, D, P
		View3D = DirectX::XMMatrixLookToLH(cameraPosition, DirectionVector, UpVector);
	}
	else
	{
		throw 0;
	}
}

void CCamera::UpdateCamera(int n)
{
	if (n == 1)
	{
		UpdateOrthogonalCamera(n);
	}
	else if (n == 2)
	{
		UpdateOrthogonalCamera(n);
	}
	else if (n == 3)
	{
		UpdateOrthogonalCamera(n);
	}
	else if (n == 0)
	{
		SetView3D();
	}

	//update combo matrices 
	UpdateViewProjection();
}
void CCamera::Strafe(float dt)
{
	DirectX::XMVECTOR s = DirectX::XMVectorReplicate(dt);
	Position = DirectX::XMVectorMultiplyAdd(s, RightVector, Position);
	bViewDirty = true;
}
void CCamera::Move(float dt)
{
	DirectX::XMVECTOR s = DirectX::XMVectorReplicate(dt);
	Position = DirectX::XMVectorMultiplyAdd(s, DirectionVector, Position);
	bViewDirty = true;
}
const HRESULT CCamera::SetView3D()
{
	View3D = DirectX::XMMatrixLookToLH(Position, DirectionVector, UpVector);
	return DXR_OK;
}

// width and height of the main window
void CCamera::Prepare2D()
{
	View2D = DirectX::XMMatrixIdentity();
	Projection2D = DirectX::XMMatrixOrthographicLH(Width, Height, NearClippingPlane, FarClippingPlane);

	// 2D view matrix
	float tx, ty, tz;
	tx = -(Width)+(Width * 0.5f);
	ty = Height - (Height * 0.5f);
	tz = NearClippingPlane + 0.1f;

	View2D.r[1].m128_f32[1] = -1.0f;
	View2D *= DirectX::XMMatrixTranslation(tx, ty, tz);

}

const HRESULT CCamera::SetProjectionPerspective(float fovY, float aspect, float nearZ, float farZ)
{
	// cache properties
	FovY = fovY;
	AspectRatio = aspect;
	NearClippingPlane = nearZ;
	FarClippingPlane = farZ;

	NearWindowHeight = 2.0f * NearClippingPlane * tanf(0.5f * FovY);
	FarWindowHeight = 2.0f * FarClippingPlane * tanf(0.5f * FovY);
	ProjectionPerspective[Stage] = DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, NearClippingPlane, FarClippingPlane);

	return DXR_OK;
}
const HRESULT CCamera::UpdateProjectionPerspective(float fov, float AspectRatio, DirectX::XMMATRIX& outMatrix)
{
	outMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, AspectRatio, NearClippingPlane, FarClippingPlane);
	return DXR_OK;
}

/**
 * Calculates look at matrix or given point and calls SetView3D
 * to active the look at matrix.
 * -> IN: float* - 3 floats building camera's position vector
 *        float* - 3 floats building lookat point vector aka target
 *        float* - 3 floats building world up vector at cam pos normally 0 1 0
 */
const HRESULT CCamera::SetViewLookAt(const DirectX::XMVECTOR& cameraPos, const DirectX::XMVECTOR& targetPoint, const DirectX::XMVECTOR& worldUp)
{
	// n o need to normalize dir, it will be normalized in xmmatrix 
	DirectX::XMVECTOR  direction = DirectX::XMVectorSubtract(targetPoint, cameraPos);
	DirectX::XMVECTOR  directionNormal = DirectX::XMVector3Normalize(direction);
	DirectX::XMVECTOR  camera_right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(worldUp, directionNormal));// The "right" vector.
	DirectX::XMVECTOR  camera_Up = DirectX::XMVector3Cross(directionNormal, camera_right);

	Position = cameraPos;
	DirectionVector = directionNormal;
	RightVector = camera_right;
	UpVector = camera_Up;

	UpdateViewProjection();

	return SetView3D(cameraPos, direction, camera_Up);
}
// set engines near and ar clipping plane
void CCamera::SetClippingPlanes(float nearPlane, float farPlane)
{
	NearClippingPlane = nearPlane;
	FarClippingPlane = farPlane;
	if (NearClippingPlane <= 0.0f)
	{
		NearClippingPlane = 0.01f;
	}
	// make sure ar is not too close to near 
	if (FarClippingPlane <= 1.0f)
	{
		FarClippingPlane = 1.00f;
	}
	if (NearClippingPlane >= FarClippingPlane) {
		NearClippingPlane = FarClippingPlane;
		FarClippingPlane = NearClippingPlane + 1.0f;
	}
	// change 2D projection and view
	Prepare2D();

	////adjust orthogonal projection
	//float Q = 1.0f / (FarClippingPlane - NearClippingPlane);
	//float X = -Q * NearClippingPlane;
	//ProjectionOrthogonal[0].r[2].m128_f32[2]= ProjectionOrthogonal[1].r[2].m128_f32[2] = Q;
	//ProjectionOrthogonal[2].r[2].m128_f32[2] = ProjectionOrthogonal[3].r[2].m128_f32[2] = Q;
	//ProjectionOrthogonal[0].r[3].m128_f32[2] = ProjectionOrthogonal[1].r[3].m128_f32[2] = X;
	//ProjectionOrthogonal[2].r[3].m128_f32[2] = ProjectionOrthogonal[3].r[3].m128_f32[2] = X;

	////adjust perspective projection
	//Q *= FarClippingPlane;
	//X = -Q * NearClippingPlane;
	//ProjectionPerspective[0].r[2].m128_f32[2] = ProjectionPerspective[1].r[2].m128_f32[2] = Q;
	//ProjectionPerspective[2].r[2].m128_f32[2] = ProjectionPerspective[3].r[2].m128_f32[2] = Q;
	//ProjectionPerspective[0].r[3].m128_f32[2] = ProjectionPerspective[1].r[3].m128_f32[2] = X;
	//ProjectionPerspective[2].r[3].m128_f32[2] = ProjectionPerspective[3].r[3].m128_f32[2] = X;

}

// set mode or stage n, 0:=3D(perspective), 1:=2D(orthogonal)
void CCamera::SetMode(RENGINEMODE mode, int stageNum)
{
	Mode = mode;
	Stage = stageNum;
}
/*
 * Calculate perspective and orthogonal projection matrix or given
 * stage using given values. This will not activate any settings.
 * -> IN: float       - field of view (horizontal) radian
 *        ZFXVIEWPORT - viewport dimensions or NULL or screen dimensions
 *        int         - which stage to set (4 possible)
 *
 */
const HRESULT CCamera::InitStage(float fov, const std::shared_ptr<D3D12_VIEWPORT>& view, int stageNum)
{
	Stage = stageNum;

	if (!view)
	{
		D3D12_VIEWPORT viewPort = { 0, 0, Width, Height };
		memcpy(&ViewPort[stageNum], &viewPort, sizeof(RECT));
	}
	else
	{
		memcpy(&ViewPort[stageNum], view.get(), sizeof(RECT));
	}

	if ((stageNum > 3) || (stageNum < 0)) { stageNum = 0; }

	float Aspect = ((float)(ViewPort[stageNum].Width) / (ViewPort[stageNum].Height));

	// PERSPECTIVE PROJEKTION MATRIX
	if (Mode == RENGINEMODE::Mode_Perspective)
	{
		if (FAILED(UpdateProjectionPerspective(fov, Aspect, ProjectionPerspective[stageNum]))) { return DXR_FAIL; }
	}
	else
	{// ORTHOGONAL PROJECTION MATRIX
		ProjectionOrthogonal[stageNum] = DirectX::XMMatrixOrthographicLH(Width, Height, NearClippingPlane, FarClippingPlane);
	}
	return DXR_OK;;

}
/**
 * Cast given point from world space to screen space coordinates.
 * -> IN:     ZFXVector - vector to position in 3d space
 * -> RETURN: POINT     - corresponding point in 2d screen space
 */
const POINT CCamera::Transform3Dto2DPosition(const DirectX::XMVECTOR& Point)
{
	POINT point;
	DWORD width, height;

	// if 2D mode use whole screen
	if (Mode == RENGINEMODE::Mode_2D)
	{
		width = (DWORD)Width;
		height = (DWORD)Height;
	}
	// else take viewport dimensions
	else
	{
		width = (DWORD)ViewPort[Stage].Width;
		height = (DWORD)ViewPort[Stage].Height;
	}

	float clip_x = (float)(width >> 1);
	float clip_y = (float)(height >> 1);

	float x = (ViewProjection.r[0].m128_f32[0] * Point.m128_f32[0]) + (ViewProjection.r[1].m128_f32[0] * Point.m128_f32[1])
		+ (ViewProjection.r[2].m128_f32[0] * Point.m128_f32[2]) + ViewProjection.r[3].m128_f32[0];
	float y = (ViewProjection.r[0].m128_f32[1] * Point.m128_f32[0]) + (ViewProjection.r[1].m128_f32[1] * Point.m128_f32[1])
		+ (ViewProjection.r[2].m128_f32[1] * Point.m128_f32[2]) + ViewProjection.r[3].m128_f32[1];
	float w = (ViewProjection.r[0].m128_f32[3] * Point.m128_f32[0]) + (ViewProjection.r[1].m128_f32[3] * Point.m128_f32[1])
		+ (ViewProjection.r[2].m128_f32[3] * Point.m128_f32[2]) + ViewProjection.r[3].m128_f32[3];

	float wInverse = 1.0f / w;

	// transform rom [-1,1] to actual viewport dimensions
	point.x = (LONG)((1.0f + (x * wInverse)) * clip_x);
	point.y = (LONG)((1.0f + (y * wInverse)) * clip_y);

	return point;
}

void CCamera::UpdateViewProjection()
{
	// 2D, perspective or orthogonal mode
	if (Mode == RENGINEMODE::Mode_Perspective)
	{
		ViewProjection = ProjectionPerspective[Stage] * View3D;
	}
	else if (Mode == RENGINEMODE::Mode_2D)
	{
		ViewProjection =  Projection2D * View2D;
	}
	else
	{
		ViewProjection =  ProjectionOrthogonal[Stage] * View3D;
	}
}
const std::array<DirectX::XMMATRIX, nStages>& CCamera::GetProjectionPerspective()
{
	return ProjectionPerspective;
}
const std::array<DirectX::XMMATRIX, nStages>& CCamera::GetProjectionOrthogonal()
{
	return	ProjectionOrthogonal;
}
const DirectX::XMMATRIX& CCamera::GetView3D()
{
	return View3D;
}
const HRESULT CCamera::GetFrustrumFromViewProjection(std::array<DirectX::XMVECTOR, 6>& frustumPlanes)
{
	DirectX::XMMATRIX ViewProjectionRow = DirectX::XMMatrixTranspose(ViewProjection);
	//left plane
	frustumPlanes[0] = DirectX::XMPlaneNormalize(DirectX::XMVectorNegate(DirectX::XMVectorAdd(ViewProjectionRow.r[3], ViewProjectionRow.r[0])));
	//right plane
	frustumPlanes[1] = DirectX::XMPlaneNormalize(DirectX::XMVectorNegate(DirectX::XMVectorSubtract(ViewProjectionRow.r[3], ViewProjectionRow.r[0])));
	//top plane
	frustumPlanes[2] = DirectX::XMPlaneNormalize(DirectX::XMVectorNegate(DirectX::XMVectorSubtract(ViewProjectionRow.r[3], ViewProjectionRow.r[1])));
	//Buttom plane
	frustumPlanes[3] = DirectX::XMPlaneNormalize(DirectX::XMVectorNegate(DirectX::XMVectorAdd(ViewProjectionRow.r[3], ViewProjectionRow.r[1])));
	//near  plane
	frustumPlanes[4] = DirectX::XMPlaneNormalize(DirectX::XMVectorNegate(ViewProjectionRow.r[2]));
	//far plane
	frustumPlanes[5] = DirectX::XMPlaneNormalize(DirectX::XMVectorNegate(DirectX::XMVectorSubtract(ViewProjectionRow.r[3], ViewProjectionRow.r[1])));

	return DXR_OK;
}

/**
 * Cast a world ray rom a given position on screen.
 *  screen to worldray, give 2 vectors or output
 * -> IN:  POINT      - point in screen coordinates
 * -> OUT: ZFXVector& - to ill with 4 floats or origin vextor
 *         ZFXVector& - to ill with 4 floats or ray direction
 */
void CCamera::Transform2Dto3D(const POINT& point, DirectX::XMVECTOR& outOrigin, DirectX::XMVECTOR& outDirection)
{
	DirectX::XMVECTOR resize;
	float   width;
	float	height;

	if (Mode == RENGINEMODE::Mode_2D)
	{
		width = Width;
		height = Height;
	}
	// else ortho or perspective projection
	else
	{
		width = ViewPort[Stage].Width;
		height = ViewPort[Stage].Height;
	}

	// resize to viewportspace [-1,1] -> projection
	resize.m128_f32[0] = (((point.x * 2.0f) / width) - 1.0f) / ProjectionPerspective[Stage].r[0].m128_f32[0];
	resize.m128_f32[1] = -(((point.y * 2.0f) / height) - 1.0f) / ProjectionPerspective[Stage].r[1].m128_f32[1];
	resize.m128_f32[2] = 1.0f;

	DirectX::XMMATRIX inverseView = DirectX::XMMatrixInverse(nullptr, View3D);

	// ray from screen to worldspace
	outDirection.m128_f32[0] = (resize.m128_f32[0] * inverseView.r[0].m128_f32[0])
		+ (resize.m128_f32[1] * inverseView.r[1].m128_f32[0])
		+ (resize.m128_f32[2] * inverseView.r[2].m128_f32[0]);
	outDirection.m128_f32[1] = (resize.m128_f32[0] * inverseView.r[0].m128_f32[1])
		+ (resize.m128_f32[1] * inverseView.r[1].m128_f32[1])
		+ (resize.m128_f32[2] * inverseView.r[2].m128_f32[1]);
	outDirection.m128_f32[2] = (resize.m128_f32[0] * inverseView.r[0].m128_f32[2])
		+ (resize.m128_f32[1] * inverseView.r[1].m128_f32[2])
		+ (resize.m128_f32[2] * inverseView.r[2].m128_f32[2]);

	// inverse translation.
	outOrigin.m128_f32[0] = inverseView.r[3].m128_f32[0];
	outOrigin.m128_f32[1] = inverseView.r[3].m128_f32[1];
	outOrigin.m128_f32[2] = inverseView.r[3].m128_f32[2];

	outOrigin = DirectX::XMVector3Normalize(outOrigin);
}