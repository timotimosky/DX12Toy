#ifndef GAMEAPP_H
#define GAMEAPP_H
#include "d3dApp.h"

//我们编写游戏逻辑的地方，这里需要进行逐帧的更新及绘制。
class GameApp : public D3DApp
{
public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();//每帧画面的绘制
};


#endif