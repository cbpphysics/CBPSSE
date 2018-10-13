#include "hook.h"

typedef HRESULT(__stdcall *D3D11PresentHook) (IDXGISwapChain* This, UINT SyncInterval, UINT Flags);
D3D11PresentHook oPresent = NULL;

//Present Function
HRESULT __stdcall hookD3D11Present(IDXGISwapChain* This, UINT SyncInterval, UINT Flags)
{
	//YourPresent
	return oPresent(This, SyncInterval, Flags);
}

//HookSwapChain with the specified ptr.
bool __stdcall DX11Manager::HookPresent(IDXGISwapChain* swapChain)
{
	if (!swapChain)
		return false;

	this->pSwapChain = swapChain; //Save swapChain ptr

	VMTSwapChainHook = new CVMTHook((DWORD_PTR**)swapChain);
	oPresent = (D3D11PresentHook)VMTSwapChainHook->dwHookMethod((DWORD_PTR)hookD3D11Present, 8);

	//Get Device PTR
	if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device), (LPVOID*)&this->pDevice)))
	{
		FILE_LOG(logERROR) << "Failed to Get Device Ptr";
		return false;
	}

	if (!pDevice)
	{
		FILE_LOG(logERROR) << "pDevice invalid";
		return false;
	}

	//Get Context PTR
	this->pDevice->GetImmediateContext(&this->pContext);

	if (!pContext)
	{
		FILE_LOG(logERROR) << "Failed to Get DeviceContext Ptr";
		return false;
	}

	ReleaseCreatedDevice();

	this->bInitialized = true;

	return true;
}

//Create temporary DeviceAndSwapChain
bool __stdcall DX11Manager::CreateDeviceAndSwapChain()
{
	HWND hWnd = GetForegroundWindow();
	if (hWnd == nullptr)
		return false;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = (GetWindowLong(hWnd, GWL_STYLE) & WS_POPUP) != 0 ? FALSE : TRUE;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1
		, D3D11_SDK_VERSION, &swapChainDesc, &tempSwapChain, &tempDevice, NULL, &tempContext)))
	{
		FILE_LOG(logERROR) << "Failed to CreateDeviceAndSwapChain";
		return false;
	}

	pSwapChainVtable = (DWORD_PTR*)tempSwapChain;
	pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

	return (pSwapChainVtable);
}


bool DX11Manager::FindDynamicallySwapChain()
{
	if (CreateDeviceAndSwapChain() == false)
	{
		return false;
	}

#ifdef _AMD64_
#define _PTR_MAX_VALUE 0x7FFFFFFEFFFF
	MEMORY_BASIC_INFORMATION64 mbi = { 0 };
#else
#define _PTR_MAX_VALUE 0xFFE00000
	MEMORY_BASIC_INFORMATION32 mbi = { 0 };
#endif

	for (DWORD_PTR memptr = 0x10000; memptr < _PTR_MAX_VALUE; memptr = mbi.BaseAddress + mbi.RegionSize) //For x64 -> 0x10000 ->  0x7FFFFFFEFFFF
	{
		if (!VirtualQuery(reinterpret_cast<LPCVOID>(memptr), reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&mbi), sizeof(MEMORY_BASIC_INFORMATION))) //Iterate memory by using VirtualQuery
			continue;

		if (mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS || mbi.Protect & PAGE_GUARD) //Filter Regions
			continue;

		DWORD_PTR len = mbi.BaseAddress + mbi.RegionSize;	 //Do once

		for (DWORD_PTR current = mbi.BaseAddress; current < len; ++current)
		{

			__try
			{
				dwVTableAddress = *(DWORD_PTR*)current;
			}
			__except (1)
			{
				continue;
			}


			if (dwVTableAddress == (DWORD_PTR)pSwapChainVtable)
			{
				if (current == (DWORD_PTR)tempSwapChain) continue;  //we don't want to hook the tempSwapChain	
				return HookPresent((IDXGISwapChain*)current);	//If found we hook Present in swapChain		
			}
		}
	}

	return true;
}

void DX11Manager::ReleaseCreatedDevice()
{
	SAFE_RELEASE(this->tempContext);
	SAFE_RELEASE(this->tempDevice);
	SAFE_RELEASE(this->tempSwapChain);
}
