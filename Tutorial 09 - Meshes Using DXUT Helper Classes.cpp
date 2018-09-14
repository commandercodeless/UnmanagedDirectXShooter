//**************************************************************************//
// This is a modified version of the Microsoft sample code and loads a mesh.//
// it uses the helper class CDXUTSDKMesh, as there is no longer any built in//
// support for meshes in DirectX 11.										//
//																			//
// The CDXUTSDKMesh is NOT DorectX, not is the file format it uses, the		//
// .sdkmesh, a standard file format. You will hnot find the .sdkmesh format	//
// outside the MS sample code.  Both these things are provided as an entry	//
// point only.																//
//																			//
// Look for the Nigel style comments, like these, for the bits you need to  //
// look at.																	//
//																			//
// You may notice that this sample tries to create a DirectX11 rendering	//
// device, and if it can't do that creates a DirectX 9 device.  I'm not		//
// using DirectX9.															//
//**************************************************************************//


//**************************************************************************//
// Modifications to the MS sample code is copyright of Dr Nigel Barlow,		//
// lecturer in computing, University of Plymouth, UK.						//
// email: nigel@soc.plymouth.ac.uk.											//
//																			//
// Sdkmesh added to MS sample Tutorial09.									//
//																			//
// You may use, modify and distribute this (rather cack-handed in places)	//
// code subject to the following conditions:								//
//																			//
//	1:	You may not use it, or sell it, or use it in any adapted form for	//
//		financial gain, without my written premission.						//
//																			//
//	2:	You must not remove the copyright messages.							//
//																			//
//	3:	You should correct at least 10% of the typig abd spekking errirs.   //
//**************************************************************************//


//--------------------------------------------------------------------------------------
// File: Tutorial 09 - Meshes Using DXUT Helper Classes.cpp
//
// This sample shows a simple example of the Microsoft Direct3D's High-Level 
// Shader Language (HLSL) using the Effect interface. 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include <xnamath.h>
#include "resource.h"
#include "Windows.h"
#include "Winuser.h"
#include "Windowsx.h"
#include "WinDef.h"
#include <vector>

//obj includes
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <fstream>		// Files.  The ones without ".h" are the new (not
#include <string>		// so new now) standard library headers.
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <vector>
#include <stdio.h>
#include "resource.h"

#define WM_MOUSEMOVE                    0x0200
#define WM_INPUT                        0x00FF

RAWINPUTDEVICE Rid[2];

int team0Score = 0;
int team1Score = 0;
//obj structs
//**************************************************************************//
// Nothing is easy in DirectX.  Before we can even create a single vertex,	//
// we need to define what it looks like.									//
//																			//
// The data types seems to be inhereted from XNA.							//
// An XMFLOAT3 is a float containing 3 numbers, an x, y, x position here.	//
// An XMFLOAT4 is a float containing 4 values, an RGBA colour.	Not that	//
// alpha effects work without additional effort.							//
//**************************************************************************//
struct SimpleVertex
{
	XMFLOAT3 Pos;	//Why not a float4?  See the shader strucrure.  Any thoughts?  Nigel
	XMFLOAT3 VecNormal;
	XMFLOAT2 TexUV;
};


struct RenderTarget{
	int distanceFromCamera;
	int type; //0 player, 1 enemy, 2 cube, 3 projectile
	int number;
};

//**************************************************************************//
// A sort of mesh subset, basically an array of vertices and indexes.		//
//**************************************************************************//
struct SortOfMeshSubset
{
	SimpleVertex *vertices;
	USHORT       *indexes;
	USHORT       numVertices;
	USHORT       numIndices;
	std::wstring	 materialFile;
	std::wstring	 textureFile;
};
int              g_numIndices = 0;	// Need to record the number of indices
// for drawing mesh.


//**************************************************************************//
// Global Variables.  There are many global variables here (we aren't OO	//
// yet.  I doubt  Roy Tucker (Comp Sci students will know him) will			//
// approve pf this either.  Sorry, Roy.										//
//**************************************************************************//
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_D3DSettingsDlg;       // Device settings dialog
CDXUTDialog                 g_HUD;                  // manages the 3D   
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls
CModelViewerCamera          g_Camera;				// Not used by Nigel.
CDXUTDirectionWidget        g_LightControl;			// Not used by Nigel.


float                       g_fLightScale;
int                         g_nNumActiveLights;
int                         g_nActiveLight;
bool                        g_bShowHelp = false;    // If true, it renders the UI control text
int							g_width  = 800;
int							g_height = 600;


int pi = 3.14159265358979323846;

//**************************************************************************//
// Meshes here.																//
//**************************************************************************//
CDXUTSDKMesh				g_MeshSkyBox;
CDXUTSDKMesh				g_MeshProjectile;
CDXUTSDKMesh				g_MeshCube;
CDXUTSDKMesh				g_MeshRamp;
CDXUTSDKMesh				g_MeshCornerIn;
CDXUTSDKMesh				g_MeshCornerOut;
CDXUTSDKMesh				g_MeshEnemy;

float g_f_TigerX = 0;
float g_f_TigerY = 0;
float g_f_TigerZ = 0;

float g_f_TigerSpeed = 2;

XMVECTOR g_vecTigerInitialDir = XMVectorSet(0, 0, -1, 0);

XMMATRIX					g_MatProjection;

ID3D11InputLayout          *g_pVertexLayout11 = NULL;
ID3D11Buffer               *g_pVertexBuffer   = NULL;
ID3D11Buffer               *g_pIndexBuffer    = NULL;
ID3D11VertexShader         *g_pVertexShader   = NULL;
ID3D11PixelShader          *g_pPixelShader    = NULL;	//this
ID3D11PixelShader          *g_pSkyPixelShader = NULL;
ID3D11SamplerState         *g_pSamLinear      = NULL;

//**********************************************************************//
// Variables to control the movement of the tiger.						//
// The only one I have coded is rotate about Y, we need an x, y, z		//
// position and maybe rotates about other axes.							//
//**********************************************************************//
float		 g_f_TigerRY            = XMConvertToRadians(45);  //45º default
float		 g_f_TigerRX			= XMConvertToRadians(45);  //45º default

bool		 g_b_LeftArrowDown      = false;	//Status of keyboard.  Thess are set
bool		 g_b_RightArrowDown     = false;	//in the callback KeyboardProc(), and 
bool		 g_b_UpArrowDown	    = false;	//are used in onFrameMove().
bool		 g_b_DownArrowDown	    = false;

bool		 g_b_WKeyDown			= false;
bool		 g_b_AKeyDown			= false;
bool		 g_b_SKeyDown			= false;
bool		 g_b_DKeyDown			= false;
bool		 g_b_SpaceKeyDown		= false;
bool		 g_b_LeftMouseDown		= false;

RECT rcClient;                 // client area rectangle 
POINT ptClientUL;              // client upper left corner 
POINT ptClientLR;              // client lower right corner 

//**************************************************************************//
// This is M$ code, but is usuig old D3DX from DirectX9.  I'm glad to see   //
// that M$ are having issues updating their sample code, same as me - Nigel.//
//**************************************************************************//
CDXUTTextHelper*            g_pTxtHelper = NULL;


//**************************************************************************//
// This is a structure we pass to the vertex shader.  						//
// Note we do it properly here and pass the WVP matrix, rather than world,	//
// view and projection matrices separately.									//
//**************************************************************************//
struct CB_VS_PER_OBJECT
{
	XMMATRIX matWorldViewProj;
    XMMATRIX matWorld;				// needed to transform the normals.
};




//**************************************************************************//
// These are structures we pass to the pixel shader.  						//
// Note we do it properly here and pass the WVP matrix, rather than world,	//
// view and projection matrices separately.									//
//																			//
// These structures must be identical to those defined in the shader that	//
// you use.  So much for encapsulation; Roy	Tucker (Comp Sci students will  //
// know him) will not approve.												//
//**************************************************************************//
struct CB_PS_PER_OBJECT
{
    XMFLOAT4 m_vObjectColor;
};
UINT                        g_iCBPSPerObjectBind = 0;

struct CB_PS_PER_FRAME
{
    XMVECTOR m_vLightDirAmbient;	// Vector pointing at the light
};


struct MexhVertexStructure
{
	XMFLOAT4 pos;
	XMFLOAT3 normal;
	XMFLOAT2 TextureUV;
};

UINT                        g_iCBPSPerFrameBind = 1;

//Structure to hold the location of a waypoint
struct WayPoint
{
	int id;
	XMFLOAT4 position;
	int connections[4];
	int numberOfConnections;
};

