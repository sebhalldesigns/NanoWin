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

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

static bool initialized = false;

static nkWindow_t *windowHandle = NULL;

static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webglContext;
static EmscriptenWebGLContextAttributes webglAttributes;
static EmscriptenMouseEvent mouseEvent;
static EmscriptenKeyboardEvent keyboardEvent;

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static void InitWeb(void);


static EM_BOOL MouseCallback(int eventType, const EmscriptenMouseEvent* e, void* userData);
static EM_BOOL TouchCallback(int eventType, const EmscriptenTouchEvent* e, void* userData);
static EM_BOOL KeyCallback(int eventType, const EmscriptenKeyboardEvent* e, void* userData);
static EM_BOOL ResizeCallback(int eventType, const EmscriptenUiEvent* e, void* userData);
static EM_BOOL DrawCallback(double time, void* userData);

static void MeasureWindow(nkWindow_t *window);
static void ArrangeWindow(nkWindow_t *window);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

bool nkWindow_Create(nkWindow_t *window, const char *title, float width, float height)
{   
    /* setup Web the first time this is run */
    if (!initialized)
    {
        InitWeb();

        initialized = true;
    }

    printf("Creating window '%s' with size %.2f x %.2f\n", title, width, height);

    nkDraw_CreateContext(&window->drawContext);


    windowHandle = window;

    /* populate the window contents */
    window->next = NULL;
    window->title = title;
    window->width = width;
    window->height = height;
    window->visibility = NK_WINDOW_VISIBILITY_VISIBLE;
    window->focus = NK_WINDOW_FOCUS_FOCUSED;
    window->backgroundColor = NK_COLOR_WHITE; /* default background color */

    return true;
}

void nkWindow_SetTitle(nkWindow_t *window, const char *title)
{
    if (window == NULL || title == NULL)
    {
        return; /* nothing to do */
    }

}

void nkWindow_SetSize(nkWindow_t *window, float width, float height)
{
    if (window == NULL)
    {
        return; /* nothing to do */
    }

}

void nkWindow_SetVisibility(nkWindow_t *window, nkWindowVisibility_t visibility)
{
    if (window == NULL)
    {
        return; /* nothing to do */
    }

}

void nkWindow_SetFocus(nkWindow_t *window, nkWindowFocus_t focus)
{
    if (window == NULL)
    {
        return; /* nothing to do */
    }


}

void nkWindow_SetCursor(nkWindow_t *window, nkCursorType_t cursorType)
{
    if (window == NULL)
    {
        return; /* nothing to do */
    }

    
}

void nkWindow_Destroy(nkWindow_t *window)
{
    if (window == NULL)
    {
        return; /* nothing to do */
    }

}

void nkWindow_RequestRedraw(nkWindow_t *window)
{
    if (window == NULL)
    {
        return; /* nothing to do */
    }

    printf("Requesting redraw for window '%s'\n", window->title);
    emscripten_request_animation_frame(DrawCallback, window);
}

bool nkWindow_IsPointerActionDown(nkWindow_t *window, nkPointerAction_t action)
{
    return false;
}

bool nkWindow_IsKeyDown(nkWindow_t *window, uint32_t keycode)
{
    return false;
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
    ResizeCallback(0, NULL, NULL); // Trigger resize to ensure window size is updated
    return false;
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

static void InitWeb(void)
{
    /* create a canvas */
    printf("Initializing Web backend...\n");


    emscripten_webgl_init_context_attributes(&webglAttributes);
    webglAttributes.majorVersion = 2;
    webglAttributes.minorVersion = 0;
    webglAttributes.enableExtensionsByDefault = true;
    //webglAttributes.explicitSwapControl = 0; // Let browser handle it
    //webglAttributes.renderViaOffscreenBackBuffer = 0; // Avoid unnecessary buffering

    webglContext = emscripten_webgl_create_context("#canvas", &webglAttributes);
    if (webglContext <= 0)
    {
        fprintf(stderr, "Failed to create WebGL context!\n");
        return;
    }
    else 
    {
        printf("WebGL context created successfully.\n");
    }

    emscripten_webgl_make_context_current(webglContext);

    emscripten_set_mousemove_callback("#canvas", NULL, false, MouseCallback);
    emscripten_set_mousedown_callback("#canvas", NULL, false, MouseCallback);
    emscripten_set_mouseup_callback("#canvas", NULL, false, MouseCallback);
    emscripten_set_touchstart_callback("#canvas", NULL, false, TouchCallback);
    emscripten_set_touchend_callback("#canvas", NULL, false, TouchCallback);
    emscripten_set_touchmove_callback("#canvas", NULL, false, TouchCallback);
    emscripten_set_keydown_callback("#canvas", NULL, false, KeyCallback);
    emscripten_set_keyup_callback("#canvas", NULL, false, KeyCallback);
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, false, ResizeCallback);
}

