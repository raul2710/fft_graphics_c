#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include "fft_calculate.h"

#define Assert(c) do { if (!(c)) __debugbreak(); } while (0)

#pragma comment (lib, "user32.lib")
#pragma comment (lib, "gdi32.lib")

typedef uint32_t u32;

int ClientWidth;
int ClientHeight;
void *Memory;

void DrawPixel(int X, int Y, u32 Color) {
    u32 *Pixel = (u32 *)Memory;
    Pixel += Y * ClientWidth + X;
    *Pixel = Color;
}

void ClearScreen(u32 Color) {
    u32 *Pixel = (u32 *)Memory;
    int Index = 0;
    for(Index; Index < ClientWidth * ClientHeight; ++Index) {
        *Pixel++ = Color;
    }
}

LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
    switch(Message) {
        case WM_DESTROY: {
            PostQuitMessage(0);
        } break;
        case WM_KEYDOWN: {
            switch(WParam) {
                case 'O': {
                    DestroyWindow(Window);
                } break;
            }
        } break;
        default: {
            return DefWindowProcW(Window, Message, WParam, LParam);
        }
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR Cmd, int CmdShow) {

    // Create a window.

    WNDCLASSW WindowClass = {
        .lpszClassName = L"MyWindowClass",
        .lpfnWndProc = WindowProc,
        .hInstance = Instance,
        .hCursor = LoadCursor(NULL, IDC_CROSS),
    };
    ATOM Atom = RegisterClassW(&WindowClass);
    Assert(Atom && "Falha ao registrar a janela grafica");

    HWND Window = CreateWindowW(WindowClass.lpszClassName, L"Desenhando pixels", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, Instance, NULL);
    Assert(Window && "Falha ao registrar a janela grafica");

    ShowWindow(Window, CmdShow);

    // Allocate memory.

    RECT Rect;
    GetClientRect(Window, &Rect);
    ClientWidth = Rect.right - Rect.left;
    ClientHeight = Rect.bottom - Rect.top;

    Memory = VirtualAlloc(0, ClientWidth * ClientHeight * 4, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    // Cria BITMAPINFO struct para usar em StretchDIBits().

    BITMAPINFO BitmapInfo;
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = ClientWidth;
    // biHeight negativo colocado como sistema de coordenadas na origem. Caso contrario embaixo a esquerda.
    BitmapInfo.bmiHeader.biHeight = -ClientHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    HDC DeviceContext = GetDC(Window);

    for(;;) {
        MSG Message;
        if(PeekMessage(&Message, NULL, 0, 0, PM_REMOVE)) {
            if(Message.message == WM_QUIT) break;
            TranslateMessage(&Message);
            DispatchMessage(&Message);
            continue;
        }

        // Limpa a tela

        ClearScreen(0x333333);
//====================================================================================
    int r = 40; // raio do circulo
    int off_set = 100;
    int x = 0;
    int y = r;
    int d = 5/4 - r;
    //
    DrawPixel(x+off_set, y+off_set, 0xffffff);
    DrawPixel(x+off_set, -y+off_set, 0xffffff);
    DrawPixel(-x+off_set, y+off_set, 0xffffff);
    DrawPixel(-x+off_set, -y+off_set, 0xffffff);
    DrawPixel(y+off_set, x+off_set, 0xffffff);
    DrawPixel(y+off_set, -x+off_set, 0xffffff);
    DrawPixel(-y+off_set, x+off_set, 0xffffff);
    DrawPixel(-y+off_set, -x+off_set, 0xffffff);
    //
    while (x < y)
    {
      if (d < 0)
      {
      /* Selecione E */
      d = d + 2*x + 3;
      x++;
         } else{
         /* Selecione SE */
         d = d + 2*(x - y) + 5;
         x++;
         y--;
      }/*end if*/
       DrawPixel(x+off_set, y+off_set, 0xffffff);
       DrawPixel(x+off_set, -y+off_set, 0xffffff);
       DrawPixel(-x+off_set, y+off_set, 0xffffff);
       DrawPixel(-x+off_set, -y+off_set, 0xffffff);
       DrawPixel(y+off_set, x+off_set, 0xffffff);
       DrawPixel(y+off_set, -x+off_set, 0xffffff);
       DrawPixel(-y+off_set, x+off_set, 0xffffff);
       DrawPixel(-y+off_set, -x+off_set, 0xffffff);
    } // fim do while
    //
    // Mostra a imagem
//====================================================================================
     StretchDIBits(DeviceContext, 0, 0, ClientWidth, ClientHeight, 0, 0, ClientWidth, ClientHeight, Memory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
    }
    test_fft(12, 12, 12, 12);
    getch();
    return 0;
}

