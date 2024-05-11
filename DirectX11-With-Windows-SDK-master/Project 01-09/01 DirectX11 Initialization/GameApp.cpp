#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	return true;
}

void GameApp::OnResize()
{
	D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{

}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);
	static float blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };	// RGBA = (0,0,255,255)
	//在每一帧画面绘制的操作中，我们需要清理一遍渲染目标视图绑定的缓冲区
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), blue);
	//每一次清空我们需要将深度值设为1.0f，模板值设为0.0f。其中深度值1.0f表示距离最远处
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//前后备缓冲区交换并呈现
	HR(m_pSwapChain->Present(0, 0));
}
