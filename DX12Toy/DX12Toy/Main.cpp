#include "stdafx.h"
#include "dx12mgr.h"

bool InitializeWindow(HINSTANCE hInstance,
    int ShowWnd,
    int width, int height,
    bool fullscreen)
{
    //检查是否要全屏显示
    if (fullscreen)
    {
        HMONITOR hmon = MonitorFromWindow(hwnd,
            MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(hmon, &mi);

        width = mi.rcMonitor.right - mi.rcMonitor.left;
        height = mi.rcMonitor.bottom - mi.rcMonitor.top;
    }

    //此结构将描述我们的窗口
    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc; //窗口事件
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WindowName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    //注册调用RegisterClassEx函数的类
    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Error registering class",
            L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    //通过调用CreateWindowEX函数使用刚刚注册的类创建窗
    hwnd = CreateWindowEx(NULL,
        WindowName,
        WindowTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        NULL,
        NULL,
        hInstance,
        NULL);

    //如果未能成功获得窗口句柄，则将返回false并显示错误消息。
        if (!hwnd)
        {
            MessageBox(NULL, L"Error creating window",
                L"Error", MB_OK | MB_ICONERROR);
            return false;
        }

       // 如果要全屏显示，则需要删除窗口样式。如果仅在单个监视器上工作，则建议您以窗口模式进行调试。如果您在全屏窗口中遇到断点或异常，
        //则使用单个监视器调试全屏应用程序可能会非常麻烦。
       if (fullscreen)
       {
           SetWindowLong(hwnd, GWL_STYLE, 0);
       }

      //显示该窗口并更新该窗口，然后返回true。

       ShowWindow(hwnd, ShowWnd);
       UpdateWindow(hwnd);

       return true;
}

void mainloop() {
    MSG msg;
    //将MSG对象的内存清零
    ZeroMemory(&msg, sizeof(MSG));

    while (true)
    {
        //该MSG对象将保存PeekMessage函数获取的所有Windows消息。如果找到一条消息，我们将翻译该消息然后将其分派。
        //这只是为了让Windows认为我们的程序没有停止响应。
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            //如果找不到消息，请运行我们的游戏代码并继续循环。
            // run game code
        }
    }
}


//Windows过程回调。创建它时，我们必须将此函数提供给Windows类。这样我们就可以获得从Windows发送的Windows消息，
//例如按键，鼠标移动或窗口大小调整。现在，我们所要检查的只是转义键，以及是否已按下窗口右上方的x按钮。
LRESULT CALLBACK WndProc(HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            if (MessageBox(0, L"Are you sure you want to exit?",
                L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
                DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd,
        msg,
        wParam,
        lParam);
}

//如果是windows程序，那么WinMain是入口函数，在VS2017中新建项目为“win32项目”
//如果是dos控制台程序，那么main是入口函数，在VS2017中新建项目为“win32控制台应用程序
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd)
{
    // create the window
    if (!InitializeWindow(hInstance, nShowCmd, Width, Height, FullScreen))
    {
        MessageBox(0, L"Window Initialization - Failed",
            L"Error", MB_OK);
        return 0;
    }

    // initialize direct3d
    if (!InitD3D())
    {
        MessageBox(0, L"Failed to initialize direct3d 12",
            L"Error", MB_OK);
        Cleanup();
        return 1;
    }

    // start the main loop
    mainloop();

    // we want to wait for the gpu to finish executing the command list before we start releasing everything
    WaitForPreviousFrame();

    // close the fence event
    CloseHandle(fenceEvent);

    return 0;
}