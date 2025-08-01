/***************************************************************
**
** NanoKit Library Header File
**
** File         :  nanowin.h
** Module       :  nanowin
** Author       :  SH
** Created      :  2025-02-23 (YYYY-MM-DD)
** License      :  MIT
** Description  :  NanoKit Window API
**
***************************************************************/

#ifndef NANOWIN_H
#define NANOWIN_H

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <stdint.h>
#include <stdbool.h>

#if _WIN32
    #define WIN32_LEAN_AND_MEAN

    #ifndef UNICODE
    #define UNICODE
    #endif 

    #include <windows.h>
    #include <windowsx.h>

    #include <extern/glad/glad.h>

#elif __EMSCRIPTEN__
    #include <emscripten/emscripten.h>
    #include <emscripten/html5.h>
    #include <GLES3/gl3.h>
#endif

#include <nanodraw.h>
#include <nanoview.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


/* General Keys */
#define NK_KEYCODE_SPACE            (0x0001U)
#define NK_KEYCODE_BACKSPACE        (0x0002U)
#define NK_KEYCODE_TAB              (0x0003U)
#define NK_KEYCODE_CLEAR            (0x0004U)
#define NK_KEYCODE_RETURN           (0x0005U)
#define NK_KEYCODE_PAUSE            (0x0006U)
#define NK_KEYCODE_ESCAPE           (0x0007U)
#define NK_KEYCODE_DELETE           (0x0008U)

/* Modifier Keys */
#define NK_KEYCODE_SHIFT            (0x0011U)
#define NK_KEYCODE_CONTROL          (0x0012U)
#define NK_KEYCODE_META             (0x0013U)
#define NK_KEYCODE_ALT              (0x0014U)
#define NK_KEYCODE_SUPER            (0x0015U)
#define NK_KEYCODE_HYPER            (0x0016U)

/* Navigation Keys */
#define NK_KEYCODE_PAGE_UP          (0x0021U)
#define NK_KEYCODE_PAGE_DOWN        (0x0022U)
#define NK_KEYCODE_END              (0x0023U)
#define NK_KEYCODE_HOME             (0x0024U)
#define NK_KEYCODE_LEFT             (0x0025U)
#define NK_KEYCODE_UP               (0x0026U)
#define NK_KEYCODE_RIGHT            (0x0027U)
#define NK_KEYCODE_DOWN             (0x0028U)

/* Editing Keys */
#define NK_KEYCODE_SELECT           (0x0031U)
#define NK_KEYCODE_PRINT            (0x0032U)
#define NK_KEYCODE_EXECUTE          (0x0033U)
#define NK_KEYCODE_INSERT           (0x0034U)
#define NK_KEYCODE_HELP             (0x0035U)

/* Function Keys */
#define NK_KEYCODE_F1                (0x0041U)
#define NK_KEYCODE_F2                (0x0042U)
#define NK_KEYCODE_F3                (0x0043U)
#define NK_KEYCODE_F4                (0x0044U)
#define NK_KEYCODE_F5                (0x0045U)
#define NK_KEYCODE_F6                (0x0046U)
#define NK_KEYCODE_F7                (0x0047U)
#define NK_KEYCODE_F8                (0x0048U)
#define NK_KEYCODE_F9                (0x0049U)
#define NK_KEYCODE_F10               (0x004AU)
#define NK_KEYCODE_F11               (0x004BU)
#define NK_KEYCODE_F12               (0x004CU)
#define NK_KEYCODE_F13               (0x004DU)
#define NK_KEYCODE_F14               (0x004EU)
#define NK_KEYCODE_F15               (0x004FU)
#define NK_KEYCODE_F16               (0x0050U)
#define NK_KEYCODE_F17               (0x0051U)
#define NK_KEYCODE_F18               (0x0052U)
#define NK_KEYCODE_F19               (0x0053U)
#define NK_KEYCODE_F20               (0x0054U)
#define NK_KEYCODE_F21               (0x0055U)
#define NK_KEYCODE_F22               (0x0056U)
#define NK_KEYCODE_F23               (0x0057U)
#define NK_KEYCODE_F24               (0x0058U)