//Structure to hold data relating to an AIPlayer
struct AIPlayer
{
	int id;
	XMFLOAT4 position;
	int Health;
	int target;
	char modelPath;
	WayPoint lastWaypoint;
	WayPoint nextWaypoint;
	XMFLOAT4 velocity;
	CDXUTSDKMesh g_MeshModel;
	float g_f_ModelRY = XMConvertToRadians(45);
	float g_f_ModelRX = XMConvertToRadians(45);
	int cooldown;
	int respawn;
	int team;
	int speed;

	float fallSpeed;
};

struct Player
{
	XMFLOAT4 position;
	int Health;
	XMFLOAT4 velocity;
	CDXUTSDKMesh g_MeshModel;
	float g_f_ModelRY = XMConvertToRadians(45);
	float g_f_ModelRX = XMConvertToRadians(45);
	float speed;
	int cooldown;
	int respawn;
	int team;

	float fallSpeed;
};

struct Bullet
{
	XMFLOAT4 position;
	CDXUTSDKMesh g_MeshModel;
	float g_f_ModelRY = XMConvertToRadians(45);
	float g_f_ModelRX = XMConvertToRadians(45);
	float speed;
	int owner;
	int team;
	bool active;
};

struct Cube
{
	XMFLOAT4 position;
	CDXUTSDKMesh g_MeshModel;
	XMMATRIX scale;
	float scaleX;
	float scaleY;
	float scaleZ;
	float rotation;
	float elevation;
	int type;
	int locationX;
	int locationZ;
};
//**************************************************************************//
// Now a global instance of each constant buffer.							//
//**************************************************************************//
ID3D11Buffer               *g_pcbVSPerObject = NULL;
ID3D11Buffer               *g_pcbPSPerObject = NULL;
ID3D11Buffer               *g_pcbPSPerFrame  = NULL;


std::vector<WayPoint> waypoints;
AIPlayer enemies[10];
Player gamePlayer;
std::vector<Bullet>  projectiles;
Cube cubes[400];
Bullet bullets[100] = {};


int worldsizex = 20;
int worldsizez = 20;
int worldheight[20][20] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 1, 2, 2, 2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 1, 1, 2, 2 },
	{ 0, 0, 1, 1, 1, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 1, 1, 2 },
	{ 0, 0, 1, 2, 1, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 4, 4, 4, 4, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 4, 4, 4, 4 },
	{ 4, 4, 4, 4, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 4, 4, 4, 4 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0 },
	{ 1, 1, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 1, 2, 1, 0, 0 },
	{ 2, 1, 1, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 1, 1, 1, 0, 0 },
	{ 2, 2, 1, 1, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 2, 2, 1, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

int worldscale = 2;

int waypointLocations[27][3] = {
	{0,4,0}, //center
	{ 10*worldscale, 4, -10*worldscale },  //four raised platforms
	{ -10*worldscale, 4, 10*worldscale },
	{ -7*worldscale, 4, -7*worldscale },
	{ 7*worldscale, 4, 7*worldscale },
	{ -10*worldscale, 0, -10*worldscale }, //top left corner
	{ 10*worldscale, 0, 10*worldscale }, //bottom right corner
	{ -10*worldscale, 0, -2*worldscale }, //two mid left
	{ -10*worldscale, 0, 2*worldscale },
	{ -2*worldscale, 0, -10*worldscale }, //two mid top
	{ 2*worldscale, 0, -10*worldscale },
	{ 10*worldscale, 0, -2*worldscale }, //two mid right
	{ 10*worldscale, 0, 2*worldscale },
	{ -2*worldscale, 0, 10*worldscale }, //two mid bottom
	{ 2*worldscale, 0, 10*worldscale },
	{ -6*worldscale, 0, -2*worldscale }, //two mid left in
	{ -6*worldscale, 0, 2*worldscale },
	{ 6*worldscale, 0, -2*worldscale }, //two mid right in
	{ 6*worldscale, 0, 2*worldscale },
	{ -2*worldscale, 0, -6*worldscale }, //two mid top in
	{ 2*worldscale, 0, -6*worldscale },
	{ -2*worldscale, 0, 6*worldscale }, //two mid bottom in
	{ 2*worldscale, 0, 6*worldscale },
	{ 4*worldscale, 0, 4*worldscale }, //four diagonals
	{ -4*worldscale, 0, 4*worldscale },
	{ 4*worldscale, 0, -4*worldscale },
	{ -4*worldscale, 0, -4*worldscale }
};

int waypointConnections[27][4] = {
	{26,25,24,23},
	{ 10, 25, 11, -1},
	{ 8, 13, 24, -1},
	{ 5, 26, -1, -1 },
	{ 6, 23, -1, -1 },
	{ 3, 7, 9, -1 },
	{ 4, 12, 14, -1 },
	{ 5, 15, -1, -1 },
	{ 2, 16, -1, -1 },
	{ 5, 19, -1, -1 },
	{ 1, 20, -1, -1 },
	{ 1, 17, -1, -1 },
	{ 6, 19, -1, -1 },
	{ 2, 21, -1, -1 },
	{ 6, 22, -1, -1 },
	{ 7, 16, 26, -1 },
	{ 8, 15, 24, -1 },
	{ 11, 18, 25, -1 },
	{ 12, 17, 23, -1 },
	{ 9, 20, 26, -1 },
	{ 10, 19, 25, -1 },
	{ 13, 22, 24, -1 },
	{ 14, 21, 23, -1 },
	{ 0, 4, 18, 22},
	{ 0, 2, 16, 21},
	{ 0, 1, 17, 20},
	{ 0, 3, 15, 19}
};


int numberOfWaypoints = 0;
int numberOfEnemies = 0;
int numberOfProjectiles = 0;
int numberOfCubes = 0;
//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4

//depth buffer?
D3DPRESENT_PARAMETERS d3dpp;


//**************************************************************************//
// If you are not used to "C" you will find that functions (or methods in	//
// "C++" must have templates defined in advance.  It is usual to define the //
// prototypes in a header file, but we'll put them here for now to keep		//
// things simple.															//
//**************************************************************************//
//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


extern bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                             bool bWindowed, void* pUserContext );
extern HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice,
                                            const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
extern HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                           void* pUserContext );
extern void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime,
                                        void* pUserContext );
extern void CALLBACK OnD3D9LostDevice( void* pUserContext );
extern void CALLBACK OnD3D9DestroyDevice( void* pUserContext );

bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                  float fElapsedTime, void* pUserContext );

void RenderWorkingMesh(ID3D11DeviceContext *pContext,
	CDXUTSDKMesh         *toRender,
	ID3D11PixelShader *PixelShader);

void InitApp();
void RenderText();
void charStrToWideChar(WCHAR *dest, char *source);
void RenderMesh (ID3D11DeviceContext* pd3dImmediateContext, CDXUTSDKMesh *toRender, ID3D11PixelShader *PixelShader);

int checkBlocks(int x, int y);

int findRotation1(int x, int y);
int findRotation2(int x, int y);
int findRotation3(int x, int y);


float MyAtan2(float y, float x);
std::wstring getTexture(std::wstring dir, std::wstring materialFile);