static EM_BOOL MouseCallback(int eventType, const EmscriptenMouseEvent* e, void* userData)
{
    printf("Mouse event: %d at (%d, %d)\n", eventType, e->targetX, e->targetY);
    if (windowHandle == NULL)
    {
        return false; /* no window to handle events for */
    }

    nkWindow_t *window = windowHandle;

    float x = (float)e->targetX;
    float y = (float)e->targetY;

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
        {
            
            if (window->pointerActionBeginCallback)
            {
                window->pointerActionBeginCallback(window, NK_POINTER_ACTION_PRIMARY, (float)e->targetX, (float)e->targetY);
            }

            nkView_ProcessPointerAction(
                window->rootView, 
                NK_POINTER_ACTION_PRIMARY, 
                POINTER_EVENT_BEGIN,
                x, 
                y, 
                window->hotView, 
                &window->activeView, 
                &window->activeAction
            );

        } break;

        case EMSCRIPTEN_EVENT_MOUSEUP:
        {
            if (window->pointerActionEndCallback)
            {
                window->pointerActionEndCallback(window, NK_POINTER_ACTION_PRIMARY, (float)e->targetX, (float)e->targetY);
            }

            nkView_ProcessPointerAction(
                window->rootView, 
                NK_POINTER_ACTION_PRIMARY, 
                POINTER_EVENT_END,
                x, 
                y, 
                window->hotView, 
                &window->activeView, 
                &window->activeAction
            );

        } break;

        case EMSCRIPTEN_EVENT_MOUSEMOVE:
        {
            if (window->pointerMoveCallback)
            {
                window->pointerMoveCallback(window, x, y);
            }

            nkView_ProcessPointerMovement(window->rootView, x, y, &window->hotView, window->activeView, window->activeAction);

        } break;

        default:
        {
            return false; /* unhandled event */
        } break; 
            
    }

    return true;
}

static EM_BOOL TouchCallback(int eventType, const EmscriptenTouchEvent* e, void* userData)
{
    printf("Touch event: %d at (%d, %d)\n", eventType, e->touches[0].targetX, e->touches[0].targetY);
    if (windowHandle == NULL)
    {
        return false; /* no window to handle events for */
    }

    nkWindow_t *window = windowHandle;

    
    float x = (float)e->touches[0].targetX;
    float y = (float)e->touches[0].targetY;

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_TOUCHSTART:
        {
            if (window->pointerActionBeginCallback)
            {
                window->pointerActionBeginCallback(window, NK_POINTER_ACTION_PRIMARY, (float)e->touches[0].targetX, (float)e->touches[0].targetY);
            }

            nkView_ProcessPointerAction(
                window->rootView, 
                NK_POINTER_ACTION_PRIMARY, 
                POINTER_EVENT_BEGIN,
                x, 
                y, 
                window->hotView, 
                &window->activeView, 
                &window->activeAction
            );

        } break;

        case EMSCRIPTEN_EVENT_TOUCHEND:
        {
            if (window->pointerActionEndCallback)
            {
                window->pointerActionEndCallback(window, NK_POINTER_ACTION_PRIMARY, (float)e->touches[0].targetX, (float)e->touches[0].targetY);
            }

            nkView_ProcessPointerAction(
                window->rootView, 
                NK_POINTER_ACTION_PRIMARY, 
                POINTER_EVENT_END,
                x, 
                y, 
                window->hotView, 
                &window->activeView, 
                &window->activeAction
            );


        } break;

        case EMSCRIPTEN_EVENT_TOUCHMOVE:
        {
            if (window->pointerMoveCallback)
            {
                window->pointerMoveCallback(window, (float)e->touches[0].targetX, (float)e->touches[0].targetY);
            }

            nkView_ProcessPointerMovement(window->rootView, x, y, &window->hotView, window->activeView, window->activeAction);

        } break;

        default:
        {
            return false; /* unhandled event */
        } break; 
            
    }

    return true;
}

static EM_BOOL KeyCallback(int eventType, const EmscriptenKeyboardEvent* e, void* userData)
{

}

static EM_BOOL ResizeCallback(int eventType, const EmscriptenUiEvent* e, void* userData)
{


    if (windowHandle == NULL)
    {
        return false; /* no window to handle events for */
    }

    float pixelRatio = (float)emscripten_get_device_pixel_ratio();

    /* broken so set to 1.0f */
    pixelRatio = 1.0f;

    float cssWidth = (float)EM_ASM_DOUBLE({ return window.innerWidth; });
    float cssHeight = (float)EM_ASM_DOUBLE({ return window.innerHeight; });

    float width = (cssWidth * pixelRatio);
    float height = (cssHeight * pixelRatio);

    windowHandle->width = (float)width;
    windowHandle->height = (float)height;

    if (windowHandle->resizeCallback)
    {
        windowHandle->resizeCallback(windowHandle, width, height);
    }    
    
    emscripten_request_animation_frame(DrawCallback, windowHandle);

    return true;
}

static EM_BOOL DrawCallback(double time, void* userData)
{
    nkWindow_t *window = (nkWindow_t *)userData;

    if (window == NULL)
    {
        return false; /* nothing to render */
    }
    
    int canvasWidth;
    int canvasHeight;

    /* Get the actual current size of the canvas's underlying drawing buffer. */
    emscripten_get_canvas_element_size("#canvas", &canvasWidth, &canvasHeight);

    /* Check if it matches the desired size from our window state. */
    if (canvasWidth != (int)window->width || canvasHeight != (int)window->height)
    {
        /* If not, resize the canvas drawing buffer now, before we draw. */
        emscripten_set_canvas_element_size("#canvas", (int)window->width, (int)window->height);
    }

    glClearColor(
        window->backgroundColor.r, 
        window->backgroundColor.g, 
        window->backgroundColor.b, 
        window->backgroundColor.a
    );

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, (int)window->width, (int)window->height);

    if (window->drawCallback)
    {
        window->drawCallback(window);
    }

    return true;
}   