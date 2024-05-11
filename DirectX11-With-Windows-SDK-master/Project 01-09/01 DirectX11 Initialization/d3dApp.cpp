#include "d3dApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include <sstream>

namespace
{
	//设置该全局变量是因为在窗口创建的时候需要绑定一个回调函数，受到回调函数指针类型的限制，
	// 我们不可以绑定d3dApp::MainWndProc的成员方法，所以还需要实现一个全局函数用于回调函数的绑定：
	// 
	// This is just used to forward Windows messages from a global window
	// procedure to our member function window procedure because we cannot
	// assign a member function to WNDCLASS::lpfnWndProc.
	D3DApp* g_pd3dApp = nullptr;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//全局函数用于回调函数的绑定：
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before m_hMainWnd is valid.
	return g_pd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance)
	: m_hAppInst(hInstance),
	m_MainWndCaption(L"DirectX11 Initialization"),
	m_ClientWidth(800),
	m_ClientHeight(600),
	m_hMainWnd(nullptr),
	m_AppPaused(false),
	m_Minimized(false),
	m_Maximized(false),
	m_Resizing(false),
	m_Enable4xMsaa(true),
	m_4xMsaaQuality(0),
	m_pd3dDevice(nullptr),
	m_pd3dImmediateContext(nullptr),
	m_pSwapChain(nullptr),
	m_pDepthStencilBuffer(nullptr),
	m_pRenderTargetView(nullptr),
	m_pDepthStencilView(nullptr)
{
	ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));


	// 让一个全局指针获取这个类，这样我们就可以在Windows消息处理的回调函数
	// 让这个类调用内部的回调函数了
	g_pd3dApp = this;
}

D3DApp::~D3DApp()
{
	//因为之前我们用的是智能指针，所以D3DApp的析构函数十分简单，
	// 只需要通过ID3D11DeviceContext::ClearState方法来恢复D3D设备上下文到默认状态，卸下所有绑定的资源即可。
	// 剩下的事情就交给COM智能指针完成：
	// 恢复所有默认设定
	if (m_pd3dImmediateContext)
		m_pd3dImmediateContext->ClearState();
}

HINSTANCE D3DApp::AppInst()const
{
	return m_hAppInst;
}

HWND D3DApp::MainWnd()const
{
	return m_hMainWnd;
}

float D3DApp::AspectRatio()const
{
	return static_cast<float>(m_ClientWidth) / m_ClientHeight;
}

int D3DApp::Run()
{
	MSG msg = { 0 };

	m_Timer.Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_Timer.Tick();

			if (!m_AppPaused)
			{
				CalculateFrameStats();
				UpdateScene(m_Timer.DeltaTime());
				DrawScene();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

//初始化阶段首先需要创建D3D设备和D3D设备上下文
bool D3DApp::Init()
{
	if (!InitMainWindow())
		return false;

	if (!InitDirect3D())
		return false;

	return true;
}

//如果涉及到窗口大小变化的情况，那么前面提到的后备缓冲区、深度/模板缓冲区、视口都需要重新调整大小。
//深度模板缓冲区和视口都可以直接重新创建一份来进行替换。通过IDXGISwapChain::ResizeBuffers来重新调整后备缓冲区的分辨率
void D3DApp::OnResize()
{
	assert(m_pd3dImmediateContext);
	assert(m_pd3dDevice);
	assert(m_pSwapChain);

	if (m_pd3dDevice1 != nullptr)
	{
		assert(m_pd3dImmediateContext1);
		assert(m_pd3dDevice1);
		assert(m_pSwapChain1);
	}

	// 释放渲染管线输出用到的相关资源
	m_pRenderTargetView.Reset();
	m_pDepthStencilView.Reset();
	m_pDepthStencilBuffer.Reset();

	// 重设交换链并且重新创建渲染目标视图
	ComPtr<ID3D11Texture2D> backBuffer;
	HR(m_pSwapChain->ResizeBuffers(1, m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	HR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf())));
	HR(m_pd3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_pRenderTargetView.GetAddressOf()));
	
	// 设置调试对象名
	D3D11SetDebugObjectName(backBuffer.Get(), "BackBuffer[0]");

	backBuffer.Reset();


	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = m_ClientWidth;
	depthStencilDesc.Height = m_ClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 要使用 4X MSAA? --需要给交换链设置MASS参数
	if (m_Enable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}
	


	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	// 创建深度缓冲区以及深度模板视图
	HR(m_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, m_pDepthStencilBuffer.GetAddressOf()));
	HR(m_pd3dDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, m_pDepthStencilView.GetAddressOf()));


	// 将渲染目标视图和深度/模板缓冲区结合到管线
	m_pd3dImmediateContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

	// 设置视口变换
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
	m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;
	//设置1个或多个视口：
	m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);

	// 设置调试对象名
	D3D11SetDebugObjectName(m_pDepthStencilBuffer.Get(), "DepthStencilBuffer");
	D3D11SetDebugObjectName(m_pDepthStencilView.Get(), "DepthStencilView");
	D3D11SetDebugObjectName(m_pRenderTargetView.Get(), "BackBufferRTV[0]");

}


LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_AppPaused = true;
			m_Timer.Stop();
		}
		else
		{
			m_AppPaused = false;
			m_Timer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		m_ClientWidth = LOWORD(lParam);
		m_ClientHeight = HIWORD(lParam);
		if (m_pd3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_AppPaused = true;
				m_Minimized = true;
				m_Maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_AppPaused = false;
				m_Minimized = false;
				m_Maximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (m_Minimized)
				{
					m_AppPaused = false;
					m_Minimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (m_Maximized)
				{
					m_AppPaused = false;
					m_Maximized = false;
					OnResize();
				}
				else if (m_Resizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or m_pSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		m_AppPaused = true;
		m_Resizing = true;
		m_Timer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		m_AppPaused = false;
		m_Resizing = false;
		m_Timer.Start();
		OnResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		return 0;
	case WM_MOUSEMOVE:
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


bool D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"D3DWndClassName";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, m_ClientWidth, m_ClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	m_hMainWnd = CreateWindow(L"D3DWndClassName", m_MainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_hAppInst, 0);
	if (!m_hMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);

	return true;
}

//D3D设备（ID3D11Device）通常代表一个显示适配器（即显卡），它最主要的功能是用于创建各种所需资源，
//最常用的资源有：资源类（ID3D11Resource, 包含纹理和缓冲区），视图类以及着色器。
//此外，D3D设备还能够用于检测系统环境对功能的支持情况。

//D3D设备上下文(ID3D11DeviceContext)可以看做是一个渲染管线。通常我们在创建D3D设备的同时也会附赠一个立即设备上下文(Immediate Context)。
// 一个D3D设备仅对应一个D3D立即设备上下文，并且只要我们拥有其中一方，
// 就能通过各自的方法获取另一方（即ID3D11Device::GetImmediateContext和ID3D11DeviceContext::GetDevice）。
// 渲染管线主要负责渲染和计算工作，它需要绑定来自与它关联的D3D设备所创建的各种资源、视图和着色器才能正常运转，
// 除此之外，它还能够负责对资源的直接读写操作。
//如果你的系统支持Direct3D 11.1的话，则对应的接口类为：ID3D11Device1、ID3D11DeviceContext1，
//它们分别继承自上面的两个接口类，区别在于额外提供了少数新的接口，并且接口方法的实现可能会有所区别。
bool D3DApp::InitDirect3D()
{
	HRESULT hr = S_OK;

	// 创建D3D设备 和 D3D设备上下文
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// 驱动类型数组
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP, //通常虚拟机环境下是不支持硬件驱动的，所以用软件
		D3D_DRIVER_TYPE_REFERENCE, 
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes); 

	// 特性等级数组
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_FEATURE_LEVEL featureLevel;
	D3D_DRIVER_TYPE d3dDriverType;
	//轮询驱动类型
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		d3dDriverType = driverTypes[driverTypeIndex];

		//HRESULT WINAPI D3D11CreateDevice(
		//	IDXGIAdapter * pAdapter,             // [In_Opt](显示适配器)，对显卡设备的一层封装，
		// 传nullptr使用的是默认的显卡适配器IDXGIAdapter（对于双显卡的笔记本大概率使用的是集成显卡
		// 通过该参数，我们可以指定需要使用哪个显卡设备。通常该参数我们设为nullptr，
		// 这样就可以交由上层驱动来帮我们决定使用哪个显卡，或者在NVIDIA控制面板来设置当前程序要使用哪个显卡。
		// 如果想要在应用层(我们编写的引擎层)决定，使用IDXGIFactory::EnumAdapters方法可以枚举当前可用的显示卡设备。
		// 在最底下的练习题你将学会如何指定显示卡设备来创建Direct3D 11.x设备。
		//	D3D_DRIVER_TYPE DriverType,         // [In]驱动类型 大多数情况支持D3D_DRIVER_TYPE_HARDWARE，享受硬件加速
		//	HMODULE Software,                   // [In_Opt]若上面为D3D_DRIVER_TYPE_SOFTWARE则这里需要提供程序模块
		//	UINT Flags,                         // [In]使用D3D11_CREATE_DEVICE_FLAG枚举类型 用于指定是否调试
		//	D3D_FEATURE_LEVEL * pFeatureLevels,  // [In_Opt]若为nullptr则为默认特性等级，否则需要提供特性等级数组，
		// 特性等级和D3D设备的版本并不是互相对应的，而是跟当前的显卡是否支持有关
		// 
		//	UINT FeatureLevels,                 // [In]特性等级数组的元素数目
		//	UINT SDKVersion,                    // [In]SDK版本，默认D3D11_SDK_VERSION
		//	ID3D11Device * *ppDevice,            // [Out_Opt]输出D3D设备
		//	D3D_FEATURE_LEVEL * pFeatureLevel,   // [Out_Opt]输出当前应用D3D特性等级
		//	ID3D11DeviceContext * *ppImmediateContext); //[Out_Opt]输出D3D设备上下文
		hr = D3D11CreateDevice(nullptr, d3dDriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, m_pd3dDevice.GetAddressOf(), &featureLevel, m_pd3dImmediateContext.GetAddressOf());

		//如果系统不支持Direct3D 11.1的API，D3D11CreateDevice会立即停止特性数组的轮询并返回E_INVALIDARG，然后从11.0开始重新轮询
		if (hr == E_INVALIDARG)
		{
			// Direct3D 11.0 的API不承认D3D_FEATURE_LEVEL_11_1，所以我们需要尝试特性等级11.0以及以下的版本
			hr = D3D11CreateDevice(nullptr, d3dDriverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, m_pd3dDevice.GetAddressOf(), &featureLevel, m_pd3dImmediateContext.GetAddressOf());
		}

		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	// 检测是否支持特性等级11.0或11.1
	if (featureLevel != D3D_FEATURE_LEVEL_11_0 && featureLevel != D3D_FEATURE_LEVEL_11_1)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	// 检测 MSAA支持的质量等级
	m_pd3dDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality);
	assert(m_4xMsaaQuality > 0); //支持特性等级11_0的显示适配器必然支持所有渲染目标纹理格式的4倍多重采样




	ComPtr<IDXGIDevice> dxgiDevice = nullptr;
	ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;
	ComPtr<IDXGIFactory1> dxgiFactory1 = nullptr;	// D3D11.0(包含DXGI1.1)的接口类
	ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr;	// D3D11.1(包含DXGI1.2)特有的接口类

	// 为了正确创建 DXGI交换链，首先我们需要获取创建 D3D设备 的 DXGI工厂，否则会引发报错：
	// "IDXGIFactory::CreateSwapChain: This function is being called with a device from a different IDXGIFactory."
	HR(m_pd3dDevice.As(&dxgiDevice));
	HR(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgiFactory1.GetAddressOf())));

	// 查看该对象是否包含IDXGIFactory2接口
	hr = dxgiFactory1.As(&dxgiFactory2);
	// 如果包含，则说明支持D3D11.1
	if (dxgiFactory2 != nullptr)
	{
		HR(m_pd3dDevice.As(&m_pd3dDevice1));
		HR(m_pd3dImmediateContext.As(&m_pd3dImmediateContext1));
		// 填充各种结构体用以描述交换链 DXGI交换链(IDXGISwapChain)缓存了一个或多个表面(2D纹理)，它们都可以称作后备缓冲区(backbuffer)
		DXGI_SWAP_CHAIN_DESC1 sd; 
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = m_ClientWidth;
		sd.Height = m_ClientHeight;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		// 是否开启4倍多重采样？
		if (m_Enable4xMsaa)
		{
			sd.SampleDesc.Count = 4;
			sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
		}
		else
		{
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
		}
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd;
		fd.RefreshRate.Numerator = 60;
		fd.RefreshRate.Denominator = 1;
		fd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		fd.Windowed = TRUE;
		// 为当前窗口创建交换链
		HR(dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice.Get(), m_hMainWnd, &sd, &fd, nullptr, m_pSwapChain1.GetAddressOf()));
		HR(m_pSwapChain1.As(&m_pSwapChain));
	}
	else
	{
		// 填充DXGI_SWAP_CHAIN_DESC用以描述交换链
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferDesc.Width = m_ClientWidth;
		sd.BufferDesc.Height = m_ClientHeight;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		// 是否开启4倍多重采样？
		if (m_Enable4xMsaa)
		{
			sd.SampleDesc.Count = 4;
			sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
		}
		else
		{
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
		}
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.OutputWindow = m_hMainWnd;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;
		HR(dxgiFactory1->CreateSwapChain(m_pd3dDevice.Get(), &sd, m_pSwapChain.GetAddressOf()));
	}

	

	// 可以禁止alt+enter全屏
	dxgiFactory1->MakeWindowAssociation(m_hMainWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

	// 设置调试对象名
	D3D11SetDebugObjectName(m_pd3dImmediateContext.Get(), "ImmediateContext");
	DXGISetDebugObjectName(m_pSwapChain.Get(), "SwapChain");

	// 每当窗口被重新调整大小的时候，都需要调用这个OnResize函数。现在调用以避免代码重复
	OnResize();

	return true;
}

void D3DApp::CalculateFrameStats()
{
	// 该代码计算每秒帧速，并计算每一帧渲染需要的时间，显示在窗口标题
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << m_MainWndCaption << L"    "
			<< L"FPS: " << fps << L"    "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(m_hMainWnd, outs.str().c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}