//**************************************************************************//
// A Windows program always kicks off in WinMain.							//
// Initializes everything and goes into a message processing				//
// loop.																	//
//																			//
// This version uses DXUT, and is much more complicated than previous		//
// versions you have seen.  This allows you to sync the frame rate to your  //
// monitor's vertical sync event.											//
//**************************************************************************//
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D11) 
    // that is available on the system depending on which D3D callbacks are set below

	
	ZeroMemory(&d3dpp, sizeof(d3dpp)); //depth buffer initialiser?
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;


	//**************************************************************************//
	// Set DXUT callbacks.														//
    //**************************************************************************//
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );


    DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );


    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    InitApp();
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"Tutorial 09 - Meshes Using DXUT Helper Classes" );
    DXUTCreateDevice (D3D_FEATURE_LEVEL_9_2, true, 800, 600 );
    //DXUTCreateDevice(true, 640, 480);
    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{


    // Initialize dialogs
    g_D3DSettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 23 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += 26, 170, 23, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += 26, 170, 23, VK_F2 );

	// Keyboard
	Rid[0].usUsagePage = 1;
	Rid[0].usUsage = 6;
	Rid[0].dwFlags = 0;
	Rid[0].hwndTarget = NULL;

	// Mouse
	Rid[1].usUsagePage = 1;
	Rid[1].usUsage = 2;
	Rid[1].dwFlags = 0;
	Rid[1].hwndTarget = NULL;

	for (int i = 0; i < 27; i++){
		WayPoint newWaypoint;
		newWaypoint.id = i;
		newWaypoint.position = XMFLOAT4(waypointLocations[i][0], waypointLocations[i][1], waypointLocations[i][2], 0);
		if (newWaypoint.position.x > 20){
			newWaypoint.position.x = 19;
		}
		if (newWaypoint.position.z > 20){
			newWaypoint.position.z = 19;
		}
		newWaypoint.connections[0] = waypointConnections[i][0];
		newWaypoint.connections[1] = waypointConnections[i][1];
		newWaypoint.connections[2] = waypointConnections[i][2];
		newWaypoint.connections[3] = waypointConnections[i][3];
		newWaypoint.numberOfConnections = 0;
		for (int i = 0; i < 4; i++){
			if (newWaypoint.connections[i] != -1){
				newWaypoint.numberOfConnections++;
			}
		}
		waypoints.push_back(newWaypoint);
		numberOfWaypoints++;
	}

	for (int i = 0; i < 8; i++){
		AIPlayer newAI;
		newAI.id = i;
		newAI.cooldown = 0;
		newAI.Health = 100;
		newAI.g_f_ModelRX = XMConvertToRadians(0);
		newAI.g_f_ModelRY = XMConvertToRadians(0);
		newAI.respawn = 0;
		if (i < 8 / 2){
			newAI.team = 0;
		}
		else{
			newAI.team = 1;
		}
		newAI.g_MeshModel = g_MeshEnemy;
		int waypoint = rand() % numberOfWaypoints;
		newAI.position = waypoints[waypoint].position;
		newAI.lastWaypoint = waypoints[waypoint];
		newAI.nextWaypoint = waypoints[waypoints[waypoint].connections[rand() % waypoints[waypoint].numberOfConnections]];
		newAI.target = -1;
		enemies[i] = newAI;
		numberOfEnemies++;
	}
	gamePlayer.speed = 0;
	gamePlayer.Health = 100;
	int waypoint = rand() % numberOfWaypoints;
	gamePlayer.position = waypoints[waypoint].position;
	gamePlayer.respawn = 0;
	gamePlayer.team = 0;
	gamePlayer.cooldown = 0;
	gamePlayer.g_f_ModelRX = XMConvertToRadians(0);
	gamePlayer.g_f_ModelRY = XMConvertToRadians(0);

	worldsizex = 20;
	worldsizez = 20;

	for (int i = 0; i < worldsizex; i++){
		for (int j = 0; j < worldsizez; j++){
			Cube newCube;
			newCube.position.x = (i - (worldsizex / 2)) * worldscale;
			newCube.position.z = (j - (worldsizez / 2)) * worldscale;
			newCube.scaleX = worldscale;
			newCube.scaleZ = worldscale;

			newCube.locationX = i;
			newCube.locationZ = j;

			newCube.rotation = 0;

			if (worldheight[i][j] == 0){
				newCube.type = 0;
				newCube.position.y = -2;
				newCube.scaleY = 1;
				newCube.g_MeshModel = g_MeshCube;
			}
			if (worldheight[i][j] == 1){
				newCube.position.y = -1;
				newCube.scaleY = 1;
			}
			if (worldheight[i][j] == 2){
				newCube.type = 0;
				newCube.position.y = 0;
				newCube.scaleY = 2 * worldscale;
				newCube.g_MeshModel = g_MeshCube;
			}
			if (worldheight[i][j] == 3){
				newCube.position.y = 0;
				newCube.scaleY = worldscale;
			}
			if (worldheight[i][j] == 4){
				newCube.type = 4;
				newCube.position.y = 0;
				newCube.scaleY = 4 * worldscale;
				newCube.g_MeshModel = g_MeshCube;
			}
			cubes[(i*20) + j] = newCube;
			numberOfCubes++;
		}
	}

	for (int i = 0; i < 100; i++){
		bullets[i].position = XMFLOAT4(0, 0, 0, 0);
		bullets[i].g_f_ModelRY = 0;
		bullets[i].g_f_ModelRX = 0;
		bullets[i].speed = 0;
		bullets[i].owner = -2;
		bullets[i].team = -1;
		bullets[i].active = false;
	}
	if (RegisterRawInputDevices(Rid, 2, sizeof(RAWINPUTDEVICE)) == FALSE){}
	SetCapture;
    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;
}


int checkBlocks(int x, int y){
	int resultH = 0;
	int result = 0;

	for (int i = -1; i < 2; i++){
		if (worldheight[x + i][y] == 2){
			resultH++;
		}
	}
	for (int j = -1; j < 2; j++){
		if (worldheight[x][y + j] == 2){
			resultH++;
		}
	}

	for (int i = -1; i < 2; i+=2){
		for (int j = -1; j < 2; j+=2){
			if (worldheight[x + i][y + j] == 2){
				result++;
			}
		}
	}

	if (result == resultH){
		return 0;
	}
	else {
		if (resultH == 0 && result == 1){
			return 1;
		}
		else{
			return 2;
		}
	}
}

int findRotation1(int x, int y){
	if (worldheight[x - 1][y] == 2){
		return 180;
	}
	if (worldheight[x + 1][y] == 2){
		return 0;
	}
	if (worldheight[x][y + 1] == 2){
		return -90;
	}
	if (worldheight[x][y - 1] == 2){
		return 90;
	}
}

int findRotation2(int x, int y){
	if (worldheight[x - 1][y - 1] == 2){
		return 180;
	}
	if (worldheight[x + 1][y - 1] == 2){
		return 90;
	}
	if (worldheight[x - 1][y + 1] == 2){
		return -90;
	}
	if (worldheight[x + 1][y + 1] == 2){
		return 0;
	}
}

int findRotation3(int x, int y){
	if (worldheight[x - 1][y] == 2 && worldheight[x][y + 1] == 2){
		return -90;
	}
	if (worldheight[x + 1][y] == 2 && worldheight[x][y + 1] == 2){
		return 0;
	}
	if (worldheight[x + 1][y] == 2 && worldheight[x][y - 1] == 2){
		return 90;
	}
	if (worldheight[x - 1][y] == 2 && worldheight[x][y - 1] == 2){
		return 180;
	}
}
//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    // Uncomment this to get debug information from D3D11
    //pDeviceSettings->d3d11.CreateFlags |= D3D11_CREATE_DEVICE_DEBUG;

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( ( DXUT_D3D11_DEVICE == pDeviceSettings->ver &&
              pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE ) )
        {
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
        }
    }

    return true;
}

