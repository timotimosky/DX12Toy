#ifndef GAMEAPP_H
#define GAMEAPP_H
#include "d3dApp.h"

//���Ǳ�д��Ϸ�߼��ĵط���������Ҫ������֡�ĸ��¼����ơ�
class GameApp : public D3DApp
{
public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();//ÿ֡����Ļ���
};


#endif