#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#include <dshow.h>
#include <d3d9.h>
#include <d3dx9tex.h>
#include <dinput.h>
#include "Camera.h"
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

#define WM_GRAPHNOTIFY  WM_APP + 1 
LPDIRECT3D9             D3D           = NULL; 
LPDIRECT3DDEVICE9       d3dDevice     = NULL; 
LPDIRECTINPUT8			g_pDin;							
LPDIRECTINPUTDEVICE8	g_pDinKeyboard;					
BYTE					g_Keystate[256];				
LPDIRECTINPUTDEVICE8	g_pDinmouse;					
DIMOUSESTATE			g_pMousestate;	
HRESULT InitD3D(HWND hWnd);								
HRESULT InitDInput(HINSTANCE hInstance, HWND hWnd);		
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ); 
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT ); 
CXCamera *camera = NULL;								
IGraphBuilder *pGraph = NULL;							
IMediaControl *pControl = NULL;							
IMediaEventEx *pEvent = NULL;							

HWND hWnd;
HDC hdc;
LPD3DXMESH              MeshKnight          = NULL;   
D3DMATERIAL9*           KnightMeshMaterials = NULL;   
LPDIRECT3DTEXTURE9*     KnightTextures  = NULL;       
DWORD                   dwKnightMaterials = 0L;        
D3DXVECTOR3				vectorTranslatie(0.0f, -7.5f, 3.0f); 
D3DXMATRIXA16			matProj;					
D3DXMATRIX              KnightMatrix;				

LPDIRECT3DTEXTURE9		TextureBox[6];					
LPDIRECT3DVERTEXBUFFER9 pVB = NULL;					


 float skybox = 5;																			
 float fUnghi = 0;
 struct CUSTOMVERTEX
 {
	 FLOAT x, y, z; 
	 DWORD color;  						
	 float tu, tv;
	 float xx, yy, zz; 
 };


#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1) 
HRESULT InitD3D( HWND hWnd ) 
{
    if( NULL == ( D3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return E_FAIL;

    D3DPRESENT_PARAMETERS d3dpp;    
    ZeroMemory( &d3dpp, sizeof(d3dpp) ); 
    d3dpp.Windowed = TRUE; 
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; 
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;								
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	
    if( FAILED( D3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &d3dDevice ) ) )
    {
		if( FAILED( D3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &d3dDevice ) ) )
			return E_FAIL;
    }
    d3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );

    return S_OK;
}


 void loadTexture(void)
 {
	 
	 D3DXCreateTextureFromFile(d3dDevice, "Skybox\\Mountain_Back.jpg", &TextureBox[0]);
	 D3DXCreateTextureFromFile(d3dDevice, "Skybox\\Mountain_Front.jpg", &TextureBox[1]);
	 D3DXCreateTextureFromFile(d3dDevice, "Skybox\\Mountain_Top.jpg", &TextureBox[2]);
	 D3DXCreateTextureFromFile(d3dDevice, "Skybox\\siege_dn.tga", &TextureBox[3]);
	 D3DXCreateTextureFromFile(d3dDevice, "Skybox\\Mountain_Right.jpg", &TextureBox[4]);
	 D3DXCreateTextureFromFile(d3dDevice, "Skybox\\Mountain_Left.jpg", &TextureBox[5]);
	 d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	 d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
 }


HRESULT InitDirectShow(HWND hWnd)
{
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, 
    		CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	hr = pGraph->QueryInterface(IID_IMediaEventEx, (void **)&pEvent);
	hr = pGraph->RenderFile(L"medieval.MP3", NULL);
	pEvent->SetNotifyWindow((OAHWND)hWnd, WM_GRAPHNOTIFY, 0);
	return S_OK;
}

void HandleGraphEvent()
{
    long evCode;
    LONG_PTR param1, param2;

    while (SUCCEEDED(pEvent->GetEvent(&evCode, &param1, &param2, 0)))
    {
        pEvent->FreeEventParams(evCode, param1, param2);
        switch (evCode)
        {
        case EC_COMPLETE:  
			pControl->Stop();
			break;
        case EC_USERABORT: 
        case EC_ERRORABORT:
            PostQuitMessage(0);
            return;
        }
    } 
}

