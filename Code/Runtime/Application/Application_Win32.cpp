//#include "WindowsPlatform.h"
//
//#include <Platform/PrimitiveTypes.h>
//#include <glad/glad.h>
//
//struct WindowData
//{
//	const wchar_t* m_Name = L"CookieKat Engine";
//};
//
//HINSTANCE opengl32dll;
//void* GetOpenGLProcAdress(const char* procName) { return GetProcAddress(opengl32dll, procName); }
//
//LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//
//int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
//{
//	// Create and register window class
//	constexpr wchar_t CLASS_NAME[] = L"Engine Window Class";
//	WNDCLASS wc{};
//	wc.lpfnWndProc = WindowProc;
//	wc.hInstance = hInstance;
//	wc.lpszClassName = CLASS_NAME;
//	RegisterClass(&wc);
//
//	// Create window
//	HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"CookieKat Engine", WS_OVERLAPPEDWINDOW | CS_OWNDC, CW_USEDEFAULT, CW_USEDEFAULT,
//							   CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
//	if (hwnd == NULL)
//	{
//		return 0;
//	}
//
//	PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR),
//								 1,
//								 PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // Flags
//								 PFD_TYPE_RGBA,												 // The kind of framebuffer. RGBA or palette.
//								 32,														 // Colordepth of the framebuffer.
//								 0,
//								 0,
//								 0,
//								 0,
//								 0,
//								 0,
//								 0,
//								 0,
//								 0,
//								 0,
//								 0,
//								 0,
//								 0,
//								 24, // Number of bits for the depthbuffer
//								 8,	 // Number of bits for the stencilbuffer
//								 0,	 // Number of Aux buffers in the framebuffer.
//								 PFD_MAIN_PLANE,
//								 0,
//								 0,
//								 0,
//								 0};
//
//	// Set pixel format
//	HDC hdc = GetDC(hwnd);
//	int pixelFmt = ChoosePixelFormat(hdc, &pfd);
//	SetPixelFormat(hdc, pixelFmt, &pfd);
//
//	// Create OpenGL context
//	HGLRC context = wglCreateContext(hdc);
//	wglMakeCurrent(hdc, context);
//
//	ShowWindow(hwnd, nCmdShow);
//
//	// opengl32dll = LoadLibraryA("opengl32.dll");
//	// gladLoadGLLoader((GLADloadproc)GetOpenGLProcAdress);
//	gladLoadGL();
//
//	MSG msg{};
//	while (GetMessage(&msg, NULL, 0, 0) > 0)
//	{
//		TranslateMessage(&msg);
//		DispatchMessage(&msg);
//
//		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT);
//		SwapBuffers(hdc);
//	}
//
//	// Destroy OpenGL context
//	wglMakeCurrent(hdc, NULL);
//	wglDeleteContext(context);
//
//	return 0;
//}
//
//LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	switch (uMsg)
//	{
//	case WM_DESTROY:
//		PostQuitMessage(0);
//		return 0;
//
//	case WM_PAINT: {
//		PAINTSTRUCT ps;
//		HDC hdc = BeginPaint(hwnd, &ps);
//
//		// All painting occurs here, between BeginPaint and EndPaint.
//
//		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
//
//		EndPaint(hwnd, &ps);
//	}
//		return 0;
//	}
//	return DefWindowProc(hwnd, uMsg, wParam, lParam);
//}