//**************************************************************************//
// Handle updates to the scene.  This is called regardless of which D3D		//
// API is used (we are only using Dx11).									//
//**************************************************************************//
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	if (g_b_WKeyDown){ gamePlayer.speed = 2; }
	else{
		if (g_b_SKeyDown){ gamePlayer.speed = -2; }
		else{ gamePlayer.speed = 0; }
	}
	if (g_b_AKeyDown){ gamePlayer.g_f_ModelRY -= fElapsedTime * 3; }

	if (g_b_DKeyDown){ gamePlayer.g_f_ModelRY += fElapsedTime * 3; }

	if (g_b_LeftMouseDown || g_b_SpaceKeyDown) {
		if (gamePlayer.cooldown <= 0){
			int fired = 0;
			for (int i = 0; i < 100; i++){
				if (bullets[i].active == false && fired == 0){
					fired++;

					bullets[i].position = gamePlayer.position;
					bullets[i].g_f_ModelRY = gamePlayer.g_f_ModelRY;
					bullets[i].g_f_ModelRX = gamePlayer.g_f_ModelRX;
					bullets[i].speed = 5;
					bullets[i].owner = -1;
					gamePlayer.cooldown = 30;
					bullets[i].team = gamePlayer.team;
					bullets[i].active = true;
				}
			}
		}
	}

	float angleOfTarget;
	for (int i = 0; i < numberOfEnemies; i++){
		if (enemies[i].target == -1){
			angleOfTarget = -atan2((enemies[i].position.z - enemies[i].nextWaypoint.position.z), (enemies[i].position.x - enemies[i].nextWaypoint.position.x));
			enemies[i].g_f_ModelRY = angleOfTarget;
			enemies[i].g_f_ModelRX = XMConvertToRadians(0);
			enemies[i].speed = 2;
		} 
		if (enemies[i].target == 0){
			angleOfTarget = -atan2(enemies[i].position.z - gamePlayer.position.z, enemies[i].position.x - gamePlayer.position.x) * 180/pi;
			enemies[i].g_f_ModelRY = angleOfTarget;
			if (enemies[i].position.z - gamePlayer.position.z > 2 || enemies[i].position.x - gamePlayer.position.x > 2){
				enemies[i].speed = 1;
			}
			else{
				enemies[i].speed = 0;
			}
		}
		if (enemies[i].target > 0){
			int target = enemies[i].target - 1;
			angleOfTarget = -atan2(enemies[i].position.z - enemies[target].position.z, enemies[i].position.x - enemies[target].position.x) * 180/pi;
			enemies[i].g_f_ModelRY = angleOfTarget;
			if (enemies[i].position.z - enemies[target].position.z > 2 || enemies[i].position.x - enemies[target].position.x > 2){
				enemies[i].speed = 1;
			}
			else{
				enemies[i].speed = 0;
			}

		}
		enemies[i].g_f_ModelRY += 90;
	}
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{
    UINT nBackBufferHeight = ( DXUTIsAppRenderingWithD3D9() ) ? DXUTGetD3D9BackBufferSurfaceDesc()->Height :
            DXUTGetDXGIBackBufferSurfaceDesc()->Height;

    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 2, 0 );
    g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );

    // Draw help
    if( g_bShowHelp )
    {
        g_pTxtHelper->SetInsertionPos( 2, nBackBufferHeight - 20 * 6 );
        g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
        g_pTxtHelper->DrawTextLine( L"Controls:" );

        g_pTxtHelper->SetInsertionPos( 20, nBackBufferHeight - 20 * 5 );
        g_pTxtHelper->DrawTextLine( L"Move : WASD keys\n"
                                    L"Shoot : Space\n" );

        g_pTxtHelper->SetInsertionPos( 550, nBackBufferHeight - 20 * 5 );
        g_pTxtHelper->DrawTextLine( L"Hide help: F1\n"
                                    L"Quit: ESC\n" );
    }
    else
    {
        g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        g_pTxtHelper->DrawTextLine( L"Press F1 for help" );
    }

    g_pTxtHelper->End();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;


    return 0;
}







//**************************************************************************//
// Handle key presses.														//
//**************************************************************************//
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1:
                g_bShowHelp = !g_bShowHelp; break;

      }
    }

	
	//**************************************************************//
	// Nigel code to rotate the tiger.								//
	//**************************************************************//
	switch( nChar )
	{		       
		case VK_LEFT:  g_b_LeftArrowDown  = bKeyDown; break;
		case VK_RIGHT: g_b_RightArrowDown = bKeyDown; break;
		case VK_UP:    g_b_UpArrowDown    = bKeyDown; break;
		case VK_DOWN:  g_b_DownArrowDown  = bKeyDown; break;
		case VK_LBUTTON: g_b_LeftMouseDown = bKeyDown; break;
		case VK_SPACE: g_b_SpaceKeyDown = bKeyDown; break;
		case 0x57: g_b_WKeyDown = bKeyDown; break;
		case 0x41: g_b_AKeyDown = bKeyDown; break;
		case 0x53: g_b_SKeyDown = bKeyDown; break;
		case 0x44: g_b_DKeyDown = bKeyDown; break;
       }
}



//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:
            DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:
            g_D3DSettingsDlg.SetActive( !g_D3DSettingsDlg.IsActive() ); break;
    }

}


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}



//**************************************************************************//
// Compile the shader file.  These files aren't pre-compiled (well, not		//
// here, and are compiled on he fly).										//
//**************************************************************************//
HRESULT CompileShaderFromFile( WCHAR* szFileName,		// File Name
							  LPCSTR szEntryPoint,		// Namee of shader
							  LPCSTR szShaderModel,		// Shader model
							  ID3DBlob** ppBlobOut )	// Blob returned
{
    HRESULT hr = S_OK;

    // find the file
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, szFileName ) );

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( str, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
		WCHAR errorCharsW[200];
        if( pErrorBlob != NULL )
		{
			charStrToWideChar(errorCharsW, (char *)pErrorBlob->GetBufferPointer());
            MessageBox( 0, errorCharsW, L"Error", 0 );
		}
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    SAFE_RELEASE( pErrorBlob );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext )
{
    HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );
    g_pTxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );





	//**********************************************************************//
    // Compile the pixel and vertex shaders.  If your computer doesn't		//
	// support shader model 5, try shader model 4.  There is nothing we are //
	// using here that needs shader model 5.								//
	//**********************************************************************..
    ID3DBlob* pVertexShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"Tutorial 09 - Meshes Using DXUT Helper Classes_VS.hlsl", "VS_DXUTSDKMesh", "vs_5_0", &pVertexShaderBuffer ) );

    ID3DBlob* pPixelShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"Tutorial 09 - Meshes Using DXUT Helper Classes_PS.hlsl", "PS_DXUTSDKMesh", "ps_5_0", &pPixelShaderBuffer ) );

	ID3DBlob* pSkyPixelShaderBuffer = NULL;
	V_RETURN(CompileShaderFromFile(L"Tutorial 09 - Meshes Using DXUT Helper Classes_PS.hlsl", "PS_DXUTSDKMesh2", "ps_5_0", &pSkyPixelShaderBuffer));
    
	//g_pSkyPixelShader
	//**********************************************************************//
    // Create the pixel and vertex shaders.									//
	//**********************************************************************//
	V_RETURN( pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
                                              pVertexShaderBuffer->GetBufferSize(), NULL, &g_pVertexShader ) );
    DXUT_SetDebugName( g_pVertexShader, "VS_DXUTSDKMesh" );
    V_RETURN( pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
                                             pPixelShaderBuffer->GetBufferSize(), NULL, &g_pPixelShader ) );
    DXUT_SetDebugName( g_pPixelShader, "PS_DXUTSDKMesh" );

	V_RETURN(pd3dDevice->CreatePixelShader(pSkyPixelShaderBuffer->GetBufferPointer(),
		pSkyPixelShaderBuffer->GetBufferSize(), NULL, &g_pSkyPixelShader));
	DXUT_SetDebugName(g_pSkyPixelShader, "PS_DXUTSDKMesh2");
	
	
	//**********************************************************************//
    // Define the input layout.  I won't go too much into this except that  //
	// the vertex defined here MUST be consistent with the vertex shader	//
	// input you use in your shader file and the constand buffer structure  //
	// at the top of this module.											//
	//																		//
	// Normal vectors are used by lighting.									//
	//**********************************************************************//
     const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    V_RETURN( pd3dDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), pVertexShaderBuffer->GetBufferPointer(),
                                             pVertexShaderBuffer->GetBufferSize(), &g_pVertexLayout11 ) );
    DXUT_SetDebugName( g_pVertexLayout11, "Primary" );

    SAFE_RELEASE( pVertexShaderBuffer );
    SAFE_RELEASE( pPixelShaderBuffer );
	SAFE_RELEASE(pSkyPixelShaderBuffer);



    //**************************************************************************//
	// Initialize the projection matrix.  Generally you will only want to create//
	// this matrix once and then forget it.										//
	//**************************************************************************//
	g_MatProjection = XMMatrixPerspectiveFovLH( XM_PIDIV2,				// Field of view (pi / 2 radians, or 90 degrees
											 g_width / (FLOAT) g_height, // Aspect ratio.
											 0.01f,						// Near clipping plane.
											 10000.0f );					// Far clipping plane.


	//**************************************************************************//
    // Load the mesh.															//
	//**************************************************************************//
	V_RETURN( g_MeshSkyBox.Create(pd3dDevice,L"Media\\CloudBox\\skysphere.sdkmesh",true));

	for (int i = 0; i < 100; i++){
		V_RETURN(bullets[i].g_MeshModel.Create(pd3dDevice, L"Media\\Bullet\\Bullet.sdkmesh", true));
	}

	for (int i = 0; i < numberOfCubes; i++){
		if (cubes[i].type != 4){
			V_RETURN(cubes[i].g_MeshModel.Create(pd3dDevice, L"Media\\GameWorld\\Cube.sdkmesh", true));
		}
		if (cubes[i].type == 4){
			V_RETURN(cubes[i].g_MeshModel.Create(pd3dDevice, L"Media\\GameWorld\\CubeTall.sdkmesh", true));
		}
	}

	for (int i = 0; i < numberOfEnemies; i++){
		if (enemies[i].team == 1){
			V_RETURN(enemies[i].g_MeshModel.Create(pd3dDevice, L"Media\\Enemy\\Enemy.sdkmesh", true));
		}
		else {
			V_RETURN(enemies[i].g_MeshModel.Create(pd3dDevice, L"Media\\PlayerModel\\enemy.sdkmesh", true));

		}
	}
	V_RETURN( g_MeshProjectile.Create(pd3dDevice, L"Media\\Bullet\\Bullet.sdkmesh", true));

	V_RETURN(g_MeshCube.Create(pd3dDevice, L"Media\\GameWorld\\Cube.sdkmesh", true));
	V_RETURN(g_MeshRamp.Create(pd3dDevice, L"Media\\GameWorld\\Wedge.sdkmesh", true));
	V_RETURN(g_MeshCornerIn.Create(pd3dDevice, L"Media\\GameWorld\\CornerIn.sdkmesh", true));
	V_RETURN(g_MeshCornerOut.Create(pd3dDevice, L"Media\\GameWorld\\CornerOut.sdkmesh", true));

	V_RETURN(g_MeshEnemy.Create(pd3dDevice, L"Media\\Enemy\\Enemy.sdkmesh", true));

	V_RETURN(gamePlayer.g_MeshModel.Create(pd3dDevice, L"Media\\PlayerModel\\enemy.sdkmesh", true));


	
	// Create a sampler state
    D3D11_SAMPLER_DESC SamDesc;
    SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.MipLODBias = 0.0f;
    SamDesc.MaxAnisotropy = 1;
    SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0;
    SamDesc.MinLOD = 0;
    SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
    V_RETURN( pd3dDevice->CreateSamplerState( &SamDesc, &g_pSamLinear ) );
    DXUT_SetDebugName( g_pSamLinear, "Primary" );

    
	//**************************************************************************//
	// Create the 3 constant bufers, using the same buffer descriptor to create //
	// all three.																//
	//**************************************************************************//
    D3D11_BUFFER_DESC Desc;
 	ZeroMemory( &Desc, sizeof(Desc) );
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = 0;
    Desc.MiscFlags = 0;
	
    Desc.ByteWidth = sizeof( CB_VS_PER_OBJECT );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pcbVSPerObject ) );
    DXUT_SetDebugName( g_pcbVSPerObject, "CB_VS_PER_OBJECT" );

    Desc.ByteWidth = sizeof( CB_PS_PER_OBJECT );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pcbPSPerObject ) );
    DXUT_SetDebugName( g_pcbPSPerObject, "CB_PS_PER_OBJECT" );

    Desc.ByteWidth = sizeof( CB_PS_PER_FRAME );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pcbPSPerFrame ) );
    DXUT_SetDebugName( g_pcbPSPerFrame, "CB_PS_PER_FRAME" );


    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

	g_width  = pBackBufferSurfaceDesc->Width;
	g_height = pBackBufferSurfaceDesc->Height;

	g_HUD.SetLocation( g_width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
	g_SampleUI.SetLocation( g_width - 170, g_height - 300 );
    g_SampleUI.SetSize( 170, 300 );

    return S_OK;
}