#if _WIN32
    #define NK_CURSOR_ARROW_VALUE        ((uintptr_t)IDC_ARROW)
    #define NK_CURSOR_IBEAM_VALUE        ((uintptr_t)IDC_IBEAM)
    #define NK_CURSOR_HAND_VALUE         ((uintptr_t)IDC_HAND)
    #define NK_CURSOR_CROSSHAIR_VALUE    ((uintptr_t)IDC_CROSS)
    #define NK_CURSOR_SIZEALL_VALUE      ((uintptr_t)IDC_SIZEALL)
    #define NK_CURSOR_SIZENWSE_VALUE     ((uintptr_t)IDC_SIZENWSE)
    #define NK_CURSOR_SIZENESW_VALUE     ((uintptr_t)IDC_SIZENESW)
    #define NK_CURSOR_SIZEWE_VALUE       ((uintptr_t)IDC_SIZEWE)   
    #define NK_CURSOR_SIZENS_VALUE       ((uintptr_t)IDC_SIZENS)
#else
    #define NK_CURSOR_ARROW_VALUE        (0x0000U)
    #define NK_CURSOR_IBEAM_VALUE        (0x0001U)
    #define NK_CURSOR_HAND_VALUE         (0x0002U)
    #define NK_CURSOR_CROSSHAIR_VALUE    (0x0003U)
    #define NK_CURSOR_SIZEALL_VALUE      (0x0004U)
    #define NK_CURSOR_SIZENWSE_VALUE     (0x0005U)
    #define NK_CURSOR_SIZENESW_VALUE     (0x0006U)
    #define NK_CURSOR_SIZEWE_VALUE       (0x0007U)
    #define NK_CURSOR_SIZENS_VALUE       (0x0008U)
#endif




/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef enum 
{
    NK_CURSOR_ARROW         = NK_CURSOR_ARROW_VALUE,
    NK_CURSOR_IBEAM         = NK_CURSOR_IBEAM_VALUE,
    NK_CURSOR_HAND          = NK_CURSOR_HAND_VALUE,
    NK_CURSOR_CROSSHAIR     = NK_CURSOR_CROSSHAIR_VALUE,
    NK_CURSOR_SIZEALL       = NK_CURSOR_SIZEALL_VALUE,
    NK_CURSOR_SIZENWSE      = NK_CURSOR_SIZENWSE_VALUE,
    NK_CURSOR_SIZENESW      = NK_CURSOR_SIZENESW_VALUE,
    NK_CURSOR_SIZEWE        = NK_CURSOR_SIZEWE_VALUE,
    NK_CURSOR_SIZENS        = NK_CURSOR_SIZENS_VALUE
} nkCursorType_t;

typedef enum
{
    NK_WINDOW_VISIBILITY_VISIBLE        = 0x01,
    NK_WINDOW_VISIBILITY_HIDDEN         = 0x02,
    NK_WINDOW_VISIBILITY_MINIMIZED      = 0x03,
    NK_WINDOW_VISIBILITY_MAXIMIZED      = 0x04,
    NK_WINDOW_VISIBILITY_FULLSCREEN     = 0x05
} nkWindowVisibility_t;

typedef enum
{
    NK_WINDOW_FOCUS_FOCUSED          = 0x01,
    NK_WINDOW_FOCUS_UNFOCUSED        = 0x02
} nkWindowFocus_t;

struct nkWindow_t; /* forward declaration */

/* General Window Events */
typedef void (*nkWindowResizeCallback_t)(struct nkWindow_t *window, float width, float height);
typedef void (*nkWindowDrawCallback_t)(struct nkWindow_t *window);
typedef void (*nkWindowCloseCallback_t)(struct nkWindow_t *window);
typedef void (*nkWindowVisibilityChangeCallback_t)(struct nkWindow_t *window, nkWindowVisibility_t visibility);
typedef void (*nkWindowFocusChangeCallback_t)(struct nkWindow_t *window, nkWindowFocus_t focus);

