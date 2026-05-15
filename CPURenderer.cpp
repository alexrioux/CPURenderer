extern "C" int _fltused = 0;

#include <windows.h>

#define ARRAY_COUNT(x) (sizeof(x) / sizeof( ( (x)[0]) ) )

#ifdef UNICODE
#define WIN32_STR(x) L##x
#define WIN32_STRPTR LPWSTR
#else
#define WIN32_STR(x) x
#define WIN32_STRPTR LPSTR
#endif

// TODO - restructure memory layout of structs for better packing
struct FrameBuffer
{
    unsigned int width;
    unsigned int height;
    unsigned short bitsPerPixel;
    void* data;
}
g_FrameBuffer;
struct Win32DIB
{
    BITMAPINFO info;
    int colorUse;
}
g_DIB;

static void RenderGradient(FrameBuffer& fb)
{
    if (!fb.data)
    {
        OutputDebugString(WIN32_STR("Could not render gradient, data is unitialized\n"));
        return;
    }

    // CALCULATE STRIDE
    unsigned int bitsPerRow     = fb.width * fb.bitsPerPixel;
    int bitMask                 = (int)0b11111111111111111111111111100000;
    unsigned int bitStride      = (bitsPerRow + 31) & bitMask; // add just enough to mask the unused bits
    unsigned int byteStride     = bitStride >> 3; // divide by 2^3

    // RENDER PIXELDATA
    unsigned int bytesPerPixel  = fb.bitsPerPixel >> 3;
    unsigned int bytesPerRow    = fb.width * bytesPerPixel;
    for (unsigned int row = 0; row < fb.height; row++)
    {
        for (unsigned int column = 0; column < bytesPerRow; column += bytesPerPixel)
        {
            float tb        = (float)column / (float)byteStride;
            int blueValue   = (int)(255.0f * tb); // TODO - change constant to max byte number
            float tr        = (float)row / (float)fb.height;
            int redValue    = (int)(255.0f * tr); // TODO - change constant to max byte number

            int currentByte                             = (row * byteStride) + column;
            ((unsigned char*)fb.data)[currentByte]      = (unsigned char)(blueValue);   // B
            ((unsigned char*)fb.data)[currentByte + 1]  = (unsigned char)(0x00);    // G
            ((unsigned char*)fb.data)[currentByte + 2]  = (unsigned char)(redValue);  // R
        }
    }
}

long long AbsoluteValue(long long value)
{
    return (~value) - 1;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT paintStruct     = {};
            HDC deviceContext           = {};
            deviceContext               = BeginPaint(hWnd, &paintStruct);

            // UPDATE DEVICE CONTEXT FROM DIB
            int linesDrawn = SetDIBitsToDevice(
                deviceContext,
                0, 0,
                (unsigned int)g_DIB.info.bmiHeader.biWidth, (unsigned int)AbsoluteValue(g_DIB.info.bmiHeader.biHeight),
                0, 0,
                0,
                (unsigned int)AbsoluteValue(g_DIB.info.bmiHeader.biHeight),
                g_FrameBuffer.data,
                &g_DIB.info,
                g_DIB.colorUse
            );

            EndPaint(hWnd, &paintStruct);

            if (linesDrawn <= 0)
            {
                OutputDebugString(WIN32_STR("SetDIBitsToDevice failed\n"));
            }
        }
        break;
            RenderGradient(g_FrameBuffer);
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

int __stdcall Win32EntryPoint(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
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
        OutputDebugString(WIN32_STR("Failed to register window class\n"));
        return 1;
    }

    // INITIALIZE GLOBAL FRAMEBUFFER & CALCULATE STRIDE
    g_FrameBuffer = {};
    g_FrameBuffer.width             = 1001;
    g_FrameBuffer.height            = 1000;
    g_FrameBuffer.bitsPerPixel      = 24;
    g_FrameBuffer.data              = 0;
    unsigned int bitsPerRow         = g_FrameBuffer.width * g_FrameBuffer.bitsPerPixel;
    int bitMask                     = (int)0b11111111111111111111111111100000;
    unsigned int bitStride          = (bitsPerRow + 31) & bitMask; // add just enough to mask the unused bits
    unsigned int byteStride         = bitStride >> 3; // divide by 2^3

    // CREATE DEVICE INDEPENDANT BITMAP (DIB)
    g_DIB = {};
    g_DIB.info                      = {};
    g_DIB.colorUse                  = DIB_RGB_COLORS;
    BITMAPINFOHEADER bitmapHeader   = {};
    bitmapHeader.biSize             = sizeof(BITMAPINFOHEADER);
    bitmapHeader.biWidth            = g_FrameBuffer.width;
    bitmapHeader.biHeight           = -(int)(g_FrameBuffer.height); // origin @ upper-left corner
    bitmapHeader.biPlanes           = 1;
    bitmapHeader.biBitCount         = g_FrameBuffer.bitsPerPixel;
    bitmapHeader.biCompression      = BI_RGB;
    bitmapHeader.biSizeImage        = (unsigned long)(AbsoluteValue(bitmapHeader.biHeight) * byteStride);
    bitmapHeader.biXPelsPerMeter    = 0;
    bitmapHeader.biYPelsPerMeter    = 0;
    bitmapHeader.biClrUsed          = 0;
    bitmapHeader.biClrImportant     = 0;
    g_DIB.info.bmiHeader            = bitmapHeader;

    // ALLOCATE FRAMEBUFFER'S MEMORY
    SIZE_T size = g_FrameBuffer.height * byteStride;
    g_FrameBuffer.data = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!g_FrameBuffer.data)
    {
        OutputDebugString(WIN32_STR("Failed to create DIB\n"));
        return -1;
    }

    RenderGradient(g_FrameBuffer);

    // CREATE WINDOW
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
        OutputDebugString(WIN32_STR("Failed to create window\n"));
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
