
//
//#define q	3		/* for 2^3 points */
////#define N	(1<<q)		/* N-point FFT, iFFT */
//		//====================================================================================	
//		
//	    int N = 8;
//		Comp *sig, *f, *sig0;
//		
//		sig = (Comp *)malloc(sizeof(Comp) * (size_t)N);
//	    sig0 = (Comp *)malloc(sizeof(Comp) * (size_t)N);
//	    f = (Comp *)malloc(sizeof(Comp) * (size_t)N);
//    
//		int k;
//		
//		for(k=0; k<N; k++) {
//			sig[k].a = sin(2*PI*50*k/N)+sin((2*PI*70*k/N)+PI/4);
//			sig[k].b = 0;
//		}
//		
//		test_fft(sig, f, sig0, N);
//    	float vector_teste[N];
//    	system("pause");
//		//fft_magnitude(f, vector_teste, N);
//		
//		fft_magnitude(f, vector_teste, N);
//		
//		int yw, xw, fft_graphic_len = 100;
//    	for (yw = 0; yw < N*fft_graphic_len; yw++){
//    		printf("%f\n", vector_teste[yw]);
//    		for(xw=0; xw< vector_teste[yw/fft_graphic_len]*30; xw++ )
//    			DrawPixel(200+yw, 500-xw, 0xffffff);
//		}
//        
//		
//		// Desenhando o sinal de entrada	
//		float jjk;
//		for(jjk=0; jjk<800; jjk+=0.01) {
//			DrawPixel(jjk+200, (50*sin(2*PI*50*jjk/10000)+50*sin((2*PI*200*jjk/10000)+PI/4)+200), 0xffffff);
//		}

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fft_calculate.h"

#define Assert(c) do { if (!(c)) __debugbreak(); } while (0)

#pragma comment (lib, "user32.lib")
#pragma comment (lib, "gdi32.lib")

typedef DWORD COLORREF;

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
    int Index;
    for(Index = 0; Index < ClientWidth * ClientHeight; ++Index) {
        *Pixel++ = Color;
    }
}

void draw_fft(HDC DeviceContext, Comp *y, int n, int width, int height) {
    double x_step = (double)width / n;
    double max_amplitude = 0;
    
    RECT textRect;
	DrawText(DeviceContext,TEXT("Gráfico Magnitude com dominio da frequncia"),-1,&textRect, DT_CENTER | DT_NOCLIP);     
    
    // Encontra a amplitude máxima
    int i;
    for (i = 0; i < n / 2; i++) {
        double amplitude = sqrt(y[i].a * y[i].a + y[i].b * y[i].b);
        if (amplitude > max_amplitude)
            max_amplitude = amplitude;
    }

    // Desenha o gráfico
    MoveToEx(DeviceContext, 0, height, NULL); // Movendo para a posição inicial do gráfico
    for (i = 0; i < n / 2; i++) {
        double x = i * x_step;
        double amplitude = sqrt(y[i].a * y[i].a + y[i].b * y[i].b);
        double y_pos = (1.0 - amplitude / max_amplitude) * height;

        // Normaliza a posição y para o intervalo [0, height-1]
        y_pos = fmax(0, fmin(y_pos, height - 1));

        // Desenha uma linha até o próximo ponto do gráfic
        SelectObject(DeviceContext,GetStockObject(WHITE_PEN));
        
        LineTo(DeviceContext, x, y_pos);
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
    
    // Definindo a amostragem e tempo
    int amostragem = 1000;
    
    double *t = malloc(601 * sizeof(double));
    
    int N = 601; // Número de amostras
    
    Comp *s = malloc(N * sizeof(Comp));

    // Construindo o sinal
    int i;
    for (i = 0; i < N; i++) {
        t[i] = (double)i / amostragem;
        s[i] = comp_create(sin(2 * M_PI * 50 * t[i]) + sin(2 * M_PI * 200 * t[i] + M_PI / 4), 0);
    }

    // Definindo o FFT
    Comp *y = malloc(N * sizeof(Comp));
    fft(s, y, 1, N, 0);
	
	i = 0;
    for (;;) {
        MSG Message;
        if (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE)) {
            if (Message.message == WM_QUIT) break;
            TranslateMessage(&Message);
            DispatchMessage(&Message);
            continue;
        }
		
        // Limpa a tela
        ClearScreen(0x333333);
		int color = 0xFFFFFFFF;
        // Desenha o FFT
        
     
        for (i=0; i<100; i++)
        	draw_fft(DeviceContext, y, N, ClientWidth, ClientHeight);
		
        // Mostra a imagem
        StretchDIBits(DeviceContext, 0, 0, ClientWidth, ClientHeight, 0, 0, ClientWidth, ClientHeight, Memory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        
    }

    // Liberando a memória
    free(t);
    free(s);
    free(y);

    return 0;
}
