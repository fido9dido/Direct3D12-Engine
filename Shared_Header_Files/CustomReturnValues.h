
// everything went smooth
#define DXR_OK                  S_OK

// just reports no errors
#define DXR_CANCELED            0x02000001

// generell error message
#define DXR_FAIL                0x82000001

// specific error messages
#define DXR_CREATEAPI           0x82000002
#define DXR_CREATEDEVICE        0x82000003
#define DXR_INVALIDPARAM        0x82000004
#define DXR_INVALIDID			0x82000005
#define DXR_NODEPTHSTENCIL		0x82000006 
#define DXR_FAILEXIST			0x82000007
#define DXR_BUFFERSIZE          0x82000008
#define DXR_FAIL8               0x82000009
#define DXR_FAIL9               0x8200000a
#define DXR_FAIL0               0x8200000b
#define DXR_FAILa               0x8200000c
#define DXR_FAILb               0x8200000d
#define DXR_FAILc               0x8200000e
#define DXR_FAILd               0x8200000f



// simple viewport type
//typedef struct  DXRIEWPORT_TYPE
//{
//	DWORD X; 
//	DWORD Y; 
//	DWORD Width;
//	DWORD Height;
//} DXRVIEWPORT;