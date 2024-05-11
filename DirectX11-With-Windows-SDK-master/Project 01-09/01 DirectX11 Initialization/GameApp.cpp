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
	//��ÿһ֡������ƵĲ����У�������Ҫ����һ����ȾĿ����ͼ�󶨵Ļ�����
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), blue);
	//ÿһ�����������Ҫ�����ֵ��Ϊ1.0f��ģ��ֵ��Ϊ0.0f���������ֵ1.0f��ʾ������Զ��
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//ǰ�󱸻���������������
	HR(m_pSwapChain->Present(0, 0));
}
