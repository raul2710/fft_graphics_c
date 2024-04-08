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
    double x_step = (double)width / n;	// Faz divide o comprimento pela quantidade de amostra
    double max_amplitude = 0;
    
    // Escreve o título acima no centro da tela
    RECT textRect;
	DrawText(DeviceContext,TEXT("Gráfico Magnitude com dominio da frequncia"),-1,&textRect, DT_CENTER | DT_NOCLIP);     
    
    // Encontra a amplitude máxima e pega o módulo
    int i;
    for (i = 0; i < n / 2; i++) {
    	// Tirando o módulo do vetor imaginário e pegando a amplitude máxima
        double amplitude = sqrt(y[i].a * y[i].a + y[i].b * y[i].b);
        if (amplitude > max_amplitude)
            max_amplitude = amplitude;
    }

    // Faz o gráfico na tela
    MoveToEx(DeviceContext, 0, height, NULL); // Coloca no inicio do gráfico
    for (i = 0; i < n / 2; i++) {
        double x = i * x_step;
        double amplitude = sqrt(y[i].a * y[i].a + y[i].b * y[i].b);
        double y_pos = (1.0 - amplitude / max_amplitude) * height;

        // Normaliza a posição y para o intervalo [0, height-1]
        y_pos = fmax(0, fmin(y_pos, height - 1));

        // Coloca a cor da linha como branca
        SelectObject(DeviceContext,GetStockObject(WHITE_PEN));
        
        // Desenha uma linha até o próximo ponto do gráfico
        // DrawPixel(x, y_pos, 0xFFFFFF);

        LineTo(DeviceContext, x, y_pos);
    }
}

void draw_signal(HDC DeviceContext, Comp *y, int n, int width, int height) {
    double x_step = (double)width / n;	// Faz divide o comprimento pela quantidade de amostra
    double max_amplitude = 0;
    
    // Faz o gráfico na tela
    MoveToEx(DeviceContext, 0, y[1].a * 100 + 300, NULL); // Coloca no inicio do gráfico
    // Desenhando o sinal de entrada
    int i;
	for (i=0; i<n/2; i++)	
    	LineTo(DeviceContext, i*10, y[i].a * 100 + 300);
//		float i;
//		for(i=0; i<800; i+=0.01) {
//			DrawPixel(i+200, y[i].a);
//		}
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
    
    // Define as frequencias da onda 1 e 2 que serão somadas
    int f1 = 50;
    int f2 = 200;
    
    // Definindo o tamanho da amostragem
    int amostragem = 1000;
    
    // Número de amostras
    int N = 601; 
    
    // Cria o vetor tempo
    double *t = malloc(N * sizeof(double)); 
    
    // Cria o vetor para o sinal
    Comp *signal = malloc(N * sizeof(Comp));		

    // Constrói o sinal e armazena na variavel signal
    int i;
    for (i = 0; i < N; i++) {
        t[i] = (double)i / amostragem;
        signal[i] = comp_create(sin(2 * M_PI * f1 * t[i]) + sin(2 * M_PI * f2 * t[i] + M_PI / 4), 0);
    }

    // Faz o cálculo da transformada e armazena o resultado na variável signalOut
    Comp *signalOut = malloc(N * sizeof(Comp));
    fft(signal, signalOut, 1, N, 0);
	
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

        // Desenha o sinal de entrada	
		float jjk;
		for(jjk=0; jjk<800; jjk+=0.01) {
			DrawPixel(jjk+200, (50*sin(2*M_PI*50*jjk/10000)+50*sin((2*M_PI*200*jjk/10000)+M_PI/4)+200), 0xffffff);
		}
        
        // Desenha o os pontos de amostragem
        //draw_signal(DeviceContext, signal, N, ClientWidth, ClientHeight);
        
        // Desenha o gráfico da transformada
		draw_fft(DeviceContext, signalOut, N, ClientWidth, ClientHeight);
		
        // Mostra a imagem
        StretchDIBits(DeviceContext, 0, 0, ClientWidth, ClientHeight, 0, 0, ClientWidth, ClientHeight, Memory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        
    }

    // Libera a memória utilizada
    free(t);
    free(signal);
    free(signalOut);

    return 0;
}
