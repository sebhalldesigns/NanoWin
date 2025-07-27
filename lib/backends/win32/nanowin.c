/***************************************************************
**
** NanoKit Library Source File
**
** File         :  nanowin.c
** Module       :  nanowin
** Author       :  SH
** Created      :  2025-02-23 (YYYY-MM-DD)
** License      :  MIT
** Description  :  NanoKit Window API
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanowin.h>
#include <nanodraw.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/* to handle unicode input */
#ifdef IS_HIGH_SURROGATE
#undef IS_HIGH_SURROGATE
#endif
#ifdef IS_LOW_SURROGATE
#undef IS_LOW_SURROGATE
#endif

#define IS_HIGH_SURROGATE(wch) (((wch) >= 0xD800) && ((wch) <= 0xDBFF))

#define IS_LOW_SURROGATE(wch)  (((wch) >= 0xDC00) && ((wch) <= 0xDFFF))

#define WGL_CONTEXT_MAJOR_VERSION_ARB       (0x2091U)
#define WGL_CONTEXT_MINOR_VERSION_ARB       (0x2092U)
#define WGL_CONTEXT_PROFILE_MASK_ARB        (0x9126U)
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB    (0x0001U)
#define WGL_DRAW_TO_WINDOW_ARB              (0x2001U)
#define WGL_ACCELERATION_ARB                (0x2003U)
#define WGL_SUPPORT_OPENGL_ARB              (0x2010U)
#define WGL_DOUBLE_BUFFER_ARB               (0x2011U)
#define WGL_PIXEL_TYPE_ARB                  (0x2013U)
#define WGL_COLOR_BITS_ARB                  (0x2014U)
#define WGL_DEPTH_BITS_ARB                  (0x2022U)
#define WGL_STENCIL_BITS_ARB                (0x2023U)
#define WGL_FULL_ACCELERATION_ARB           (0x2027U)
#define WGL_TYPE_RGBA_ARB                   (0x202BU)

const int pixelFormatAttribs[] = 
{
    WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
    WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
    WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
    WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
    WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
    WGL_COLOR_BITS_ARB,         32,
    WGL_DEPTH_BITS_ARB,         24,
    WGL_STENCIL_BITS_ARB,       8,
    0
};

const int gl33Attribs[] = 
{
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 3,
    WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0,
};