/* Pointer Events */
typedef void (*nkWindowPointerMoveCallback_t)(struct nkWindow_t *window, float x, float y);
typedef void (*nkWindowPointerActionBeginCallback_t)(struct nkWindow_t *window, nkPointerAction_t action, float x, float y);
typedef void (*nkWindowPointerActionEndCallback_t)(struct nkWindow_t *window, nkPointerAction_t action, float x, float y);
typedef void (*nkWindowScrollCallback_t)(struct nkWindow_t *window, float deltaX, float deltaY);

/* Keyboard Events */
typedef void (*nkWindowKeyDownCallback_t)(struct nkWindow_t *window, uint32_t keycode);
typedef void (*nkWindowKeyUpCallback_t)(struct nkWindow_t *window, uint32_t keycode);
typedef void (*nkWindowCodepointInputCallback_t)(struct nkWindow_t *window, uint32_t codepoint);

typedef struct nkWindow_t
{
    struct nkWindow_t *next;

    const char *title;
    float width;
    float height;

    nkColor_t backgroundColor;

    nkDrawContext_t drawContext;

    nkWindowVisibility_t visibility;
    nkWindowFocus_t focus;
    nkCursorType_t cursorType;

    nkWindowResizeCallback_t resizeCallback;
    nkWindowDrawCallback_t drawCallback;
    nkWindowCloseCallback_t closeCallback;
    nkWindowVisibilityChangeCallback_t visibilityChangeCallback;
    nkWindowFocusChangeCallback_t focusChangeCallback;

    nkWindowPointerMoveCallback_t pointerMoveCallback;
    nkWindowPointerActionBeginCallback_t pointerActionBeginCallback;
    nkWindowPointerActionEndCallback_t pointerActionEndCallback;
    nkWindowScrollCallback_t scrollCallback;

    nkWindowKeyDownCallback_t keyDownCallback;
    nkWindowKeyUpCallback_t keyUpCallback;
    nkWindowCodepointInputCallback_t codepointInputCallback;

    /* view management */
    nkView_t *rootView;     /* root view of the window */
    nkView_t *hotView;      /* view under cursor */
    nkView_t *activeView;   /* view capturing input */
    nkPointerAction_t activeAction;
    nkPoint_t activeOrigin; /* origin of the active pointer action in window coords */

    #if _WIN32
        HWND windowHandle;
        HINSTANCE instanceHandle;
        HDC drawingContext;
        HGLRC glRenderContext;
        PAINTSTRUCT paintStruct;
    #endif
} nkWindow_t;

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

bool nkWindow_Create(nkWindow_t *window, const char *title, float width, float height); 
void nkWindow_SetTitle(nkWindow_t *window, const char *title);
void nkWindow_SetSize(nkWindow_t *window, float width, float height);
void nkWindow_SetVisibility(nkWindow_t *window, nkWindowVisibility_t visibility);
void nkWindow_SetFocus(nkWindow_t *window, nkWindowFocus_t focus);
void nkWindow_SetCursor(nkWindow_t *window, nkCursorType_t cursorType);
void nkWindow_Destroy(nkWindow_t *window);

bool nkWindow_IsPointerActionDown(nkWindow_t *window, nkPointerAction_t action);
bool nkWindow_IsKeyDown(nkWindow_t *window, uint32_t keycode);

void nkWindow_RequestRedraw(nkWindow_t *window);

void nkWindow_RedrawViews(nkWindow_t *window);
void nkWindow_LayoutViews(nkWindow_t *window);

/* polls for events, returning true if application should stay open */
bool nkWindow_PollEvents(void);

#ifdef __cplusplus
}
#endif

#endif /* NANOWIN_H */