//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                  float fElapsedTime, void* pUserContext )
{
	std::vector<RenderTarget> listToRender;
    HRESULT hr;

    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.OnRender( fElapsedTime );
        return;
    }

    // Clear the render target and depth stencil
    float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.55f };
    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	//things that would go in the next loop, but are the same for each loop and so only need be defined once




	CB_VS_PER_OBJECT CBMatrices;

	float    fAmbient = 0.1f;
	XMVECTOR vectorLightDirection = XMVectorSet(-1, 1, -2, 0);  // 4th value unused.
	vectorLightDirection = XMVector3Normalize(vectorLightDirection);



	CB_PS_PER_FRAME CBPerFrame;
	CBPerFrame.m_vLightDirAmbient = vectorLightDirection;
	pd3dImmediateContext->UpdateSubresource(g_pcbPSPerFrame, 0, NULL, &CBPerFrame, 0, 0);
	pd3dImmediateContext->PSSetConstantBuffers(1, 1, &g_pcbPSPerFrame);


	CB_PS_PER_OBJECT CBPerObject;
	CBPerObject.m_vObjectColor = XMFLOAT4(1, 1, 1, 1);
	pd3dImmediateContext->UpdateSubresource(g_pcbPSPerObject, 0, NULL, &CBPerObject, 0, 0);
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &g_pcbPSPerObject);


	pd3dImmediateContext->PSSetSamplers(0, 1, &g_pSamLinear);

	XMMATRIX matView;
	if (gamePlayer.respawn <= 1){
		//move model
		XMMATRIX matTigerRotate = XMMatrixRotationY(gamePlayer.g_f_ModelRY);
		XMVECTOR vecNewDir;
		vecNewDir = XMVector3TransformCoord(g_vecTigerInitialDir, matTigerRotate);
		vecNewDir = XMVector3Normalize(vecNewDir);

		XMVECTOR vecArse = vecNewDir *= 3;
		vecNewDir *= gamePlayer.speed * fElapsedTime;
		gamePlayer.position.x += XMVectorGetX(vecNewDir); // Weird syntax; can't just
		gamePlayer.position.z += XMVectorGetZ(vecNewDir);

		//collision detection with other players

		for (int i = 0; i < numberOfEnemies; i++){
			if (pow(enemies[i].position.x - gamePlayer.position.x, 2) + pow(enemies[i].position.y - gamePlayer.position.y, 2) + pow(enemies[i].position.z - gamePlayer.position.z, 2) < 3){
				gamePlayer.position.x -= XMVectorGetX(vecNewDir);
				gamePlayer.position.z -= XMVectorGetZ(vecNewDir);
				if (pow(enemies[i].position.x - gamePlayer.position.x, 2) + pow(enemies[i].position.y - gamePlayer.position.y, 2) + pow(enemies[i].position.z - gamePlayer.position.z, 2) < 3){
					gamePlayer.position.x -= XMVectorGetX(vecNewDir);
					gamePlayer.position.z -= XMVectorGetZ(vecNewDir);
				}
			}
		}

		//keep player inside the world
		if (gamePlayer.position.x > 19){
			gamePlayer.position.x = 19;
		}

		if (gamePlayer.position.x < -20){
			gamePlayer.position.x = -20;
		}

		if (gamePlayer.position.z > 19){
			gamePlayer.position.z = 19;
		}

		if (gamePlayer.position.z < -20){
			gamePlayer.position.z = -20;
		}
		gamePlayer.cooldown -= fElapsedTime;

		if (gamePlayer.position.y < 0) {
			gamePlayer.position.y = 0;
		}

		//calculate intended player height
		int positionX = worldsizex / 2 + ceil(gamePlayer.position.x / worldscale) - 1;
		int positionZ = worldsizez / 2 + ceil(gamePlayer.position.z / worldscale) - 1;

		if (positionX < 0){
			positionX = 0;
		}
		if (positionZ < 0){
			positionZ = 0;
		}
		if (positionX > 19){
			positionX = 19;
		}
		if (positionZ > 19){
			positionZ = 19;
		}


		if (worldheight[positionX][positionZ] == 0){
			if (gamePlayer.position.y < 1) {
				gamePlayer.position.y = 1;
				gamePlayer.fallSpeed = 0;
			}
		}
		if (worldheight[positionX][positionZ] == 2){
			if (gamePlayer.position.y < 3) {
				gamePlayer.position.y = 3;
				gamePlayer.fallSpeed = 0;
			}
		}
		if (worldheight[positionX][positionZ] == 1){
			if (gamePlayer.position.y < 2) {
				gamePlayer.position.y = 2;
				gamePlayer.fallSpeed = 0;
			}
		}

		if (gamePlayer.position.y > (worldheight[positionX][positionZ] + 1)){
			gamePlayer.fallSpeed -= 9.8 * fElapsedTime * fElapsedTime;
			gamePlayer.position.y += gamePlayer.fallSpeed;
		}

		if (worldheight[positionX][positionZ] == 4){
			gamePlayer.position.x -= XMVectorGetX(vecNewDir);
			positionX = worldsizez / 2 + ceil(gamePlayer.position.x / worldscale);
				if (worldheight[positionX][positionZ] == 4){
					gamePlayer.position.z -= XMVectorGetZ(vecNewDir);
				}
		}

		//**************************************************************************//
		// Initialize the view matrix.  What you do to the viewer matrix moves the  //
		// viewer, or course.														//
		//																			//
		// The viewer matrix is created every frame here, which looks silly as the	//
		// viewer never moves.  However in general your viewer does move.			//
		//**************************************************************************//
		XMVECTOR Eye = XMVectorSet((gamePlayer.position.x - XMVectorGetX(vecArse)) * 10, (gamePlayer.position.y + 1 + XMVectorGetY(vecArse)) * 10, (gamePlayer.position.z - XMVectorGetZ(vecArse)) * 10, 0.0f);
		XMVECTOR At = XMVectorSet(gamePlayer.position.x * 10, gamePlayer.position.y * 10, gamePlayer.position.z * 10, 0.0f);
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		
		matView = XMMatrixLookAtLH(Eye,	// The eye, or viewer's position.
			At,		// The look at point.
			Up);	// Which way is up.

		//******************************************************************//
		// Create the world matrix for the tiger: just a rotate around	    //
		// the Y axis of 45 degrees.  DirectX does all angles in radians,	// //THIS
		// hence the conversion.  And a translate.							//
		//******************************************************************//
		XMMATRIX matTigerTranslate = XMMatrixTranslation(gamePlayer.position.x, gamePlayer.position.y, gamePlayer.position.z);
		//XMMATRIX matDogFix = XMMatrixTranslation(-1, 0, 0);
		XMMATRIX matTigerScale = XMMatrixScaling(10, 10, 10);
		XMMATRIX matTigerWorld = matTigerRotate * matTigerTranslate * matTigerScale;

		XMMATRIX matWorldViewProjection;
		matWorldViewProjection = matTigerWorld * matView * g_MatProjection;

		CBMatrices.matWorld = XMMatrixTranspose(matTigerWorld);
		CBMatrices.matWorldViewProj = XMMatrixTranspose(matWorldViewProjection);
		pd3dImmediateContext->UpdateSubresource(g_pcbVSPerObject, 0, NULL, &CBMatrices, 0, 0);
		pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject);

		//**************************************************************************//
		// Render the mesh.															// //THIS
		//**************************************************************************//
		RenderMesh(pd3dImmediateContext, &gamePlayer.g_MeshModel, g_pSkyPixelShader);

	}
	else{
		gamePlayer.respawn -= 1 * fElapsedTime;
		if (gamePlayer.respawn <= 1){
			gamePlayer.Health = 100;
			gamePlayer.position = waypoints[rand() % numberOfWaypoints].position;
		}
	}
	//**************************************************************************//
	//Put all of the render and move code in a loop, and loop through every object//
	//**************************************************************************//
	for (int i = 0; i < numberOfEnemies; i++){

		if (enemies[i].respawn <= 1){
			XMMATRIX matTigerRotate = XMMatrixRotationY(XMConvertToRadians(enemies[i].g_f_ModelRY));
			XMVECTOR vecNewDir;
			vecNewDir = XMVector3TransformCoord(g_vecTigerInitialDir, matTigerRotate);
			vecNewDir = XMVector3Normalize(vecNewDir);

			XMVECTOR vecArse = vecNewDir *= 3;
			vecNewDir *= enemies[i].speed * fElapsedTime;
			enemies[i].position.x += XMVectorGetX(vecNewDir); // Weird syntax; can't just
			enemies[i].position.z += XMVectorGetZ(vecNewDir);
			if (enemies[i].position.y < 0) {
				enemies[i].position.y = 0;
			}

			if (enemies[i].position.x > 19){
				enemies[i].position.x = 19;
			}

			if (enemies[i].position.x < -20){
				enemies[i].position.x = -20;
			}

			if (enemies[i].position.z > 19){
				enemies[i].position.z = 19;
			}

			if (enemies[i].position.z < -20){
				enemies[i].position.z = -20;
			}
			int positionX = worldsizex / 2 + ceil(enemies[i].position.x / worldscale) - 1;
			int positionZ = worldsizez / 2 + ceil(enemies[i].position.z / worldscale) - 1;

			if (positionX < 0){
				positionX = 0;
			}
			if (positionZ < 0){
				positionZ = 0;
			}
			if (positionX > 19){
				positionX = 19;
			}
			if (positionZ > 19){
				positionZ = 19;
			}

			if (worldheight[positionX][positionZ] == 0){
				if (enemies[i].position.y < 1) {
					enemies[i].position.y = 1;
					enemies[i].fallSpeed = 0;
				}
			}
			if (worldheight[positionX][positionZ] == 2){
				if (enemies[i].position.y < 3) {
					enemies[i].position.y = 3;
					enemies[i].fallSpeed = 0;
				}
			}
			if (worldheight[positionX][positionZ] == 1){
				if (enemies[i].position.y < 2) {
					enemies[i].position.y = 2;
					enemies[i].fallSpeed = 0;
				}
			}

			if (enemies[i].position.y > (worldheight[positionX][positionZ] + 1)){
				enemies[i].fallSpeed -= 9.8 * fElapsedTime * fElapsedTime;
				enemies[i].position.y += enemies[i].fallSpeed;
			}

			if (worldheight[positionX][positionZ] == 4){
				enemies[i].position.x -= XMVectorGetX(vecNewDir);
				positionX = worldsizez / 2 + ceil(enemies[i].position.x / worldscale);
				if (worldheight[positionX][positionZ] == 4){
					enemies[i].position.z -= XMVectorGetZ(vecNewDir);
				}
			}

			for (int j = 0; j < numberOfEnemies; j++){
				if (i != j){
					if (pow(enemies[j].position.x - enemies[i].position.x, 2) + pow(enemies[j].position.y - enemies[i].position.y, 2) + pow(enemies[j].position.z - enemies[i].position.z, 2) < 3){
						enemies[i].position.x -= XMVectorGetX(vecNewDir);
						enemies[i].position.z -= XMVectorGetZ(vecNewDir);
					}
				}
			}

			//if reached a waypoint, choose next waypoint
			if (abs(enemies[i].nextWaypoint.position.x - enemies[i].position.x) < 3 && abs(enemies[i].nextWaypoint.position.z - enemies[i].position.z) < 3){
				if (enemies[i].nextWaypoint.numberOfConnections == 2){
					int last = enemies[i].lastWaypoint.id;
					enemies[i].lastWaypoint = enemies[i].nextWaypoint;
					if (waypoints[enemies[i].nextWaypoint.connections[0]].id == last){
						enemies[i].nextWaypoint = waypoints[enemies[i].nextWaypoint.connections[1]];
					}
					else{
						enemies[i].nextWaypoint = waypoints[enemies[i].nextWaypoint.connections[0]];
					}
				}
				else{
					int next;
					do {
						next = rand() % (enemies[i].nextWaypoint.numberOfConnections - 1);
					} while (waypoints[enemies[i].nextWaypoint.connections[next]].id == enemies[i].lastWaypoint.id);
					enemies[i].lastWaypoint = enemies[i].nextWaypoint;
					enemies[i].nextWaypoint = waypoints[enemies[i].lastWaypoint.connections[next]];
				}
			}

			//calculate if enemy is seen
			if (enemies[i].target == -1){
				for (int j = 0; j < numberOfEnemies; j++){
					if (enemies[i].team != enemies[j].team){
						if (atan2(enemies[i].position.z - enemies[j].position.z, enemies[i].position.x - enemies[j].position.x) < 20 && atan2(enemies[i].position.z - enemies[j].position.z, enemies[i].position.x - enemies[j].position.x) > -20){ //see AI on other team
							enemies[i].target = enemies[j].id+1;
						}
					}
				}
				if (enemies[i].team == 1){
					if (atan2(enemies[i].position.z - gamePlayer.position.z, enemies[i].position.x - gamePlayer.position.x) < 20 && atan2(enemies[i].position.z - gamePlayer.position.z, enemies[i].position.x - gamePlayer.position.x) > -20){ //see Player
						enemies[i].target = 0;
					}
				}
			}

			//shoot target 
			if (enemies[i].target > -1){
				if (enemies[i].cooldown <= 0){
					int fired = 0;
					for (int j = 0; j < 100; j++){
						if (bullets[j].active == false && fired == 0){
							fired++;

							bullets[j].position = enemies[j].position;
							bullets[j].g_f_ModelRY = enemies[i].g_f_ModelRY + pi/2;
							bullets[j].g_f_ModelRX = enemies[i].g_f_ModelRX;
							bullets[j].speed = 5;
							bullets[j].owner = enemies[i].id + 1;
							enemies[i].cooldown = 30;
							bullets[j].team = enemies[i].team;
							bullets[j].active = true;
						}
					}
				}
				if (enemies[enemies[i].target].Health < 1){
					enemies[i].target = -1;
				}
				if (enemies[i].target == 0 && gamePlayer.Health < 1){
					enemies[i].target = -1;
				}
			}

			enemies[i].cooldown -= 1 * fElapsedTime;

			//**************************************************************************//
			// Initialize the view matrix.  What you do to the viewer matrix moves the  //
			// viewer, or course.														//
			//																			//
			// The viewer matrix is created every frame here, which looks silly as the	//
			// viewer never moves.  However in general your viewer does move.			//
			//**************************************************************************//



			//******************************************************************//
			// Create the world matrix for the tiger: just a rotate around	    //
			// the Y axis of 45 degrees.  DirectX does all angles in radians,	// //THIS
			// hence the conversion.  And a translate.							//
			//******************************************************************//
			XMMATRIX matTigerTranslate = XMMatrixTranslation(enemies[i].position.x, enemies[i].position.y, enemies[i].position.z);
			//XMMATRIX matDogFix = XMMatrixTranslation(-1, 0, 0);
			XMMATRIX matTigerScale = XMMatrixScaling(10, 10, 10);
			XMMATRIX matTigerWorld = matTigerRotate * matTigerTranslate * matTigerScale;

			XMMATRIX matWorldViewProjection;
			matWorldViewProjection = matTigerWorld * matView * g_MatProjection;



			//******************************************************************//    
			// Update shader variables.  We must update these for every mesh	//
			// that we draw (well, actually we need only update the position	//
			// for each mesh, think hard about this - Nigel						//
			//																	//
			// We pass the parameters to it in a constant buffer.  The buffer	//
			// we define in this module MUST match the constant buffer in the	//
			// shader.															// //THIS
			//																	//
			// It would seem that the constant buffer we pass to the shader must//
			// be global, well defined on the heap anyway.  Not a local variable//
			// it would seem.													//
			//******************************************************************//

			CBMatrices.matWorld = XMMatrixTranspose(matTigerWorld);
			CBMatrices.matWorldViewProj = XMMatrixTranspose(matWorldViewProjection);
			pd3dImmediateContext->UpdateSubresource(g_pcbVSPerObject, 0, NULL, &CBMatrices, 0, 0);
			pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject);

			//******************************************************************//
			// Lighting.  Ambient light and a light direction, above, to the	//
			// left and two paces back, I think.  Then normalise the light		//
			// vector.  It is kind-a-silly doing this every frame unless the	//
			// light moves.														//
			//******************************************************************//


			//**************************************************************************//
			// Render the mesh.															// //THIS
			//**************************************************************************//
			RenderMesh(pd3dImmediateContext, &enemies[i].g_MeshModel, g_pSkyPixelShader);
		}
		else{
			enemies[i].respawn -= 1 * fElapsedTime;
			if (enemies[i].respawn <= 1){
				enemies[i].Health = 100;
				enemies[i].position = waypoints[rand() % numberOfWaypoints].position;
			}
		}


	}

	for (int i = 0; i < 100; i++){
		if (bullets[i].active == true){
			XMMATRIX matTigerRotate = XMMatrixRotationY(bullets[i].g_f_ModelRY);
			XMVECTOR vecNewDir;
			vecNewDir = XMVector3TransformCoord(g_vecTigerInitialDir, matTigerRotate);
			vecNewDir = XMVector3Normalize(vecNewDir);

			XMVECTOR vecArse = vecNewDir *= 3;
			vecNewDir *= bullets[i].speed * fElapsedTime;
			bullets[i].position.x += XMVectorGetX(vecNewDir); // Weird syntax; can't just
			bullets[i].position.y += XMVectorGetY(vecNewDir); // use vector.x.
			bullets[i].position.z += XMVectorGetZ(vecNewDir);

			for (int j = 0; j < numberOfEnemies; j++){
				if (enemies[j].team != bullets[i].team){ //check only enemies enemy
					if (pow(enemies[j].position.x - bullets[i].position.x, 2) + pow(enemies[j].position.y - bullets[i].position.y, 2) + pow(enemies[j].position.z - bullets[i].position.z, 2) < 3){
						enemies[j].Health -= 20;
						if (enemies[j].Health <= 0){
							enemies[j].respawn = 10; //10 frames?
							enemies[bullets[i].owner].target = -1;
							if (enemies[j].team == 0){
								team1Score++;
							}
							else{
								team0Score++;
							}
						}
						enemies[j].target = bullets[i].owner;
						bullets[i].active = false;
					}
				}
			}

			if (gamePlayer.team != bullets[i].team){ //check only enemies enemy
				if (pow(gamePlayer.position.x - bullets[i].position.x, 2) + pow(gamePlayer.position.y - bullets[i].position.y, 2) + pow(gamePlayer.position.z - bullets[i].position.z, 2) < 3){
					gamePlayer.Health -= 20;
					if (gamePlayer.Health <= 0){
						team1Score++;
						gamePlayer.respawn = 10; //10 frames?
						enemies[bullets[i].owner].target = -1;
						bullets[i].active = false;
					}
				}
			}


			XMMATRIX matTigerTranslate = XMMatrixTranslation(bullets[i].position.x, bullets[i].position.y, bullets[i].position.z);
			//XMMATRIX matDogFix = XMMatrixTranslation(-1, 0, 0);
			XMMATRIX matTigerScale = XMMatrixScaling(10, 10, 10);
			XMMATRIX matTigerWorld = matTigerRotate * matTigerTranslate * matTigerScale;

			XMMATRIX matWorldViewProjection;
			matWorldViewProjection = matTigerWorld * matView * g_MatProjection;



			//******************************************************************//    
			// Update shader variables.  We must update these for every mesh	//
			// that we draw (well, actually we need only update the position	//
			// for each mesh, think hard about this - Nigel						//
			//																	//
			// We pass the parameters to it in a constant buffer.  The buffer	//
			// we define in this module MUST match the constant buffer in the	//
			// shader.															// //THIS
			//																	//
			// It would seem that the constant buffer we pass to the shader must//
			// be global, well defined on the heap anyway.  Not a local variable//
			// it would seem.													//
			//******************************************************************//

			CBMatrices.matWorld = XMMatrixTranspose(matTigerWorld);
			CBMatrices.matWorldViewProj = XMMatrixTranspose(matWorldViewProjection);
			pd3dImmediateContext->UpdateSubresource(g_pcbVSPerObject, 0, NULL, &CBMatrices, 0, 0);
			pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject);

			//******************************************************************//
			// Lighting.  Ambient light and a light direction, above, to the	//
			// left and two paces back, I think.  Then normalise the light		//
			// vector.  It is kind-a-silly doing this every frame unless the	//
			// light moves.														//
			//******************************************************************//


			//**************************************************************************//
			// Render the mesh.															// //THIS
			//**************************************************************************//
			if (bullets[i].position.x > 20 || bullets[i].position.x < -20 || bullets[i].position.z > 20 || bullets[i].position.z < -20){
				bullets[i].active = false;
			}

			int positionX = worldsizex / 2 + ceil(enemies[i].position.x / worldscale) - 1;
			int positionZ = worldsizez / 2 + ceil(enemies[i].position.z / worldscale) - 1;

			if (positionX < 0){
				positionX = 0;
			}
			if (positionZ < 0){
				positionZ = 0;
			}
			if (positionX > 19){
				positionX = 19;
			}
			if (positionZ > 19){
				positionZ = 19;
			}

			if (worldheight[positionX][positionZ] == 4){
				bullets[i].active = false;
			}
			RenderMesh(pd3dImmediateContext, &bullets[i].g_MeshModel, g_pSkyPixelShader);
		}
	}

	for (int i = 0; i < numberOfCubes; i++){
		XMMATRIX matTigerRotate = XMMatrixRotationY(cubes[i].rotation);

		XMMATRIX matTigerTranslate = XMMatrixTranslation(cubes[i].position.x, cubes[i].position.y, cubes[i].position.z);
		//XMMATRIX matDogFix = XMMatrixTranslation(-1, 0, 0);
		XMMATRIX matTigerScale = XMMatrixScaling(10, 10, 10);
		if (cubes[i].type == 4){
			matTigerScale = XMMatrixScaling(10, 10, 10);
		}
		XMMATRIX matTigerWorld = matTigerRotate * matTigerTranslate * matTigerScale;

		XMMATRIX matWorldViewProjection;
		matWorldViewProjection = matTigerWorld * matView * g_MatProjection;



		//******************************************************************//    
		// Update shader variables.  We must update these for every mesh	//
		// that we draw (well, actually we need only update the position	//
		// for each mesh, think hard about this - Nigel						//
		//																	//
		// We pass the parameters to it in a constant buffer.  The buffer	//
		// we define in this module MUST match the constant buffer in the	//
		// shader.															// //THIS
		//																	//
		// It would seem that the constant buffer we pass to the shader must//
		// be global, well defined on the heap anyway.  Not a local variable//
		// it would seem.													//
		//******************************************************************//

		CBMatrices.matWorld = XMMatrixTranspose(matTigerWorld);
		CBMatrices.matWorldViewProj = XMMatrixTranspose(matWorldViewProjection);
		pd3dImmediateContext->UpdateSubresource(g_pcbVSPerObject, 0, NULL, &CBMatrices, 0, 0);
		pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject);

		//******************************************************************//
		// Lighting.  Ambient light and a light direction, above, to the	//
		// left and two paces back, I think.  Then normalise the light		//
		// vector.  It is kind-a-silly doing this every frame unless the	//
		// light moves.														//
		//******************************************************************//


		//**************************************************************************//
		// Render the mesh.															// //THIS
		//**************************************************************************//
		RenderMesh(pd3dImmediateContext, &cubes[i].g_MeshModel, g_pSkyPixelShader);



	}

	//**************************************************************************//
	//SkyBox Shit																//
	//**************************************************************************//
	XMMATRIX matSkyTranslate = XMMatrixTranslation(0, 5, 0);
	XMMATRIX matSkyScale = XMMatrixScaling(1, 1, 1);
	XMMATRIX matSkyRotate = XMMatrixRotationY(XMConvertToRadians(-90));

	XMMATRIX matSkyWorld = matSkyRotate * matSkyTranslate * matSkyScale;
	XMMATRIX matSkyWorldViewProjection;

	//matFloorWorld *= matTigerWorld;
	matSkyWorldViewProjection = matSkyWorld * matView * g_MatProjection;

	CBMatrices.matWorld = XMMatrixTranspose(matSkyWorld);
	CBMatrices.matWorldViewProj = XMMatrixTranspose(matSkyWorldViewProjection);

	pd3dImmediateContext->UpdateSubresource(g_pcbVSPerObject, 0, NULL, &CBMatrices, 0, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject);

	RenderWorkingMesh(pd3dImmediateContext, &g_MeshSkyBox, g_pSkyPixelShader);


	//**************************************************************************//
	// Render what is rather grandly called the head up display.				//
	//**************************************************************************//
	DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    g_HUD.OnRender( fElapsedTime );
    g_SampleUI.OnRender( fElapsedTime );
    RenderText();
    DXUT_EndPerfEvent();
}