const wchar_t *WINDOW_CLASS_NAME = L"NanoKitWindowClass";

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef HGLRC WINAPI wglCreateContextAttribsARB_t(HDC hdc, HGLRC hShareContext, const int *attribList);
typedef BOOL WINAPI wglChoosePixelFormatARB_t(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

static bool initialized = false;

static wglCreateContextAttribsARB_t *wglCreateContextAttribsARB;
static wglChoosePixelFormatARB_t* wglChoosePixelFormatARB;

static WNDCLASS windowClass;

static nkWindow_t *windowList = NULL;

static HGLRC currentGlrc = NULL;

static uint16_t highUnicodeSurrogate = 0;

static LARGE_INTEGER frequency = {0}; /* for high precision timing */

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static void InitWin32(void);
static bool InitOpenGL();

static LPWSTR CreateWideString(const char* str);

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static uint32_t GetNkKeycodeFromWin32(WPARAM wParam);
static WPARAM GetWin32KeycodeFromNk(WPARAM wParam);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

bool nkWindow_Create(nkWindow_t *window, const char *title, float width, float height)
{   
    /* setup Win32 the first time this is run */
    if (!initialized)
    {
        InitWin32();

        if (!InitOpenGL())
        {
            fprintf(stderr, "Failed to initialize OpenGL");
            return false;
        }

        initialized = true;
    }

    HINSTANCE instance = GetModuleHandle(NULL);

    RECT desiredClientRect;
    desiredClientRect.left = 0;
    desiredClientRect.top = 0;
    desiredClientRect.right = (LONG)width;
    desiredClientRect.bottom = (LONG)height;

    AdjustWindowRect(&desiredClientRect, WS_OVERLAPPEDWINDOW, FALSE);

    /* allocate and poulate a wide string */
    LPWSTR wtitle = CreateWideString(title);

    HWND hwnd = CreateWindowEx(
        0, 
        WINDOW_CLASS_NAME, 
        wtitle, 
        WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, CW_USEDEFAULT, 
        (int)desiredClientRect.right - (int)desiredClientRect.left, 
        (int)desiredClientRect.bottom - (int)desiredClientRect.top,
        NULL, 
        NULL, 
        instance, 
        NULL
    );

    free(wtitle); /* IMPORTANT: free the allocated wstring title */

    if (!hwnd)
    {
        fprintf(stderr, "Failed to create a Win32 Window!\n");
        return false;
    }

    HDC gldc = GetDC(hwnd);

    int pixelFormat;
    UINT numFormats;
    wglChoosePixelFormatARB(gldc, pixelFormatAttribs, 0, 1, &pixelFormat, &numFormats);

    if (!numFormats) 
    {
        fprintf(stderr, "Failed to set the OpenGL 3.3 pixel format.");
        return 0;
    }

    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(gldc, pixelFormat, sizeof(pfd), &pfd);

    if (!SetPixelFormat(gldc, pixelFormat, &pfd)) 
    {
        fprintf(stderr, "Failed to set the OpenGL 3.3 pixel format.");
        return 0;
    }

    HGLRC glrc = wglCreateContextAttribsARB(gldc, 0, gl33Attribs);
    if (!glrc) 
    {
        fprintf(stderr, "Failed to create OpenGL 3.3 context.");
        return 0;
    }

    if (!wglMakeCurrent(gldc, glrc)) 
    {
        fprintf(stderr, "Failed to activate OpenGL 3.3 rendering context.");
        return 0;
    }

    currentGlrc = glrc;

    if (!gladLoadGL())
    {
        fprintf(stderr, "Failed to initialize GLAD for OpenGL 3.3.");
        return false;
    }

    nkDraw_CreateContext(&window->drawContext);

    ShowWindow(hwnd, SW_SHOW);

    /* populate the window contents */
    window->next = NULL;
    window->title = title;
    window->width = width;
    window->height = height;
    window->visibility = NK_WINDOW_VISIBILITY_VISIBLE;
    window->focus = NK_WINDOW_FOCUS_FOCUSED;
    window->windowHandle = hwnd;
    window->instanceHandle = instance;
    window->drawingContext = gldc;
    window->glRenderContext = glrc;
    window->cursorType = (uintptr_t)IDC_ARROW; /* default cursor type */
    window->backgroundColor = NK_COLOR_WHITE; /* default background color */

    /* add this window to the linked list */
    if (windowList == NULL)
    {
        windowList = window;
    }
    else
    {
        nkWindow_t *current = windowList;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = window;
    }

    return true;
}

void nkWindow_SetTitle(nkWindow_t *window, const char *title)
{
    if (window == NULL || title == NULL)
    {
        return; /* nothing to do */
    }

    /* allocate and poulate a wide string */
    LPWSTR wtitle = CreateWideString(title);

    /* set the window title */
    SetWindowText(window->windowHandle, wtitle);

    free(wtitle); /* IMPORTANT: free the allocated wstring title */

    window->title = title; /* update the title in the window struct */
}

void nkWindow_SetSize(nkWindow_t *window, float width, float height)
{
    if (window == NULL)
    {
        return; /* nothing to do */
    }

    /* set the window size */
    SetWindowPos(
        window->windowHandle, 
        NULL, 
        0, 0, 
        (int)width, (int)height, 
        SWP_NOZORDER | SWP_NOMOVE
    );

    /* update the width and height in the window struct */
    window->width = width;
    window->height = height;

    /* call the resize callback if it exists */
    if (window->resizeCallback)
    {
        window->resizeCallback(window, width, height);
    }
}

void nkWindow_SetVisibility(nkWindow_t *window, nkWindowVisibility_t visibility)
{
    if (window == NULL)
    {
        return; /* nothing to do */
    }

    /* set the window visibility */
    switch (visibility)
    {
        case NK_WINDOW_VISIBILITY_VISIBLE:
        {
            ShowWindow(window->windowHandle, SW_SHOW);
        } break;

        case NK_WINDOW_VISIBILITY_HIDDEN:
        {
            ShowWindow(window->windowHandle, SW_HIDE);
        } break;

        case NK_WINDOW_VISIBILITY_MINIMIZED:
        {
            ShowWindow(window->windowHandle, SW_MINIMIZE);
        } break;

        case NK_WINDOW_VISIBILITY_MAXIMIZED:
        {
            ShowWindow(window->windowHandle, SW_MAXIMIZE);
        } break;

        case NK_WINDOW_VISIBILITY_FULLSCREEN:
        {
            ShowWindow(window->windowHandle, SW_SHOWMAXIMIZED);
        } break;

        default:
        {
            return; /* nothing to do */
        }
    }

    window->visibility = visibility; /* update the visibility in the window struct */
}

void nkWindow_SetFocus(nkWindow_t *window, nkWindowFocus_t focus)
{
    if (window == NULL)
    {
        return; /* nothing to do */
    }

    /* set the window focus */
    switch (focus)
    {
        case NK_WINDOW_FOCUS_FOCUSED:
        {
            SetFocus(window->windowHandle);
        } break;

        default:
        {
            /* unfocus not available in Win32 */
            return;
        } break;
    }

    window->focus = focus; /* update the focus in the window struct */
}

void nkWindow_SetCursor(nkWindow_t *window, nkCursorType_t cursorType)
{
    if (window == NULL)
    {
        return; /* nothing to do */
    }

    /* set the window cursor */
    switch (cursorType)
    {
        case NK_CURSOR_ARROW:
        case NK_CURSOR_IBEAM:
        case NK_CURSOR_HAND:
        case NK_CURSOR_CROSSHAIR:
        case NK_CURSOR_SIZEALL:
        case NK_CURSOR_SIZENWSE:
        case NK_CURSOR_SIZENESW:
        case NK_CURSOR_SIZEWE:
        case NK_CURSOR_SIZENS:
        {
            window->cursorType = cursorType;
        } break;

        default:
        {
            
        } break;
    }
}

void nkWindow_Destroy(nkWindow_t *window)
{
    if (window == NULL)
    {
        return; /* nothing to do */
    }

    /* call the close callback if it exists */
    if (window->closeCallback)
    {
        window->closeCallback(window);
    }

    /* destroy the window */
    DestroyWindow(window->windowHandle);
}

void nkWindow_RequestRedraw(nkWindow_t *window)
{
    if (window == NULL)
    {
        return; /* nothing to do */
    }

    /* request a redraw by invalidating the window */
    InvalidateRect(window->windowHandle, NULL, TRUE);
}

bool nkWindow_IsPointerActionDown(nkWindow_t *window, nkPointerAction_t action)
{
    switch (action)
    {
        case NK_POINTER_ACTION_PRIMARY:
        {
            return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0; /* check if the left mouse button is down */
        } break;

        case NK_POINTER_ACTION_SECONDARY:
        {
            return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0; /* check if the right mouse button is down */
        } break;

        case NK_POINTER_ACTION_TERTIARY:
        {
            return (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0; /* check if the middle mouse button is down */
        } break;

        case NK_POINTER_ACTION_EXTENDED_1:
        {
            return (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0; /* check if the first extended button is down */
        } break;

        case NK_POINTER_ACTION_EXTENDED_2:
        {
            return (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0; /* check if the second extended button is down */
        } break;

        default:
        {
            return false; /* invalid action */
        } break;
    }
}

bool nkWindow_IsKeyDown(nkWindow_t *window, uint32_t keycode)
{
    WPARAM wParam = GetWin32KeycodeFromNk(keycode);
    if (wParam != 0)
    {
        return (GetAsyncKeyState((int)wParam) & 0x8000) != 0; /* check if the key is down */
    }
    else
    {
        return false; /* invalid keycode */
    }
}

void nkWindow_RedrawViews(nkWindow_t *window)
{
    if (window == NULL || window->rootView == NULL)
    {
        printf("Window contains no views!\n");
        return;
    }

    nkView_RenderTree(window->rootView, &window->drawContext);
}

void nkWindow_LayoutViews(nkWindow_t *window)
{
    if (window == NULL || window->rootView == NULL)
    {
        printf("Window contains no views!\n");
        return;
    }

    nkView_LayoutTree(window->rootView, (nkSize_t){window->width, window->height});
}

bool nkWindow_PollEvents(void)
{
    
    static bool firstRun = true;

    if (firstRun)
    {
        firstRun = false;

        nkWindow_t *current = windowList;

        while (current != NULL)
        {
            nkWindow_LayoutViews(current);
            nkWindow_RedrawViews(current);
            current = current->next;
        }
    }

    
    MSG msg;
    
    /* reset the high surrogate for the next event loop */
    highUnicodeSurrogate = 0;
    
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return false;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return true;
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

static LPWSTR CreateWideString(const char* str)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    LPWSTR wstr = malloc(size * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, size);
    return wstr;
}

static void InitWin32()
{
    SetConsoleOutputCP(CP_UTF8);

    QueryPerformanceFrequency(&frequency); /* get the frequency for high precision timing */

    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = GetModuleHandle(0);
    windowClass.lpszClassName = WINDOW_CLASS_NAME;
    windowClass.hCursor = NULL;

    RegisterClassW(&windowClass);
}


static bool InitOpenGL()
{
    /* create temporary window and window class to chose window pixel format */

    WNDCLASS tempWindowClass = {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = DefWindowProc,
        .hInstance = GetModuleHandle(0),
        .lpszClassName = L"TempWindowClass",
        .hCursor = NULL
    };

    if (!RegisterClassW(&tempWindowClass)) 
    {
        fprintf(stderr, "Failed to register window class");
        return false;
    }

    HWND tempWindow = CreateWindowEx(
        0,
        tempWindowClass.lpszClassName,
        L"TempWindow",
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        windowClass.hInstance,
        0
    );

    if (!tempWindow) 
    {
        fprintf(stderr, "Failed to create temp window");
        return false;
    }

    HDC tempDc = GetDC(tempWindow);

    PIXELFORMATDESCRIPTOR pfd = {
        .nSize = sizeof(pfd),
        .nVersion = 1,
        .iPixelType = PFD_TYPE_RGBA,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .cColorBits = 32,
        .cAlphaBits = 8,
        .iLayerType = PFD_MAIN_PLANE,
        .cDepthBits = 24,
        .cStencilBits = 8,
    };

    int pixelFormat = ChoosePixelFormat(tempDc, &pfd);

    if (!pixelFormat) 
    {
        fprintf(stderr, "Failed to find a suitable pixel format.");
        return false;
    }

    if (!SetPixelFormat(tempDc, pixelFormat, &pfd)) 
    {
        fprintf(stderr, "Failed to set the pixel format.");
        return false;
    }

    HGLRC tempContext = wglCreateContext(tempDc);

    if (!tempContext) 
    {
        fprintf(stderr, "Failed to create a dummy OpenGL rendering context.");
        return false;
    }

    if (!wglMakeCurrent(tempDc, tempContext)) 
    {
        fprintf(stderr, "Failed to activate dummy OpenGL rendering context.");
        return false;
    }

    wglCreateContextAttribsARB = (wglCreateContextAttribsARB_t*)wglGetProcAddress("wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (wglChoosePixelFormatARB_t*)wglGetProcAddress("wglChoosePixelFormatARB");

    wglMakeCurrent(tempDc, 0);
    wglDeleteContext(tempContext);
    ReleaseDC(tempWindow, tempDc);
    DestroyWindow(tempWindow);

    return true;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    /* first, try and find this window */
    nkWindow_t *window = NULL;
    for (nkWindow_t *current = windowList; current != NULL; current = current->next)
    {
        if (current->windowHandle == hwnd)
        {
            window = current;
            break;
        }
    }

    if (window == NULL)
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    /* now, process the event if the window was found */
    switch (uMsg)
    {
        case WM_CHAR:
        {
            uint16_t u16Codepoint = (uint16_t)wParam;

            if (IS_HIGH_SURROGATE(u16Codepoint))
            {
                /* store the high surrogate */
                highUnicodeSurrogate = u16Codepoint;
            }
            else if (IS_LOW_SURROGATE(u16Codepoint))
            {
                /* check if we have a high surrogate stored */
                if (highUnicodeSurrogate != 0)
                {
                    /* combine the high and low surrogate to form a full codepoint */
                    uint32_t codepoint = ((highUnicodeSurrogate - 0xD800) << 10) | (u16Codepoint - 0xDC00);
                    codepoint += 0x10000; /* adjust to full Unicode codepoint range */

                    if (window->codepointInputCallback)
                    {
                        window->codepointInputCallback(window, codepoint);
                    }

                    highUnicodeSurrogate = 0; /* reset the high surrogate */
                }
            }
            else
            {
                /* handle as a single codepoint, filtering out control keys such as delete and backspace */
                if (window->codepointInputCallback && u16Codepoint > 0x1F)
                {
                    window->codepointInputCallback(window, u16Codepoint);
                }
            }
        } break;

        case WM_SIZE:
        {
            nkWindowVisibility_t prevVisibility = window->visibility;

            switch (wParam)
            {
                case SIZE_MINIMIZED:
                {
                    window->visibility = NK_WINDOW_VISIBILITY_MINIMIZED;
                } break;

                case SIZE_MAXIMIZED:
                {
                    window->visibility = NK_WINDOW_VISIBILITY_MAXIMIZED;
                } break;
                    
                case SIZE_RESTORED:
                {
                    window->visibility = NK_WINDOW_VISIBILITY_VISIBLE;
                } break;

                default:
                {
                    /* do nothing */
                } break;
            }

            if (window->visibilityChangeCallback && window->visibility != prevVisibility)
            {
                window->visibilityChangeCallback(window, window->visibility);
            }

            float width = LOWORD(lParam);
            float height = HIWORD(lParam);

            window->width = width;
            window->height = height;

            if (window->resizeCallback)
            {
                window->resizeCallback(window, width, height);
            }
        } break;

        case WM_DESTROY:
        {
            if (window->closeCallback)
            {
                window->closeCallback(window);
            }

            if (windowList == window && window->next == NULL)
            {
                PostQuitMessage(0); /* quit if this is the last */
            }
            else if (windowList == window)
            {
                windowList = window->next; /* remove from head */
            }
            else
            {
                nkWindow_t *prev = windowList;
                while (prev->next != window && prev->next != NULL)
                {
                    prev = prev->next;
                }

                if (prev->next == window)
                {
                    prev->next = window->next; /* remove from middle or end */
                }
            }
        } break;

        case WM_PAINT:
        {
            if (currentGlrc != window->glRenderContext)
            {
                wglMakeCurrent(window->drawingContext, window->glRenderContext);
                currentGlrc = window->glRenderContext;
            }

            BeginPaint(hwnd, &window->paintStruct);

            glClearColor(
                window->backgroundColor.r, 
                window->backgroundColor.g, 
                window->backgroundColor.b, 
                window->backgroundColor.a
            );
            glClear(GL_COLOR_BUFFER_BIT);

            glViewport(0, 0, (int)window->width, (int)window->height);

            if (window->drawCallback)
            {
                window->drawCallback(window);
            }

            SwapBuffers(window->drawingContext);
            EndPaint(hwnd, &window->paintStruct);
            
        } break;    
        
        case WM_SETCURSOR:
        {
            if (LOWORD(lParam) == HTCLIENT)
            {
                
                SetCursor(LoadCursor(NULL, (LPCTSTR)window->cursorType));
                return TRUE; /* indicate that we handled the cursor */
            }
        } break;

        case WM_ACTIVATE:
        {
            if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
            {
                if (window->focusChangeCallback)
                {
                    window->focusChangeCallback(window, NK_WINDOW_FOCUS_FOCUSED);
                }
            }
            else
            {
                if (window->focusChangeCallback)
                {
                    window->focusChangeCallback(window, NK_WINDOW_FOCUS_UNFOCUSED);
                }
            }
        } break;

        case WM_MOUSEMOVE:
        {
            float x = (float)GET_X_LPARAM(lParam);
            float y = (float)GET_Y_LPARAM(lParam);

            /* Request WM_MOUSELEAVE */
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);

            if (window->pointerMoveCallback)
            {
                window->pointerMoveCallback(window, x, y);
            }

            nkView_ProcessPointerMovement(window->rootView, x, y, &window->hotView, window->activeView, window->activeAction);

        } break;

        case WM_MOUSELEAVE:
        {   
            
            /* set origin to -1, -1 */
            nkView_ProcessPointerAction(
                window->rootView, 
                window->activeAction, 
                POINTER_EVENT_CANCEL,
                -1.0f, 
                -1.0f,
                window->hotView, 
                &window->activeView, 
                &window->activeAction
            );

            nkView_ProcessPointerMovement(window->rootView, -1.0f, -1.0f, &window->hotView, window->activeView, window->activeAction);
        
            nkWindow_RequestRedraw(window);

        } break;

        case WM_LBUTTONDOWN:
        {
            if (window->pointerActionBeginCallback)
            {
                window->pointerActionBeginCallback(window, NK_POINTER_ACTION_PRIMARY, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
            }

            nkView_ProcessPointerAction(
                window->rootView, 
                NK_POINTER_ACTION_PRIMARY, 
                POINTER_EVENT_BEGIN,
                (float)GET_X_LPARAM(lParam), 
                (float)GET_Y_LPARAM(lParam), 
                window->hotView, 
                &window->activeView, 
                &window->activeAction
            );

            nkWindow_RequestRedraw(window);

        } break;

        case WM_LBUTTONUP:
        {
            if (window->pointerActionEndCallback)
            {
                window->pointerActionEndCallback(window, NK_POINTER_ACTION_PRIMARY, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
            }

            nkView_ProcessPointerAction(
                window->rootView, 
                NK_POINTER_ACTION_PRIMARY, 
                POINTER_EVENT_END,
                (float)GET_X_LPARAM(lParam), 
                (float)GET_Y_LPARAM(lParam), 
                window->hotView, 
                &window->activeView, 
                &window->activeAction
            );

            nkWindow_RequestRedraw(window);
            
        } break;

        case WM_RBUTTONDOWN:
        {
            if (window->pointerActionBeginCallback)
            {
                window->pointerActionBeginCallback(window, NK_POINTER_ACTION_SECONDARY, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
            }

            nkView_ProcessPointerAction(
                window->rootView, 
                NK_POINTER_ACTION_SECONDARY, 
                POINTER_EVENT_BEGIN,
                (float)GET_X_LPARAM(lParam), 
                (float)GET_Y_LPARAM(lParam), 
                window->hotView, 
                &window->activeView, 
                &window->activeAction
            );

            nkWindow_RequestRedraw(window);

        } break;

        case WM_RBUTTONUP:
        {
            if (window->pointerActionEndCallback)
            {
                window->pointerActionEndCallback(window, NK_POINTER_ACTION_SECONDARY, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
            }

            nkView_ProcessPointerAction(
                window->rootView, 
                NK_POINTER_ACTION_SECONDARY, 
                POINTER_EVENT_END,
                (float)GET_X_LPARAM(lParam), 
                (float)GET_Y_LPARAM(lParam), 
                window->hotView, 
                &window->activeView, 
                &window->activeAction
            );

            nkWindow_RequestRedraw(window);

        } break;

        case WM_MBUTTONDOWN:
        {
            if (window->pointerActionBeginCallback)
            {
                window->pointerActionBeginCallback(window, NK_POINTER_ACTION_TERTIARY, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
            }

            nkView_ProcessPointerAction(
                window->rootView, 
                NK_POINTER_ACTION_TERTIARY, 
                POINTER_EVENT_BEGIN,
                (float)GET_X_LPARAM(lParam), 
                (float)GET_Y_LPARAM(lParam), 
                window->hotView, 
                &window->activeView, 
                &window->activeAction
            );

            nkWindow_RequestRedraw(window);

        } break;

        case WM_MBUTTONUP:
        {
            if (window->pointerActionEndCallback)
            {
                window->pointerActionEndCallback(window, NK_POINTER_ACTION_TERTIARY, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
            }

            nkView_ProcessPointerAction(
                window->rootView, 
                NK_POINTER_ACTION_TERTIARY, 
                POINTER_EVENT_END,
                (float)GET_X_LPARAM(lParam), 
                (float)GET_Y_LPARAM(lParam), 
                window->hotView, 
                &window->activeView, 
                &window->activeAction
            );

            nkWindow_RequestRedraw(window);

        } break;

        case WM_XBUTTONDOWN:
        {
            if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
            {
                if (window->pointerActionBeginCallback)
                {
                    window->pointerActionBeginCallback(window, NK_POINTER_ACTION_EXTENDED_1, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
                }

                nkView_ProcessPointerAction(
                    window->rootView, 
                    NK_POINTER_ACTION_EXTENDED_1, 
                    POINTER_EVENT_BEGIN,
                    (float)GET_X_LPARAM(lParam), 
                    (float)GET_Y_LPARAM(lParam), 
                    window->hotView, 
                    &window->activeView, 
                    &window->activeAction
                );

                nkWindow_RequestRedraw(window);



            }
            else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2)
            {
                if (window->pointerActionBeginCallback)
                {
                    window->pointerActionBeginCallback(window, NK_POINTER_ACTION_EXTENDED_2, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
                }

                 nkView_ProcessPointerAction(
                    window->rootView, 
                    NK_POINTER_ACTION_EXTENDED_2, 
                    POINTER_EVENT_BEGIN,
                    (float)GET_X_LPARAM(lParam), 
                    (float)GET_Y_LPARAM(lParam), 
                    window->hotView, 
                    &window->activeView, 
                    &window->activeAction
                );

                nkWindow_RequestRedraw(window);
            }
        } break;

        case WM_XBUTTONUP:
        {
            if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
            {
                if (window->pointerActionEndCallback)
                {
                    window->pointerActionEndCallback(window, NK_POINTER_ACTION_EXTENDED_1, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
                }

                nkView_ProcessPointerAction(
                    window->rootView, 
                    NK_POINTER_ACTION_EXTENDED_1, 
                    POINTER_EVENT_END,
                    (float)GET_X_LPARAM(lParam), 
                    (float)GET_Y_LPARAM(lParam), 
                    window->hotView, 
                    &window->activeView, 
                    &window->activeAction
                );

                nkWindow_RequestRedraw(window);
            }
            else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2)
            {
                if (window->pointerActionEndCallback)
                {
                    window->pointerActionEndCallback(window, NK_POINTER_ACTION_EXTENDED_2, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
                }

                nkView_ProcessPointerAction(
                    window->rootView, 
                    NK_POINTER_ACTION_EXTENDED_2, 
                    POINTER_EVENT_END,
                    (float)GET_X_LPARAM(lParam), 
                    (float)GET_Y_LPARAM(lParam), 
                    window->hotView, 
                    &window->activeView, 
                    &window->activeAction
                );

                nkWindow_RequestRedraw(window);
            }
        } break;

        case WM_MOUSEWHEEL:
        {
            float deltaX = 0.0f;
            float deltaY = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

            if (window->scrollCallback)
            {
                window->scrollCallback(window, deltaX, deltaY);
            }
        } break;

        case WM_KEYDOWN:
        {
            uint32_t keycode = GetNkKeycodeFromWin32(wParam);
            if (window->keyDownCallback)
            {
                window->keyDownCallback(window, keycode);
            }
        } break;

        case WM_KEYUP:
        {
            uint32_t keycode = GetNkKeycodeFromWin32(wParam);
            if (window->keyUpCallback)
            {
                window->keyUpCallback(window, keycode);
            }
        } break;
        
        default: 
        {
            /* do nothing */
        } break;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);  
}

static uint32_t GetNkKeycodeFromWin32(WPARAM wParam)
{
    switch (wParam)
    {
        case VK_SPACE: return NK_KEYCODE_SPACE;
        case VK_BACK: return NK_KEYCODE_BACKSPACE;
        case VK_TAB: return NK_KEYCODE_TAB;
        case VK_CLEAR: return NK_KEYCODE_CLEAR;
        case VK_RETURN: return NK_KEYCODE_RETURN;
        case VK_PAUSE: return NK_KEYCODE_PAUSE;
        case VK_ESCAPE: return NK_KEYCODE_ESCAPE;
        case VK_DELETE: return NK_KEYCODE_DELETE;

        case VK_SHIFT: return NK_KEYCODE_SHIFT;
        case VK_CONTROL: return NK_KEYCODE_CONTROL;
        case VK_LWIN: return NK_KEYCODE_META;
        case VK_RWIN: return NK_KEYCODE_META;
        case VK_MENU: return NK_KEYCODE_ALT;

        case VK_PRIOR: return NK_KEYCODE_PAGE_UP;
        case VK_NEXT: return NK_KEYCODE_PAGE_DOWN;
        case VK_END: return NK_KEYCODE_END;
        case VK_HOME: return NK_KEYCODE_HOME;
        case VK_LEFT: return NK_KEYCODE_LEFT;
        case VK_UP: return NK_KEYCODE_UP;
        case VK_RIGHT: return NK_KEYCODE_RIGHT;
        case VK_DOWN: return NK_KEYCODE_DOWN;

        case VK_SELECT: return NK_KEYCODE_SELECT;
        case VK_PRINT: return NK_KEYCODE_PRINT;
        case VK_EXECUTE: return NK_KEYCODE_EXECUTE;
        case VK_INSERT: return NK_KEYCODE_INSERT;
        case VK_HELP: return NK_KEYCODE_HELP;

        case VK_F1: return NK_KEYCODE_F1;
        case VK_F2: return NK_KEYCODE_F2;
        case VK_F3: return NK_KEYCODE_F3;
        case VK_F4: return NK_KEYCODE_F4;
        case VK_F5: return NK_KEYCODE_F5;
        case VK_F6: return NK_KEYCODE_F6;
        case VK_F7: return NK_KEYCODE_F7;
        case VK_F8: return NK_KEYCODE_F8;
        case VK_F9: return NK_KEYCODE_F9;
        case VK_F10: return NK_KEYCODE_F10;
        case VK_F11: return NK_KEYCODE_F11;
        case VK_F12: return NK_KEYCODE_F12;
        case VK_F13: return NK_KEYCODE_F13;
        case VK_F14: return NK_KEYCODE_F14;
        case VK_F15: return NK_KEYCODE_F15;
        case VK_F16: return NK_KEYCODE_F16;
        case VK_F17: return NK_KEYCODE_F17;
        case VK_F18: return NK_KEYCODE_F18;
        case VK_F19: return NK_KEYCODE_F19;
        case VK_F20: return NK_KEYCODE_F20;
        case VK_F21: return NK_KEYCODE_F21;
        case VK_F22: return NK_KEYCODE_F22;
        case VK_F23: return NK_KEYCODE_F23;
        case VK_F24: return NK_KEYCODE_F24;

        default:
        {
            return 0;
        } break;
    }
}

static WPARAM GetWin32KeycodeFromNk(WPARAM wParam)
{
    switch (wParam)
    {
        case NK_KEYCODE_SPACE: return VK_SPACE;
        case NK_KEYCODE_BACKSPACE: return VK_BACK;
        case NK_KEYCODE_TAB: return VK_TAB;
        case NK_KEYCODE_CLEAR: return VK_CLEAR;
        case NK_KEYCODE_RETURN: return VK_RETURN;
        case NK_KEYCODE_PAUSE: return VK_PAUSE;
        case NK_KEYCODE_ESCAPE: return VK_ESCAPE;
        case NK_KEYCODE_DELETE: return VK_DELETE;

        case NK_KEYCODE_SHIFT: return VK_SHIFT;
        case NK_KEYCODE_CONTROL: return VK_CONTROL;
        case NK_KEYCODE_META: return VK_LWIN;
        case NK_KEYCODE_ALT: return VK_MENU;

        case NK_KEYCODE_PAGE_UP: return VK_PRIOR;
        case NK_KEYCODE_PAGE_DOWN: return VK_NEXT;
        case NK_KEYCODE_END: return VK_END;
        case NK_KEYCODE_HOME: return VK_HOME;
        case NK_KEYCODE_LEFT: return VK_LEFT;
        case NK_KEYCODE_UP: return VK_UP;
        case NK_KEYCODE_RIGHT: return VK_RIGHT;
        case NK_KEYCODE_DOWN: return VK_DOWN;

        case NK_KEYCODE_SELECT: return VK_SELECT;
        case NK_KEYCODE_PRINT: return VK_PRINT;
        case NK_KEYCODE_EXECUTE: return VK_EXECUTE;
        case NK_KEYCODE_INSERT: return VK_INSERT;
        case NK_KEYCODE_HELP: return VK_HELP;

        case NK_KEYCODE_F1: return VK_F1;
        case NK_KEYCODE_F2: return VK_F2;
        case NK_KEYCODE_F3: return VK_F3;
        case NK_KEYCODE_F4: return VK_F4;
        case NK_KEYCODE_F5: return VK_F5;
        case NK_KEYCODE_F6: return VK_F6;
        case NK_KEYCODE_F7: return VK_F7;
        case NK_KEYCODE_F8: return VK_F8;
        case NK_KEYCODE_F9: return VK_F9;
        case NK_KEYCODE_F10: return VK_F10;
        case NK_KEYCODE_F11: return VK_F11;
        case NK_KEYCODE_F12: return VK_F12;
        case NK_KEYCODE_F13: return VK_F13;
        case NK_KEYCODE_F14: return VK_F14;
        case NK_KEYCODE_F15: return VK_F15;
        case NK_KEYCODE_F16: return VK_F16;
        case NK_KEYCODE_F17: return VK_F17;
        case NK_KEYCODE_F18: return VK_F18;
        case NK_KEYCODE_F19: return VK_F19;
        case NK_KEYCODE_F20: return VK_F20;
        case NK_KEYCODE_F21: return VK_F21;
        case NK_KEYCODE_F22: return VK_F22;
        case NK_KEYCODE_F23: return VK_F23;
        case NK_KEYCODE_F24: return VK_F24;

        default:
        {
            return 0; /* unknown keycode */
        } break;
    }
}