#include <windows.h>

#define ARRAY_COUNT(x) (sizeof(x) / sizeof( ( (x)[0]) ) )

#ifdef UNICODE
#define WIN32_STR(x) L##x
#define WIN32_STRPTR LPWSTR
#else
#define WIN32_STR(x) x
#define WIN32_STRPTR LPSTR
#endif


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT paintStruct     = {};
    HDC deviceContext           = {};

    switch (message)
    {
        case WM_PAINT:
        {
            OutputDebugString(WIN32_STR("PAINT msg received\n"));

            deviceContext       = BeginPaint(hWnd, &paintStruct);
            WIN32_STRPTR text   = WIN32_STR("Text Out");
            TextOut(deviceContext, 5, 5, text, ARRAY_COUNT(text));
            EndPaint(hWnd, &paintStruct);
        }
        break;
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        break;
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // CREATE WINDOW CLASS
    WNDCLASSEX windowClass      = {};
    windowClass.cbSize          = sizeof(windowClass);
    windowClass.style           = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc     = WndProc;
    windowClass.hInstance       = hInstance;
    windowClass.lpszClassName   = WIN32_STR("CPURenderer");

    if (!RegisterClassEx(&windowClass))
    {
        OutputDebugString(WIN32_STR("Failed to register window class"));
        return 1;
    }

    HWND windowHandle = CreateWindowEx(
        0,
        windowClass.lpszClassName,
        0,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        0,
        0,
        hInstance,
        0
    );

    if (!windowHandle)
    {
        OutputDebugString(WIN32_STR("Failed to create window"));
        return 1;
    }

    MSG msg = {};
    while (GetMessage(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
