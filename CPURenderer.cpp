#include <windows.h>

#define ARRAY_COUNT(x) (sizeof(x) / sizeof( ( (x)[0]) ) )

#ifdef UNICODE
#define WIN32_STR(x) L##x
#define WIN32_STRPTR LPWSTR
#else
#define WIN32_STR(x) x
#define WIN32_STRPTR LPSTR
#endif

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
            PAINTSTRUCT paintStruct             = {};
            HDC deviceContext                   = {};
            deviceContext                       = BeginPaint(hWnd, &paintStruct);
            const DWORD bitmapWidth             = 1001;
            const DWORD bitmapHeight            = 1000;
            const unsigned short bitsPerPixel   = 24;
            const unsigned int bitsPerRow       = bitmapWidth * bitsPerPixel;
            int bitMask                         = (int)0b11111111111111111111111111100000;
            const unsigned int bitStride        = (bitsPerRow + 31) & bitMask; // add just enough to mask the unused bits
            const unsigned int byteStride       = bitStride >> 3; // divide by 2^3

            // CREATE BITMAPINFO
            BITMAPINFOHEADER bitmapHeader       = {};
            bitmapHeader.biSize                 = sizeof(BITMAPINFOHEADER);
            bitmapHeader.biWidth                = bitmapWidth;
            bitmapHeader.biHeight               = -bitmapHeight; // origin @ upper-left corner
            bitmapHeader.biPlanes               = 1;
            bitmapHeader.biBitCount             = bitsPerPixel;
            bitmapHeader.biCompression          = BI_RGB;
            bitmapHeader.biSizeImage            = AbsoluteValue(bitmapHeader.biHeight) * byteStride;
            bitmapHeader.biXPelsPerMeter        = 0;
            bitmapHeader.biYPelsPerMeter        = 0;
            bitmapHeader.biClrUsed              = 0;
            bitmapHeader.biClrImportant         = 0;
            BITMAPINFO bitmapInfo               = {};
            bitmapInfo.bmiHeader                = bitmapHeader;
            void* pixelData                     = 0;
            int colorUse                        = DIB_RGB_COLORS;
            // CREATE DIB & GET PIXELDATA POINTER
            HBITMAP bitmapHandle = CreateDIBSection(
                deviceContext,
                &bitmapInfo,
                colorUse,
                &pixelData,
                0,
                0
            );
            if (!bitmapHandle || !pixelData)
            {
                OutputDebugString(WIN32_STR("Failed to create DIB section"));
                return -1;
            }

            // RENDER PIXELDATA
            for (int scanLine = 0; scanLine < bitmapHeight; scanLine++)
            {
                for (int pixel = 0; pixel < byteStride; pixel += 3)
                {
                    int currentByte = (scanLine * byteStride) + pixel;

                    float t = (float)pixel / (float)byteStride;
                    int value = 255.0f * t;

                    ((unsigned char*)pixelData)[currentByte]      = (unsigned char)(value);   // B
                    ((unsigned char*)pixelData)[currentByte + 1]  = (unsigned char)(0x00);    // G
                    ((unsigned char*)pixelData)[currentByte + 2]  = (unsigned char)(0x00);    // R
                }
            }

            // SET DIB's PIXEL DATA
            int linesDrawn = SetDIBitsToDevice(
                deviceContext,
                0, 0,
                bitmapWidth, bitmapHeight,
                0, 0,
                0,
                bitmapHeight,
                pixelData,
                &bitmapInfo,
                colorUse
            );

            EndPaint(hWnd, &paintStruct);

            DeleteObject(bitmapHandle); // TODO - reuse the same bitmap to avoid allocs
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