//**************************************************************************//
// Render a CDXUTSDKMesh, using the Device Context specified.				//
//**************************************************************************//
void RenderMesh (ID3D11DeviceContext *pContext, 
				 CDXUTSDKMesh         *toRender,
	ID3D11PixelShader *PixelShader)
{
	//Get the mesh
    //IA setup
    pContext->IASetInputLayout( g_pVertexLayout11 );
    UINT Strides[1];
    UINT Offsets[1];
    ID3D11Buffer* pVB[1];
    pVB[0] = toRender->GetVB11( 0, 0 );
    Strides[0] = ( UINT )toRender->GetVertexStride( 0, 0 );
    Offsets[0] = 0;
    pContext->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
    pContext->IASetIndexBuffer( toRender->GetIB11( 0 ), toRender->GetIBFormat11( 0 ), 0 );

    // Set the shaders
    pContext->VSSetShader( g_pVertexShader, NULL, 0 );
    pContext->PSSetShader( PixelShader,  NULL, 0 ); //this
    
	for( UINT subset = 0; subset < toRender->GetNumSubsets( 0 ); ++subset )
    {
		//Render
		SDKMESH_SUBSET* pSubset = NULL;
		D3D11_PRIMITIVE_TOPOLOGY PrimType;
        
		// Get the subset
        pSubset = toRender->GetSubset( 0, subset );

        PrimType = CDXUTSDKMesh::GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
        pContext->IASetPrimitiveTopology( PrimType );

		//**************************************************************************//
		// At the moment we load a texture into video memory every frame, which is	//
		// HORRIBLE, we need to create more Texture2D variables.					//
		//**************************************************************************//
        ID3D11ShaderResourceView* pDiffuseRV = toRender->GetMaterial( pSubset->MaterialID )->pDiffuseRV11;
        //pContext->PSSetShaderResources( 0, 1, &pDiffuseRV );

        pContext->DrawIndexed( ( UINT )pSubset->IndexCount, 0, ( UINT )pSubset->VertexStart );
    }

}

