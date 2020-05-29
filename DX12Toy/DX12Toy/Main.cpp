#include "stdafx.h"
#include "dx12mgr.h"

bool InitializeWindow(HINSTANCE hInstance,
    int ShowWnd,
    int width, int height,
    bool fullscreen)
{
    //����Ƿ�Ҫȫ����ʾ
    if (fullscreen)
    {
        HMONITOR hmon = MonitorFromWindow(hwnd,
            MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(hmon, &mi);

        width = mi.rcMonitor.right - mi.rcMonitor.left;
        height = mi.rcMonitor.bottom - mi.rcMonitor.top;
    }

    //�˽ṹ���������ǵĴ���
    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc; //�����¼�
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WindowName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    //ע�����RegisterClassEx��������
    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Error registering class",
            L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    //ͨ������CreateWindowEX����ʹ�øո�ע����ഴ����
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

    //���δ�ܳɹ���ô��ھ�����򽫷���false����ʾ������Ϣ��
        if (!hwnd)
        {
            MessageBox(NULL, L"Error creating window",
                L"Error", MB_OK | MB_ICONERROR);
            return false;
        }

       // ���Ҫȫ����ʾ������Ҫɾ��������ʽ��������ڵ����������Ϲ������������Դ���ģʽ���е��ԡ��������ȫ�������������ϵ���쳣��
        //��ʹ�õ�������������ȫ��Ӧ�ó�����ܻ�ǳ��鷳��
       if (fullscreen)
       {
           SetWindowLong(hwnd, GWL_STYLE, 0);
       }

      //��ʾ�ô��ڲ����¸ô��ڣ�Ȼ�󷵻�true��

       ShowWindow(hwnd, ShowWnd);
       UpdateWindow(hwnd);

       return true;
}

void mainloop() {
    MSG msg;
    //��MSG������ڴ�����
    ZeroMemory(&msg, sizeof(MSG));

    while (true)
    {
        //��MSG���󽫱���PeekMessage������ȡ������Windows��Ϣ������ҵ�һ����Ϣ�����ǽ��������ϢȻ������ɡ�
        //��ֻ��Ϊ����Windows��Ϊ���ǵĳ���û��ֹͣ��Ӧ��
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            //����Ҳ�����Ϣ�����������ǵ���Ϸ���벢����ѭ����
            // run game code
        }
    }
}


//Windows���̻ص���������ʱ�����Ǳ��뽫�˺����ṩ��Windows�ࡣ�������ǾͿ��Ի�ô�Windows���͵�Windows��Ϣ��
//���簴��������ƶ��򴰿ڴ�С���������ڣ�������Ҫ����ֻ��ת������Լ��Ƿ��Ѱ��´������Ϸ���x��ť��
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

//�����windows������ôWinMain����ں�������VS2017���½���ĿΪ��win32��Ŀ��
//�����dos����̨������ômain����ں�������VS2017���½���ĿΪ��win32����̨Ӧ�ó���
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