HRESULT InitGeometry()
{
    LPD3DXBUFFER pKnightBuffer = NULL; 
	LPD3DXBUFFER pD3DXMtrlBuffer;
	loadTexture();
	CUSTOMVERTEX g_Vertices[] =
	{
		{ -skybox / 2, skybox / 2, -skybox / 2, 0xffffffff, 0.0f,0.0f, 0, 0, -1 },
	{ skybox / 2, skybox / 2, -skybox / 2, 0xffffffff, 1.0f,0.0f, 0, 0, -1 },
	{ -skybox / 2, -skybox / 2, -skybox / 2, 0xffffffff, 0.0f,1.0f, 0, 0, -1 },
	{ skybox / 2, -skybox / 2, -skybox / 2, 0xffffffff, 1.0f,1.0f, 0, 0, -1 },


	{ -skybox / 2, skybox / 2, skybox / 2, 0xffffffff, 1.0f,0.0f, 0, 0, 1 },
	{ -skybox / 2, -skybox / 2, skybox / 2, 0xffffffff, 1.0f,1.0f, 0, 0, 1 },
	{ skybox / 2, skybox / 2, skybox / 2, 0xffffffff, 0.0f,0.0f, 0, 0, 1 },
	{ skybox / 2, -skybox / 2, skybox / 2, 0xffffffff, 0.0f,1.0f, 0, 0, 1 },


	{ -skybox / 2, skybox / 2, skybox / 2, 0xffffffff,  0.0f,0.0f, 0, 1, 0 },
	{ skybox / 2, skybox / 2, skybox / 2, 0xffffffff, 1.0f,0.0f, 0, 1, 0 },
	{ -skybox / 2, skybox / 2, -skybox / 2, 0xffffffff, 0.0f,1.0f, 0, 1, 0 },
	{ skybox / 2, skybox / 2, -skybox / 2, 0xffffffff, 1.0f,1.0f, 0, 1, 0 },


	{ -skybox / 2, -skybox / 2, skybox / 2, 0xffffffff, 0.0f,0.0f, 0, -1, 0 },
	{ -skybox / 2, -skybox / 2, -skybox / 2, 0xffffffff, 1.0f,0.0f, 0, -1, 0 },
	{ skybox / 2, -skybox / 2, skybox / 2, 0xffffffff, 0.0f,1.0f, 0, -1, 0 },
	{ skybox / 2, -skybox / 2, -skybox / 2, 0xffffffff, 1.0f,1.0f, 0, -1, 0 },


	{ skybox / 2, skybox / 2, -skybox / 2, 0xffffffff, 0.0f,0.0f, 1, 0, 0 },
	{ skybox / 2, skybox / 2, skybox / 2, 0xffffffff, 1.0f,0.0f, 1, 0, 0 },
	{ skybox / 2, -skybox / 2, -skybox / 2, 0xffffffff, 0.0f,1.0f, 1, 0, 0 },
	{ skybox / 2, -skybox / 2, skybox / 2, 0xffffffff, 1.0f,1.0f, 1, 0, 0 },


	{ -skybox / 2, skybox / 2, -skybox / 2, 0xffffffff, 1.0f,0.0f, -1, 0, 0 },
	{ -skybox / 2, -skybox / 2, -skybox / 2, 0xffffffff, 1.0f,1.0f, -1, 0, 0 },
	{ -skybox / 2, skybox / 2, skybox / 2, 0xffffffff, 0.0f,0.0f, -1, 0, 0 },
	{ -skybox / 2, -skybox / 2, skybox / 2, 0xffffffff, 0.0f,1.0f, -1, 0, 0 },
	};

	if (FAILED(d3dDevice->CreateVertexBuffer(24 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &pVB, NULL)))
	{
		return E_FAIL;
	}

	VOID* pVertices;  
	if (FAILED(pVB->Lock(0, sizeof(g_Vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	memcpy(pVertices, g_Vertices, sizeof(g_Vertices));
	pVB->Unlock();
	if( FAILED( D3DXLoadMeshFromX( "Knight.x", D3DXMESH_SYSTEMMEM, 
                                   d3dDevice, NULL, 
                                   &pKnightBuffer, NULL, &dwKnightMaterials, 
                                   &MeshKnight ) ) )
	{
		MessageBox(NULL, "Could not find Knight.x", "Meshes.exe", MB_OK);
        return E_FAIL;
    }
    D3DXMATERIAL* d3dxMaterialsKnight = (D3DXMATERIAL*)pKnightBuffer->GetBufferPointer();

    KnightMeshMaterials = new D3DMATERIAL9[dwKnightMaterials];
    KnightTextures  = new LPDIRECT3DTEXTURE9[dwKnightMaterials];

    for( DWORD i=0; i<dwKnightMaterials; i++ )
    {
        KnightMeshMaterials[i] = d3dxMaterialsKnight[i].MatD3D;
        KnightMeshMaterials[i].Ambient = KnightMeshMaterials[i].Diffuse;
        KnightTextures[i] = NULL;
        if( d3dxMaterialsKnight[i].pTextureFilename != NULL && 
            lstrlen(d3dxMaterialsKnight[i].pTextureFilename) > 0 )
        {
            if( FAILED( D3DXCreateTextureFromFile( d3dDevice, 
                                                d3dxMaterialsKnight[i].pTextureFilename, 
                                                &KnightTextures[i] ) ) )
            {
				MessageBox(NULL, "Could not find texture map", "Meshes.exe", MB_OK);
            }
        }
    }
    pKnightBuffer->Release();

	camera = new CXCamera(d3dDevice); 
	D3DXVECTOR3 vEyePt( 0.0f, -6.0f, -3.0f );
    D3DXVECTOR3 vLookatPt( 0.0f, -6.0f, 0.0f );
    D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
	camera->LookAtPos(&vEyePt,&vLookatPt,&vUpVec);

    return S_OK;
}
HRESULT InitDInput(HINSTANCE hInstance, HWND hWnd)
{
    
    DirectInput8Create(hInstance,DIRECTINPUT_VERSION,IID_IDirectInput8, (void**)&g_pDin, NULL);
    g_pDin->CreateDevice(GUID_SysKeyboard, &g_pDinKeyboard, NULL);               
	g_pDin->CreateDevice(GUID_SysMouse, &g_pDinmouse, NULL);
    g_pDinKeyboard->SetDataFormat(&c_dfDIKeyboard);
	g_pDinmouse->SetDataFormat(&c_dfDIMouse);
    g_pDinKeyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	g_pDinmouse->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	return S_OK;
}

VOID DetectInput()
{
    g_pDinKeyboard->Acquire();
	g_pDinmouse->Acquire();
    g_pDinKeyboard->GetDeviceState(256, (LPVOID)g_Keystate);
	g_pDinmouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&g_pMousestate);


}



VOID Cleanup()
{
    if( KnightMeshMaterials != NULL ) 
        delete[] KnightMeshMaterials;

    if( KnightTextures )
    {
        for( DWORD i = 0; i < dwKnightMaterials; i++ )
        {
            if( KnightTextures[i] )
                KnightTextures[i]->Release();
        }
        delete[] KnightTextures;
    }
    if( MeshKnight != NULL )
        MeshKnight->Release();

	if(pGraph)
		pGraph->Release();

	if(pControl)
		pControl->Release();

	if(pEvent)
		pEvent->Release();
    
    if( d3dDevice != NULL )
        d3dDevice->Release();

    if( D3D != NULL )
        D3D->Release();
}

VOID CleanDInput()
{
	if(g_pDin){
		if(g_pDinKeyboard){
			g_pDinKeyboard->Unacquire();  
			g_pDinKeyboard->Release();   
			g_pDinKeyboard = NULL;    
		}
		if(g_pDinmouse){
			g_pDinmouse->Unacquire();    
			g_pDinmouse->Release();    
			g_pDinmouse = NULL;    
		}
		g_pDin->Release();
		g_pDin = NULL;
	}
}

VOID SetupMatrices()
{
	camera->Update();
	D3DXMatrixIdentity(&KnightMatrix); 
	D3DXMatrixTranslation(&KnightMatrix, vectorTranslatie.x, vectorTranslatie.y, vectorTranslatie.z);
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 100.0f );
    d3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}

VOID Render()
{

    d3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 
                         D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );
    
    if( SUCCEEDED( d3dDevice->BeginScene() ) )
    {
		D3DXMATRIXA16 matScaleSkybox;
		D3DXMATRIXA16 matTranslateSkybox;
		D3DXMATRIXA16 result;
		float val = 3;
		D3DXMatrixScaling(&matScaleSkybox, val,val,val);
		D3DXMatrixTranslation(&matTranslateSkybox, 0.0, 0.0, 0.0);
		result = matTranslateSkybox * matScaleSkybox;
		d3dDevice->SetTransform(D3DTS_WORLD, &result);
		d3dDevice->SetStreamSource(0, pVB, 0, sizeof(CUSTOMVERTEX)); 
		d3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX); 
		d3dDevice->SetTexture(0, TextureBox[0]);
		d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		d3dDevice->SetTexture(0, TextureBox[1]);
		d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4, 2);
		d3dDevice->SetTexture(0, TextureBox[2]);
		d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 8, 2);
		d3dDevice->SetTexture(0, TextureBox[3]);
		d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 12, 2);
		d3dDevice->SetTexture(0, TextureBox[4]);
		d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 16, 2);
		d3dDevice->SetTexture(0, TextureBox[5]);
		d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 20, 2);
        SetupMatrices();
		D3DXMATRIX rotationZ, rotationY; 
		D3DXMatrixRotationAxis(&rotationZ, &D3DXVECTOR3(1.0f, 0.0f, 0.0f), D3DXToRadian(-90)); 
		D3DXMatrixMultiply(&KnightMatrix, &rotationZ, &KnightMatrix);
		D3DXMatrixRotationAxis(&rotationY, &D3DXVECTOR3(0.0f, 0.0f, 1.0f), D3DXToRadian(15.0f * fUnghi )); 
		D3DXMatrixMultiply(&KnightMatrix, &rotationY, &KnightMatrix);
		d3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

        for( DWORD i=0; i<dwKnightMaterials; i++ )
        {
			d3dDevice->SetTransform(D3DTS_WORLD, &KnightMatrix);
            d3dDevice->SetMaterial( &KnightMeshMaterials[i] );
            d3dDevice->SetTexture( 0, KnightTextures[i] );
            MeshKnight->DrawSubset( i );
        }
        d3dDevice->EndScene();
    }
    d3dDevice->Present( NULL, NULL, NULL, NULL );
}

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            Cleanup();
			CleanDInput();
            PostQuitMessage( 0 );
            return 0;

		case WM_GRAPHNOTIFY:
			HandleGraphEvent();
			return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "D3D Tutorial", NULL };
    RegisterClassEx( &wc );

    HWND hWnd = CreateWindow( "D3D Tutorial", "D3D Tutorial 06: Meshes", 
                              WS_OVERLAPPEDWINDOW, 5, 5, 1500, 800,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );
	HRESULT hr = CoInitialize(NULL);

	hdc = GetDC(hWnd);

    if( SUCCEEDED( InitD3D( hWnd ) ) )
    { 
		if(FAILED(InitDirectShow(hWnd)))
			return 0;
        if( SUCCEEDED( InitGeometry() ) )
        {
			InitDInput(hInst, hWnd);
            ShowWindow( hWnd, SW_SHOWDEFAULT );
            UpdateWindow( hWnd );
            MSG msg; 
            ZeroMemory( &msg, sizeof(msg) );
            while( msg.message!=WM_QUIT )
            {
                if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
                else{
						DetectInput();
						Render();
						if(g_Keystate[DIK_ESCAPE] & 0x80){
							PostMessage(hWnd, WM_DESTROY, 0, 0);
						}
						if(g_Keystate[DIK_UP] & 0x80){
							if(vectorTranslatie.z > -(skybox +2.1))
							vectorTranslatie.z -= 0.1; 
						}
						if(g_Keystate[DIK_DOWN] & 0x80){
							if(vectorTranslatie.z < (skybox +2.1))
							vectorTranslatie.z += 0.1; 
						}
						if(g_Keystate[DIK_LEFT] & 0x80){
							if(vectorTranslatie.x > -(skybox +2))
							vectorTranslatie.x -= 0.1; 
						}
						if(g_Keystate[DIK_RIGHT] & 0x80){
							if(vectorTranslatie.x < (skybox +2))
							vectorTranslatie.x += 0.1; 
						}
						if(g_Keystate[DIK_A] & 0x80){
							camera->MoveRight(-0.1);
							camera->Update();
						}
						if(g_Keystate[DIK_S] & 0x80){
							camera->MoveForward(-0.1);
							camera->Update();
						}
						if(g_Keystate[DIK_D] & 0x80){
							camera->MoveRight(0.1);
							camera->Update();
						}
						if(g_Keystate[DIK_W] & 0x80){
							camera->MoveForward(0.1);
							camera->Update();
						}
						if(g_Keystate[DIK_Q] & 0x80){
							camera->RotateRight(-0.1);
							camera->Update();
						}
						if(g_Keystate[DIK_E] & 0x80){
							camera->RotateRight(0.1);
							camera->Update();
						}
						if(g_Keystate[DIK_Z] & 0x80){
							fUnghi+=1;
						}
						if(g_Keystate[DIK_X] & 0x80){
							fUnghi-=1;	
						}
						if(g_Keystate[DIK_T] & 0x80){
							pControl->Stop();
						}
						if(g_Keystate[DIK_SPACE] & 0x80){
							pControl->Run(); 
						}
						if(g_pMousestate.lX != 0)
						{
							camera->RotateRight(g_pMousestate.lX *0.001 );
							camera->Update();
						}
						if(g_pMousestate.lY != 0)
						{
							camera->RotateDown(g_pMousestate.lY *0.001 );
							camera->Update();
						}
				}
            }
        }
    }
	CoUninitialize();

    UnregisterClass( "D3D Tutorial", wc.hInstance );
    return 0;
}