void RenderWorkingMesh(ID3D11DeviceContext *pContext,
	CDXUTSDKMesh         *toRender,
	ID3D11PixelShader *PixelShader)
{
	//Get the mesh
	//IA setup
	pContext->IASetInputLayout(g_pVertexLayout11);
	UINT Strides[1];
	UINT Offsets[1];
	ID3D11Buffer* pVB[1];
	pVB[0] = toRender->GetVB11(0, 0);
	Strides[0] = (UINT)toRender->GetVertexStride(0, 0);
	Offsets[0] = 0;
	pContext->IASetVertexBuffers(0, 1, pVB, Strides, Offsets);
	pContext->IASetIndexBuffer(toRender->GetIB11(0), toRender->GetIBFormat11(0), 0);

	// Set the shaders
	pContext->VSSetShader(g_pVertexShader, NULL, 0);
	pContext->PSSetShader(PixelShader, NULL, 0); //this

	for (UINT subset = 0; subset < toRender->GetNumSubsets(0); ++subset)
	{
		//Render
		SDKMESH_SUBSET* pSubset = NULL;
		D3D11_PRIMITIVE_TOPOLOGY PrimType;

		// Get the subset
		pSubset = toRender->GetSubset(0, subset);

		PrimType = CDXUTSDKMesh::GetPrimitiveType11((SDKMESH_PRIMITIVE_TYPE)pSubset->PrimitiveType);
		pContext->IASetPrimitiveTopology(PrimType);

		//**************************************************************************//
		// At the moment we load a texture into video memory every frame, which is	//
		// HORRIBLE, we need to create more Texture2D variables.					//
		//**************************************************************************//
		ID3D11ShaderResourceView* pDiffuseRV = toRender->GetMaterial(pSubset->MaterialID)->pDiffuseRV11;
		pContext->PSSetShaderResources( 0, 1, &pDiffuseRV );

		pContext->DrawIndexed((UINT)pSubset->IndexCount, 0, (UINT)pSubset->VertexStart);
	}

}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	ReleaseCapture;
    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_D3DSettingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_DELETE( g_pTxtHelper );

	g_MeshSkyBox.Destroy();
	g_MeshProjectile.Destroy();
	g_MeshEnemy.Destroy();

	g_MeshCornerIn.Destroy();
	g_MeshCornerOut.Destroy();
	g_MeshCube.Destroy();
	g_MeshRamp.Destroy();

	for (int i = 0; i < 100; i++){
		bullets[i].g_MeshModel.Destroy();
	}

	for (int i = 0; i < numberOfEnemies; i++){
		enemies[i].g_MeshModel.Destroy();
	}
	gamePlayer.g_MeshModel.Destroy();
	for (int i = 0; i < numberOfProjectiles; i++){
		projectiles[i].g_MeshModel.Destroy();
	}

	for (int i = 0; i < numberOfCubes; i++){
		cubes[i].g_MeshModel.Destroy();
	}
                
    SAFE_RELEASE( g_pVertexLayout11 );
    SAFE_RELEASE( g_pVertexBuffer );
    SAFE_RELEASE( g_pIndexBuffer );
    SAFE_RELEASE( g_pVertexShader );
    SAFE_RELEASE( g_pPixelShader );
    SAFE_RELEASE( g_pSamLinear );
	SAFE_RELEASE(g_pSkyPixelShader);

    SAFE_RELEASE( g_pcbVSPerObject );
    SAFE_RELEASE( g_pcbPSPerObject );
    SAFE_RELEASE( g_pcbPSPerFrame );
}




//**************************************************************************//
// Convert an old chracter (char *) string to a WCHAR * string.  There must //
// be something built into Visual Studio to do this for me, but I can't		//
// find it - Nigel.															//
//**************************************************************************//
void charStrToWideChar(WCHAR *dest, char *source)
{
	int length = strlen(source);
	for (int i = 0; i <= length; i++)
		dest[i] = (WCHAR) source[i];
}
